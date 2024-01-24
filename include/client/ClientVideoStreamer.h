// VideoStreamer.h

#ifndef VIDEO_STREAMER_H
#define VIDEO_STREAMER_H

#include <iostream>
#include <grpcpp/grpcpp.h>
#include "streaming.grpc.pb.h" // gRPC 프로토콜 파일을 포함해야 합니다

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using streaming::Frame;
using streaming::Streaming;

class VideoStreamer
{
public:
  VideoStreamer(const std::string &server_address);
  ~VideoStreamer();
  void StreamVideo();

private:
  std::unique_ptr<Streaming::Stub> stub_;
};

#endif // VIDEO_STREAMER_H
