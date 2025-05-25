#!/usr/bin/env python3
import argparse
import grpc
import logging
import threading
from concurrent import futures
from grpc_reflection.v1alpha import reflection_pb2, reflection_pb2_grpc
from google.protobuf import descriptor_pb2, descriptor_pool, message_factory
from google.protobuf.json_format import MessageToJson

# Prometheus
from prometheus_client import start_http_server, Counter, Histogram

logging.basicConfig(level=logging.INFO)

# Prometheus metrics
GRPC_REQUESTS = Counter(
    'grpc_proxy_requests_total', 'Total gRPC requests proxied',
    ['method', 'type', 'grpc_code'])
GRPC_LATENCY = Histogram(
    'grpc_proxy_request_duration_seconds', 'Latency of proxied gRPC requests',
    ['method', 'type'])

# Health proto import (generated from grpc-health-probe standard)
from grpc_health.v1 import health_pb2, health_pb2_grpc

# ProxyService supporting 4 call types
class ProxyService:
    def __init__(self, target, upstream_tls_opts=None, json_log=False):
        # Setup upstream channel with optional TLS
        if upstream_tls_opts:
            creds = grpc.ssl_channel_credentials(
                root_certificates=upstream_tls_opts.get('ca_cert'),
                private_key=upstream_tls_opts.get('client_key'),
                certificate_chain=upstream_tls_opts.get('client_cert')
            )
            self.channel = grpc.secure_channel(target, creds)
        else:
            self.channel = grpc.insecure_channel(target)

        self.stub = reflection_pb2_grpc.ServerReflectionStub(self.channel)
        self.pool = {}
        self.json_log = json_log
        self.method_map = {}
        self.method_type_map = {}
        self._load_services()

    def _load_services(self):
        response_stream = self.stub.ServerReflectionInfo(iter([
            reflection_pb2.ServerReflectionRequest(list_services=""),
        ]))

        service_names = []
        for response in response_stream:
            if response.HasField("list_services_response"):
                for service in response.list_services_response.service:
                    if service.name not in self.pool:
                        self.pool[service.name] = descriptor_pool.DescriptorPool()
                        service_names.append(service.name)

        logging.info(f"Found services: {service_names}")

        for service in service_names:
            response_stream = self.stub.ServerReflectionInfo(iter([
                reflection_pb2.ServerReflectionRequest(file_containing_symbol=service),
            ]))
            for response in response_stream:
                if response.HasField("file_descriptor_response"):
                    for fd_bytes in response.file_descriptor_response.file_descriptor_proto:
                        fd_proto = descriptor_pb2.FileDescriptorProto.FromString(fd_bytes)
                        try:
                            self.pool[service].Add(fd_proto)
                        except Exception:
                            pass  # ignore duplicates

        for service in service_names:
            service_desc = self.pool[service].FindServiceByName(service)
            for method in service_desc.methods:
                full_method = f"/{service}/{method.name}"
                self.method_map[full_method] = (service, method)
                # Determine method type
                if method.client_streaming and method.server_streaming:
                    call_type = "stream_stream"
                elif method.client_streaming and not method.server_streaming:
                    call_type = "stream_unary"
                elif not method.client_streaming and method.server_streaming:
                    call_type = "unary_stream"
                else:
                    call_type = "unary_unary"
                self.method_type_map[full_method] = call_type
                logging.info(f"Registered method: {full_method} [{call_type}]")

    def log_request_response(self, method_name, req_msg, resp_msg, grpc_code, call_type):
        GRPC_REQUESTS.labels(method_name, call_type, grpc_code.name).inc()
        GRPC_LATENCY.labels(method_name, call_type).observe(0)  # We'll measure time in actual handlers if needed

        if self.json_log:
            if req_msg:
                logging.info(f"Request to {method_name}:\n{MessageToJson(req_msg)}")
            if resp_msg:
                logging.info(f"Response from {method_name}:\n{MessageToJson(resp_msg)}")
        else:
            if req_msg:
                logging.info(f"Request to {method_name}: {req_msg}")
            if resp_msg:
                logging.info(f"Response from {method_name}: {resp_msg}")

    def _get_message_classes(self, method):
        request_cls = message_factory.GetMessageClass(method.input_type)
        response_cls = message_factory.GetMessageClass(method.output_type)
        return request_cls, response_cls

    # Proxy unary-unary call
    def proxy_unary_unary(self, method_name, request, context):
        if method_name not in self.method_map:
            context.set_code(grpc.StatusCode.UNIMPLEMENTED)
            context.set_details("Unknown method")
            return None

        service_name, method = self.method_map[method_name]
        request_cls, response_cls = self._get_message_classes(method)

        try:
            # Deserialize request protobuf (request is already protobuf message here)
            req_msg = request_cls.FromString(request)
            if self.json_log:
                logging.info(f"Unary-Unary Incoming request to {method_name}:\n{MessageToJson(req_msg)}")
            else:
                logging.info(f"Unary-Unary Incoming request to {method_name}:\n{req_msg}")

            upstream_call = self.channel.unary_unary(
                method_name,
                request_serializer=lambda msg: msg.SerializeToString(),
                response_deserializer=response_cls.FromString,
            )
            response = upstream_call(req_msg)

            grpc_code = grpc.StatusCode.OK
            self.log_request_response(method_name, req_msg, response, grpc_code, "unary_unary")
            return response.SerializeToString()
        except grpc.RpcError as e:
            logging.error(f"Upstream error on unary-unary: {e.code()}: {e.details()}")
            context.set_code(e.code())
            context.set_details(e.details())
            grpc_code = e.code()
            # Logging request with no response on error
            self.log_request_response(method_name, request, None, grpc_code, "unary_unary")
            return None

    # Proxy unary-stream call
    def proxy_unary_stream(self, method_name, request, context):
        if method_name not in self.method_map:
            context.set_code(grpc.StatusCode.UNIMPLEMENTED)
            context.set_details("Unknown method")
            return

        service_name, method = self.method_map[method_name]
        request_cls, response_cls = self._get_message_classes(method)
        req_msg = request_cls.FromString(request)
        if self.json_log:
            logging.info(f"Unary-Stream Incoming request to {method_name}:\n{MessageToJson(req_msg)}")
        else:
            logging.info(f"Unary-Stream Incoming request to {method_name}:\n{req_msg}")

        upstream_call = self.channel.unary_stream(
            method_name,
            request_serializer=lambda msg: msg.SerializeToString(),
            response_deserializer=response_cls.FromString,
        )
        try:
            response_iterator = upstream_call(req_msg)
            grpc_code = grpc.StatusCode.OK
            for resp_msg in response_iterator:
                if self.json_log:
                    logging.info(f"Unary-Stream Response chunk from {method_name}:\n{MessageToJson(resp_msg)}")
                else:
                    logging.info(f"Unary-Stream Response chunk from {method_name}:\n{resp_msg}")
                yield resp_msg.SerializeToString()
            # Log summary metrics after successful stream end
            GRPC_REQUESTS.labels(method_name, "unary_stream", grpc_code.name).inc()
        except grpc.RpcError as e:
            logging.error(f"Upstream error on unary-stream: {e.code()}: {e.details()}")
            context.set_code(e.code())
            context.set_details(e.details())
            grpc_code = e.code()
            GRPC_REQUESTS.labels(method_name, "unary_stream", grpc_code.name).inc()

    # Proxy stream-unary call
    def proxy_stream_unary(self, method_name, request_iterator, context):
        if method_name not in self.method_map:
            context.set_code(grpc.StatusCode.UNIMPLEMENTED)
            context.set_details("Unknown method")
            return None

        service_name, method = self.method_map[method_name]
        request_cls, response_cls = self._get_message_classes(method)

        def serialize_requests():
            for req in request_iterator:
                yield request_cls.FromString(req)

        upstream_call = self.channel.stream_unary(
            method_name,
            request_serializer=lambda msg: msg.SerializeToString(),
            response_deserializer=response_cls.FromString,
        )
        try:
            reqs = list(request_iterator)  # Need to consume for logging before forwarding
            if self.json_log:
                for r in reqs:
                    request = request_cls.FromString(r)
                    logging.info(f"Stream-Unary Incoming request chunk to {method_name}:\n{MessageToJson(request)}")
            else:
                logging.info(f"Stream-Unary Incoming request with {len(reqs)} messages to {method_name}")

            response = upstream_call(serialize_requests())

            grpc_code = grpc.StatusCode.OK
            self.log_request_response(method_name, None, response, grpc_code, "stream_unary")
            return response.SerializeToString()
        except grpc.RpcError as e:
            logging.error(f"Upstream error on stream-unary: {e.code()}: {e.details()}")
            context.set_code(e.code())
            context.set_details(e.details())
            grpc_code = e.code()
            self.log_request_response(method_name, None, None, grpc_code, "stream_unary")
            return None

    # Proxy stream-stream call
    def proxy_stream_stream(self, method_name, request_iterator, context):
        if method_name not in self.method_map:
            context.set_code(grpc.StatusCode.UNIMPLEMENTED)
            context.set_details("Unknown method")
            return

        service_name, method = self.method_map[method_name]
        request_cls, response_cls = self._get_message_classes(method)

        upstream_call = self.channel.stream_stream(
            method_name,
            request_serializer=lambda msg: msg.SerializeToString(),
            response_deserializer=response_cls.FromString,
        )
        try:
            # We cannot rewind request_iterator easily, so we just forward it directly
            # Logging chunks as they come
            def request_gen():
                for req in request_iterator:
                    request = request_cls.FromString(req)
                    if self.json_log:
                        logging.info(f"Stream-Stream Incoming request chunk to {method_name}:\n{MessageToJson(request)}")
                    else:
                        logging.info(f"Stream-Stream Incoming request chunk to {method_name}: {request}")
                    yield request

            responses = upstream_call(request_gen())
            grpc_code = grpc.StatusCode.OK
            for resp in responses:
                if self.json_log:
                    logging.info(f"Stream-Stream Response chunk from {method_name}:\n{MessageToJson(resp)}")
                else:
                    logging.info(f"Stream-Stream Response chunk from {method_name}:\n{resp}")
                yield resp.SerializeToString()
            GRPC_REQUESTS.labels(method_name, "stream_stream", grpc_code.name).inc()
        except grpc.RpcError as e:
            logging.error(f"Upstream error on stream-stream: {e.code()}: {e.details()}")
            context.set_code(e.code())
            context.set_details(e.details())
            grpc_code = e.code()
            GRPC_REQUESTS.labels(method_name, "stream_stream", grpc_code.name).inc()

    # Health proxy (grpc.health.v1.Health) transparently proxied upstream
    def proxy_health(self, request, context):
        health_stub = health_pb2_grpc.HealthStub(self.channel)
        method = context._rpc_event.call_details.method  # Hack to get method name
        try:
            if context._rpc_event.is_streaming_request:
                if context._rpc_event.is_streaming_response:
                    # stream_stream
                    responses = health_stub.Health.stream_health(request)
                    for resp in responses:
                        yield resp
                else:
                    # stream_unary
                    response = health_stub.Health.watch(request)
                    yield response
            else:
                if context._rpc_event.is_streaming_response:
                    # unary_stream
                    responses = health_stub.Health.watch(request)
                    for resp in responses:
                        yield resp
                else:
                    # unary_unary
                    response = health_stub.Check(request)
                    return response
        except grpc.RpcError as e:
            context.set_code(e.code())
            context.set_details(e.details())
            return None

    def get_generic_rpc_handlers(self):
        proxy = self

        class GenericHandler(grpc.GenericRpcHandler):
            def service(self, handler_call_details):
                method = handler_call_details.method
                if method not in proxy.method_map:
                    # Special case: health check proxying
                    if method.startswith("/grpc.health.v1.Health/"):
                        # Handle health service methods by forwarding
                        method_name = method
                        # We need to determine the call type by reflection or assume unary_unary (Check)
                        # For simplicity, handle Check as unary-unary, Watch as unary-stream
                        if method.endswith("Check"):
                            return grpc.unary_unary_rpc_method_handler(
                                lambda request, context: proxy.proxy_unary_unary(method_name, request, context)
                            )
                        elif method.endswith("Watch"):
                            return grpc.unary_stream_rpc_method_handler(
                                lambda request, context: proxy.proxy_unary_stream(method_name, request, context)
                            )
                        else:
                            return None

                    return None

                call_type = proxy.method_type_map[method]
                if call_type == "unary_unary":
                    return grpc.unary_unary_rpc_method_handler(
                        lambda request, context: proxy.proxy_unary_unary(method, request, context)
                    )
                elif call_type == "unary_stream":
                    return grpc.unary_stream_rpc_method_handler(
                        lambda request, context: proxy.proxy_unary_stream(method, request, context)
                    )
                elif call_type == "stream_unary":
                    return grpc.stream_unary_rpc_method_handler(
                        lambda request_iterator, context: proxy.proxy_stream_unary(method, request_iterator, context)
                    )
                elif call_type == "stream_stream":
                    return grpc.stream_stream_rpc_method_handler(
                        lambda request_iterator, context: proxy.proxy_stream_stream(method, request_iterator, context)
                    )
                else:
                    return None

        return [GenericHandler()]

