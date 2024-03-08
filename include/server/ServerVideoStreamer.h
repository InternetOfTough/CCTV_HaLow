
#ifndef SERVER_VIDEO_STREAMER_H
#define SERVER_VIDEO_STREAMER_H

#include <iostream>
#include <grpcpp/grpcpp.h>
#include "streaming.grpc.pb.h"

using grpc::CallbackServerContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReadReactor;
using grpc::Status;
using streaming::Frame;
using streaming::ServerMessage;
using streaming::Streaming;

using namespace std;

// class VideoStreamingImpl final : public Streaming::Service //동기식
class VideoStreamingImpl final : public Streaming::CallbackService // 비 동기식
{
public:
  ServerReadReactor<Frame> *streamVideo(CallbackServerContext *context, ServerMessage *response) override; // 비동기
  // Status streamVideo(ServerContext *context, ServerReader<Frame> *reader, ServerMessage *response) override; //동기식

private:
  class CallBack : public ServerReadReactor<Frame>
  {
  public:
    CallBack(ServerMessage *response);
    ~CallBack();
    void OnDone() override;
    void OnReadDone(bool ok) override;

  private:
    void UpdateM3u8(unsigned int index_name, string *pi_name);

    Frame frame_;
    ServerMessage *response_;

    unsigned int index_name_ = 1;

    // ffmpeg 명령어를 실행
    const static string kFirstCommand;
    const static string kAfterCommand1;

    const static string kAfterCommand2;
    const static string kM38uName;

    const static string kDirectoryPath;
    const static string kFileType;
  };
};

void RunServer(uint16_t port);

void RunServerHLS(const string &address, int port);

#endif // VIDEO_STREAMER_H
