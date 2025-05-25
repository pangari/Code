#!/usr/bin/env python3
import grpc
import argparse
import example_pb2
import example_pb2_grpc
import time

def unary_call(stub):
    request = example_pb2.Request(message="Hello from unary client")
    response = stub.UnaryCall(request)
    print(f"Unary response: {response.message}")

def server_stream_call(stub):
    request = example_pb2.Request(message="Hello from server stream client")
    responses = stub.ServerStreamingCall(request)
    for response in responses:
        print(f"Server stream response: {response.message}")

def client_stream_call(stub):
    def request_generator():
        for i in range(3):
            msg = f"Client stream message {i+1}"
            print(f"Sending: {msg}")
            yield example_pb2.Request(message=msg)
            time.sleep(0.5)
    response = stub.ClientStreamingCall(request_generator())
    print(f"Client stream response: {response.message}")

def bidi_stream_call(stub):
    def request_generator():
        for i in range(3):
            msg = f"Bidi message {i+1}"
            print(f"Sending: {msg}")
            yield example_pb2.Request(message=msg)
            time.sleep(0.5)
    responses = stub.BidiStreamingCall(request_generator())
    for response in responses:
        print(f"Bidi stream response: {response.message}")

def main():
    parser = argparse.ArgumentParser(description="Test gRPC call modes")
    parser.add_argument('--mode', choices=['unary', 'server_stream', 'client_stream', 'bidi_stream'], required=True, help='Call mode to test')
    parser.add_argument('--port', type=int, default=50051, help='Port to connect to (default: 50051)')
    args = parser.parse_args()

    channel = grpc.insecure_channel(f'localhost:{args.port}')
    stub = example_pb2_grpc.DemoServiceStub(channel)

    if args.mode == 'unary':
        unary_call(stub)
    elif args.mode == 'server_stream':
        server_stream_call(stub)
    elif args.mode == 'client_stream':
        client_stream_call(stub)
    elif args.mode == 'bidi_stream':
        bidi_stream_call(stub)

if __name__ == '__main__':
    main()

