#!/usr/bin/env python3
import grpc
from concurrent import futures
import time
import argparse

from grpc_reflection.v1alpha import reflection

import example_pb2
import example_pb2_grpc

class DemoService(example_pb2_grpc.DemoServiceServicer):
    def UnaryCall(self, request, context):
        print(f"UnaryCall received: {request.message}")
        return example_pb2.Response(message=f"Unary response to: {request.message}")

    def ServerStreamingCall(self, request, context):
        print(f"ServerStreamingCall received: {request.message}")
        for i in range(3):
            yield example_pb2.Response(message=f"Stream {i+1} for: {request.message}")
            time.sleep(1)

    def ClientStreamingCall(self, request_iterator, context):
        messages = []
        for request in request_iterator:
            print(f"ClientStreamingCall received: {request.message}")
            messages.append(request.message)
        response_msg = f"Received {len(messages)} messages: {', '.join(messages)}"
        return example_pb2.Response(message=response_msg)

    def BidiStreamingCall(self, request_iterator, context):
        for request in request_iterator:
            print(f"BidiStreamingCall received: {request.message}")
            yield example_pb2.Response(message=f"Echo: {request.message}")

def serve(port: int):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    example_pb2_grpc.add_DemoServiceServicer_to_server(DemoService(), server)

    # Enable reflection
    SERVICE_NAMES = (
        example_pb2.DESCRIPTOR.services_by_name['DemoService'].full_name,
        reflection.SERVICE_NAME,
    )
    reflection.enable_server_reflection(SERVICE_NAMES, server)

    address = f"[::]:{port}"
    server.add_insecure_port(address)
    server.start()
    print(f"gRPC server with reflection running on port {port}...")
    server.wait_for_termination()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Run gRPC DemoService server.')
    parser.add_argument('--port', type=int, default=50051, help='Port to listen on (default: 50051)')
    args = parser.parse_args()

    serve(args.port)

