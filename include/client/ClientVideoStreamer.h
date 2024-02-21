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
  void ReadFile(string &filePath, std::vector<char> &buffer);
  void EncodeToFile(unique_ptr<grpc::ClientAsyncWriter<Frame>> &writer, VideoWriter &out);
  void EncodeToMemory(unique_ptr<grpc::ClientAsyncWriter<Frame>> &writer);
  string CheckPiStatus();
  void GrpcThread();

private:
  //for gRPC
  std::unique_ptr<std::thread> grpc_thread_;
  CompletionQueue cq_;
  unique_ptr<Streaming::Stub> stub_;
  const string pi_name_;
  streaming::ServerMessage response_;
  Frame frame_message_;
  ClientContext context_;
  Status status_;

  // 파일이름 생성
  unsigned int name_index_ = 1;
  const string kFileType = ".mp4";
  string output_file_name_ = "";

  MemoryVideoWriter *memory_video_writer_ = nullptr;

  // 시간 기록
  const int kDurationSeconds = 5; // 영상 찍는 시간
  int64 start_tick_count_;

  // frame 개수 계산
  int frame_count_ = 1;
  int frame_width_;
  int frame_height_;

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
