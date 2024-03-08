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
  string executeCommand(const char* command);
  string getWifiESSID();
  string getWifiSignalLevel();
  string getIPv4Address();
  string getIPv6Address();
  string getWifiChannel();
  string getWifiFrequency();
  string getWifiWidth();
  string getWifiInfo();
  string getNetworkTraffic();
  string getCamera();
  void GrpcThread();
  void CheckVisionEmergency(Mat& frame);

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

  // cmd for status
  const char* cmd_signal = "iwconfig wlan0";
  const char* cmd_ip = "ifconfig";
  const char* cmd_channel = "iw wlan0 info";
  const char* cmd_traffic = "ip -s -d link show wlan0";
  const char* cmd_camera = "vcgencmd get_camera";

  MemoryVideoWriter *memory_video_writer_ = nullptr;

  // 시간 기록
  const int kDurationSeconds = 5; // 영상 찍는 시간
  int64 start_tick_count_;

  // frame 개수 계산
  int frame_count_ = 1;
  int frame_width_;
  int frame_height_;

  bool is_connected_ = true;

  //for vision
  int emergency_red_pixel_threshold_ratio_= 0.1;
  string is_emergency_ = "ok";
  
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
