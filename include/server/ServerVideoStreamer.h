
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
using streaming::ServerMessage;
using streaming::Frame;
using streaming::Streaming;

using namespace std;

class VideoStreamingImpl final : public Streaming::Service
{
public:
  Status streamVideo(ServerContext *context, ServerReader<Frame> *reader, ServerMessage *response) override;
    // Status streamVideo(ServerContext* context, ServerReaderWriter< ServerMessage, Frame>* stream) override;

  void UpdateM3u8(unsigned int index_name, string *pi_name);

private:
  // ffmpeg 명령어를 실행
  const string kFirstCommand = "ffmpeg -i ";
  const string kAfterCommand1 = ".mp4 -c:v copy -f hls -hls_time 10 -hls_list_size 6 -hls_delete_threshold 1 -hls_flags delete_segments+omit_endlist ./../../video/";

  const string kAfterCommand2 = ".mp4 -c:v copy -f hls -hls_time 10 -hls_list_size 6 -hls_delete_threshold 1 -hls_flags delete_segments+append_list+omit_endlist ./../../video/";
  const string kM38uName = "/output.m3u8";

  const string kDirectoryPath = "./../../video/";
  const string kFileType = ".mp4";
};

void RunServer(uint16_t port);

void RunServerHLS(const string &address, int port);

#endif // VIDEO_STREAMER_H
