# Generated by the gRPC Python protocol compiler plugin. DO NOT EDIT!
"""Client and server classes corresponding to protobuf-defined services."""
import grpc
import warnings

import example_pb2 as example__pb2

GRPC_GENERATED_VERSION = '1.71.0'
GRPC_VERSION = grpc.__version__
_version_not_supported = False

try:
    from grpc._utilities import first_version_is_lower
    _version_not_supported = first_version_is_lower(GRPC_VERSION, GRPC_GENERATED_VERSION)
except ImportError:
    _version_not_supported = True

if _version_not_supported:
    raise RuntimeError(
        f'The grpc package installed is at version {GRPC_VERSION},'
        + f' but the generated code in example_pb2_grpc.py depends on'
        + f' grpcio>={GRPC_GENERATED_VERSION}.'
        + f' Please upgrade your grpc module to grpcio>={GRPC_GENERATED_VERSION}'
        + f' or downgrade your generated code using grpcio-tools<={GRPC_VERSION}.'
    )


class DemoServiceStub(object):
    """Missing associated documentation comment in .proto file."""

    def __init__(self, channel):
        """Constructor.

        Args:
            channel: A grpc.Channel.
        """
        self.UnaryCall = channel.unary_unary(
                '/example.DemoService/UnaryCall',
                request_serializer=example__pb2.Request.SerializeToString,
                response_deserializer=example__pb2.Response.FromString,
                _registered_method=True)
        self.ServerStreamingCall = channel.unary_stream(
                '/example.DemoService/ServerStreamingCall',
                request_serializer=example__pb2.Request.SerializeToString,
                response_deserializer=example__pb2.Response.FromString,
                _registered_method=True)
        self.ClientStreamingCall = channel.stream_unary(
                '/example.DemoService/ClientStreamingCall',
                request_serializer=example__pb2.Request.SerializeToString,
                response_deserializer=example__pb2.Response.FromString,
                _registered_method=True)
        self.BidiStreamingCall = channel.stream_stream(
                '/example.DemoService/BidiStreamingCall',
                request_serializer=example__pb2.Request.SerializeToString,
                response_deserializer=example__pb2.Response.FromString,
                _registered_method=True)


class DemoServiceServicer(object):
    """Missing associated documentation comment in .proto file."""

    def UnaryCall(self, request, context):
        """Missing associated documentation comment in .proto file."""
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')

    def ServerStreamingCall(self, request, context):
        """Missing associated documentation comment in .proto file."""
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')

    def ClientStreamingCall(self, request_iterator, context):
        """Missing associated documentation comment in .proto file."""
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')

    def BidiStreamingCall(self, request_iterator, context):
        """Missing associated documentation comment in .proto file."""
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')


def add_DemoServiceServicer_to_server(servicer, server):
    rpc_method_handlers = {
            'UnaryCall': grpc.unary_unary_rpc_method_handler(
                    servicer.UnaryCall,
                    request_deserializer=example__pb2.Request.FromString,
                    response_serializer=example__pb2.Response.SerializeToString,
            ),
            'ServerStreamingCall': grpc.unary_stream_rpc_method_handler(
                    servicer.ServerStreamingCall,
                    request_deserializer=example__pb2.Request.FromString,
                    response_serializer=example__pb2.Response.SerializeToString,
            ),
            'ClientStreamingCall': grpc.stream_unary_rpc_method_handler(
                    servicer.ClientStreamingCall,
                    request_deserializer=example__pb2.Request.FromString,
                    response_serializer=example__pb2.Response.SerializeToString,
            ),
            'BidiStreamingCall': grpc.stream_stream_rpc_method_handler(
                    servicer.BidiStreamingCall,
                    request_deserializer=example__pb2.Request.FromString,
                    response_serializer=example__pb2.Response.SerializeToString,
            ),
    }
    generic_handler = grpc.method_handlers_generic_handler(
            'example.DemoService', rpc_method_handlers)
    server.add_generic_rpc_handlers((generic_handler,))
    server.add_registered_method_handlers('example.DemoService', rpc_method_handlers)


 # This class is part of an EXPERIMENTAL API.
class DemoService(object):
    """Missing associated documentation comment in .proto file."""

    @staticmethod
    def UnaryCall(request,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.unary_unary(
            request,
            target,
            '/example.DemoService/UnaryCall',
            example__pb2.Request.SerializeToString,
            example__pb2.Response.FromString,
            options,
            channel_credentials,
            insecure,
            call_credentials,
            compression,
            wait_for_ready,
            timeout,
            metadata,
            _registered_method=True)

    @staticmethod
    def ServerStreamingCall(request,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.unary_stream(
            request,
            target,
            '/example.DemoService/ServerStreamingCall',
            example__pb2.Request.SerializeToString,
            example__pb2.Response.FromString,
            options,
            channel_credentials,
            insecure,
            call_credentials,
            compression,
            wait_for_ready,
            timeout,
            metadata,
            _registered_method=True)

    @staticmethod
    def ClientStreamingCall(request_iterator,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.stream_unary(
            request_iterator,
            target,
            '/example.DemoService/ClientStreamingCall',
            example__pb2.Request.SerializeToString,
            example__pb2.Response.FromString,
            options,
            channel_credentials,
            insecure,
            call_credentials,
            compression,
            wait_for_ready,
            timeout,
            metadata,
            _registered_method=True)

    @staticmethod
    def BidiStreamingCall(request_iterator,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.stream_stream(
            request_iterator,
            target,
            '/example.DemoService/BidiStreamingCall',
            example__pb2.Request.SerializeToString,
            example__pb2.Response.FromString,
            options,
            channel_credentials,
            insecure,
            call_credentials,
            compression,
            wait_for_ready,
            timeout,
            metadata,
            _registered_method=True)
