// VideoStreamer.h

#ifndef VIDEO_STREAMER_H
#define VIDEO_STREAMER_H

#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <grpcpp/grpcpp.h>
#include "streaming.grpc.pb.h" // gRPC 프로토콜 파일을 포함해야 합니다
#include "MemoryVideoWriter.h"

using grpc::Channel;
using grpc::ClientContext;
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
  void encodeToFile(unique_ptr<grpc::ClientWriter<Frame>> &writer, VideoWriter &out);
  void encodeToMemory(unique_ptr<grpc::ClientWriter<Frame>> &writer);
  string checkPiStatus();

private:
  unique_ptr<Streaming::Stub> stub_;
  const string pi_name_;
  streaming::EmptyMessage response;
  ClientContext context;
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
};

#endif // VIDEO_STREAMER_H
