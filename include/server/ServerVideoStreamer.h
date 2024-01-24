
#ifndef SERVER_VIDEO_STREAMER_H
#define SERVER_VIDEO_STREAMER_H

#include <iostream>
#include <grpcpp/grpcpp.h>
#include "streaming.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::Status;
using streaming::Frame;
using streaming::Streaming;
using streaming::EmptyMessage;


class VideoStreamingImpl final : public Streaming::Service
{
public:
  Status StreamVideo(ServerContext *context,ServerReader<Frame> *reader,EmptyMessage *response) override;
  // Status StreamVideo(ServerContext *context, const Frame *request, Frame *response) override;
  // virtual ::grpc::Status StreamVideo(::grpc::ServerContext *context, ::grpc::ServerReader<::streaming::Frame> *reader, ::streaming::EmptyMessage *response);
};

void RunServer(uint16_t port);
#endif // VIDEO_STREAMER_H
