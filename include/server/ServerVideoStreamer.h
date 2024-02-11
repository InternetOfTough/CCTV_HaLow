
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

using namespace std;

class VideoStreamingImpl final : public Streaming::Service
{
public:
  Status StreamVideo(ServerContext *context,ServerReader<Frame> *reader,EmptyMessage *response) override;
  // Status StreamVideo(ServerContext *context, const Frame *request, Frame *response) override;
  // virtual ::grpc::Status StreamVideo(::grpc::ServerContext *context, ::grpc::ServerReader<::streaming::Frame> *reader, ::streaming::EmptyMessage *response);
  void updateM3u8();

private:
  // ffmpeg 명령어를 실행
  const string firstCommand1 = "ffmpeg -i ";
  const string afterCommand1 = ".mp4 -c:v copy -f hls -hls_time 10 -hls_list_size 6 -hls_delete_threshold 1 -hls_flags delete_segments+omit_endlist ./../../video/output.m3u8";

  const string firstCommand2 = "ffmpeg -i ";
  const string afterCommand2 = ".mp4 -c:v copy -f hls -hls_time 10 -hls_list_size 6 -hls_delete_threshold 1 -hls_flags delete_segments+append_list+omit_endlist ./../../video/output.m3u8";

  const string directoryPath =  "./../../video/";
  const string fileType = ".mp4";
  unsigned int nameIndex = 1;
};

void RunServer(uint16_t port);

void runServerHLS(const string &address, int port);

#endif // VIDEO_STREAMER_H
