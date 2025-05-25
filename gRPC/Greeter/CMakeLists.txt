cmake_minimum_required(VERSION 3.5)
project(GreeterServer)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_executable(greeter_server
  greeter_server.cpp
  helloworld.pb.cc
  helloworld.grpc.pb.cc
)

target_link_libraries(greeter_server
  gRPC::grpc++
  gRPC::grpc++_reflection
  protobuf::libprotobuf
)