def serve(
    proxy_port,
    upstream_target,
    json_log=False,
    server_tls_opts=None,
    upstream_tls_opts=None,
    metrics_port=None,
):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))

    proxy = ProxyService(upstream_target, upstream_tls_opts=upstream_tls_opts, json_log=json_log)

    for handler in proxy.get_generic_rpc_handlers():
        server.add_generic_rpc_handlers((handler,))

    if server_tls_opts:
        server_credentials = grpc.ssl_server_credentials(
            [(server_tls_opts['private_key'], server_tls_opts['certificate_chain'])]
        )
        server.add_secure_port(f"[::]:{proxy_port}", server_credentials)
        logging.info(f"gRPC proxy listening with TLS on port {proxy_port}, forwarding to {upstream_target}")
    else:
        server.add_insecure_port(f"[::]:{proxy_port}")
        logging.info(f"gRPC proxy listening on port {proxy_port}, forwarding to {upstream_target}")

    # Start Prometheus metrics server if requested
    if metrics_port:
        threading.Thread(target=start_http_server, args=(metrics_port,), daemon=True).start()
        logging.info(f"Prometheus metrics available on http://localhost:{metrics_port}/")

    server.start()
    server.wait_for_termination()


def load_file(path):
    with open(path, 'rb') as f:
        return f.read()


