// VideoStreamer.h

#ifndef VIDEO_STREAMER_H
#define VIDEO_STREAMER_H

#include <vector>
#include <thread>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <grpcpp/grpcpp.h>
#include "streaming.grpc.pb.h" // gRPC 프로토콜 파일을 포함해야 합니다
#include "MemoryVideoWriter.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using streaming::Frame;
using streaming::Streaming;

using namespace cv;
using namespace std;

class VideoStreamer
{
public:
  VideoStreamer(const string &server_address, char *pi_name);
  ~VideoStreamer();
  void StreamVideo();
  void readFile(string &filePath, std::vector<char> &buffer);
  void encodeToFile(unique_ptr<grpc::ClientAsyncWriter<Frame>> &writer, VideoWriter &out);
  void encodeToMemory(unique_ptr<grpc::ClientAsyncWriter<Frame>> &writer);
  string checkPiStatus();
  void GrpcThread();

private:
  // The producer-consumer queue we use to communicate asynchronously with the
  // gRPC runtime.
    // Thread that notifies the gRPC completion queue tags.
  std::unique_ptr<std::thread> grpc_thread_;
  CompletionQueue cq_;
  unique_ptr<Streaming::Stub> stub_;
  const string pi_name_;
  streaming::ServerMessage response_;
  Frame frame_message;
  ClientContext context_;
  Status status;
  // 파일이름 생성
  unsigned int nameIndex = 1;
  // const string firstName("1.mp4");
  const string kFileType = ".mp4";
  string outputFileName = "";
  MemoryVideoWriter *memoryVideoWriter = nullptr;

  // 시작 시간 기록
  const int durationSeconds = 10;
  int64 startTickCount;

  // frame 개수 계산
  int frameCount = 1;
  int frameWidth;
  int frameHeight;

  bool is_connected_ = true;
  
  enum class Type
  {
    READ = 1,
    WRITE = 2,
    CONNECT = 3,
    WRITES_DONE = 4,
    FINISH = 5
  };
};

#endif // VIDEO_STREAMER_H
