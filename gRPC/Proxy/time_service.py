#!/usr/bin/env python3
import grpc
from concurrent import futures
import datetime
import argparse

from grpc_reflection.v1alpha import reflection
from grpc_health.v1 import health, health_pb2_grpc
import grpc_health.v1.health_pb2 as health_pb2


import time_service_pb2
import time_service_pb2_grpc


class TimeServiceServicer(time_service_pb2_grpc.TimeServiceServicer):
    def GetCurrentTime(self, request, context):
        now = datetime.datetime.utcnow()
        epoch = now.timestamp()
        seconds = int(epoch)
        nanos = int((epoch - seconds) * 1e9)
        return time_service_pb2.TimeResponse(seconds=seconds, nanos=nanos)


def serve(port: int, enable_health: bool):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))

    # Register TimeService
    time_service_pb2_grpc.add_TimeServiceServicer_to_server(TimeServiceServicer(), server)

    service_names = [
        time_service_pb2.DESCRIPTOR.services_by_name['TimeService'].full_name,
        reflection.SERVICE_NAME,
    ]

    # Optional Health Service
    if enable_health:
        health_servicer = health.HealthServicer()
        health_pb2_grpc.add_HealthServicer_to_server(health_servicer, server)
        health_servicer.set('', health_pb2.HealthCheckResponse.SERVING)
        health_servicer.set('time.TimeService', health_pb2.HealthCheckResponse.SERVING)
        service_names.append(health.SERVICE_NAME)

    # Enable Reflection
    reflection.enable_server_reflection(service_names, server)

    server.add_insecure_port(f'[::]:{port}')
    server.start()
    print(f"Server started on port {port} (health check {'enabled' if enable_health else 'disabled'})")
    server.wait_for_termination()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="gRPC Time Server")
    parser.add_argument('--port', type=int, default=50051, help='Port to listen on')
    parser.add_argument('--enable-health', action='store_true', help='Enable gRPC Health API')
    parser.add_argument('--disable-health', action='store_false', dest='enable_health', help='Disable gRPC Health API')
    parser.set_defaults(enable_health=True)
    args = parser.parse_args()

    serve(args.port, args.enable_health)

