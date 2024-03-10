// VideoStreamer.h

#ifndef VIDEO_STREAMER_H
#define VIDEO_STREAMER_H

#include <vector>
#include <thread>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "MemoryVideoWriter.h"
#include "ClientCheckPi.h"
#include "Grpc.h"


using namespace cv;
using namespace std;

class VideoStreamer
{
public:
  VideoStreamer(const string server_address, char *pi_name);
  ~VideoStreamer();
  void StreamVideo();
  void Initialize(VideoCapture &cap, int &first_fps);
  void ReadFile(string &filePath, std::vector<char> &buffer);
  void EncodeToFile(VideoWriter &out);
  void EncodeToMemory();
  void CheckVisionEmergency(Mat &frame);

private:
  // for gRPC
  Grpc grpc_;

  // 파일이름 생성
  unsigned int name_index_ = 1;
  const string kFileType = ".mp4";
  string output_file_name_ = "";

  //for checking pi status
  ClientCheckPi pi_status_;

  MemoryVideoWriter *memory_video_writer_ = nullptr;

  // 시간 기록
  const int kDurationSeconds = 5; // 영상 찍는 시간
  int64 start_tick_count_;

  // frame 개수 계산
  int frame_count_ = 1;
  int frame_width_;
  int frame_height_;

  // for vision
  int emergency_red_pixel_threshold_ratio_ = 0.1;
  string is_emergency_ = "ok";

};

#endif // VIDEO_STREAMER_H