def main():
    parser = argparse.ArgumentParser(description="gRPC Proxy using reflection")

    parser.add_argument("--port", type=int, default=50051, help="Port to listen on")
    parser.add_argument("--upstream", required=True, help="Upstream gRPC server address (e.g. localhost:50052)")
    parser.add_argument("--json-log", action="store_true", help="Log requests/responses in JSON format")

    # TLS for proxy server (client connections)
    parser.add_argument("--server-tls-cert", help="TLS certificate file for proxy server")
    parser.add_argument("--server-tls-key", help="TLS private key file for proxy server")

    # TLS for upstream channel
    parser.add_argument("--upstream-tls-ca", help="Upstream TLS CA cert file")
    parser.add_argument("--upstream-tls-client-cert", help="Upstream client cert file (optional for mTLS)")
    parser.add_argument("--upstream-tls-client-key", help="Upstream client private key file (optional for mTLS)")

    # Metrics HTTP port (optional)
    parser.add_argument("--metrics-port", type=int, default=None, help="HTTP port to expose Prometheus metrics")

    args = parser.parse_args()

    server_tls_opts = None
    if args.server_tls_cert and args.server_tls_key:
        server_tls_opts = {
            'certificate_chain': load_file(args.server_tls_cert),
            'private_key': load_file(args.server_tls_key),
        }

    upstream_tls_opts = None
    if args.upstream_tls_ca:
        upstream_tls_opts = {
            'ca_cert': load_file(args.upstream_tls_ca),
            'client_cert': None,
            'client_key': None,
        }
        if args.upstream_tls_client_cert and args.upstream_tls_client_key:
            upstream_tls_opts['client_cert'] = load_file(args.upstream_tls_client_cert)
            upstream_tls_opts['client_key'] = load_file(args.upstream_tls_client_key)

    serve(
        proxy_port=args.port,
        upstream_target=args.upstream,
        json_log=args.json_log,
        server_tls_opts=server_tls_opts,
        upstream_tls_opts=upstream_tls_opts,
        metrics_port=args.metrics_port,
    )


if __name__ == "__main__":
    main()

