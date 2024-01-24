// VideoStreamer.cpp

#include <iostream>
#include <opencv2/opencv.hpp>
#include <grpcpp/grpcpp.h>
#include "streaming.grpc.pb.h" // gRPC 프로토콜 파일을 포함해야 합니다
#include "ClientVideoStreamer.h"

using namespace cv;
using namespace std;

VideoStreamer::VideoStreamer(const string &server_address)
    : stub_(Streaming::NewStub(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials())))
{
  std::cout << "VideoStreamer instance generated" << std::endl;
}

VideoStreamer::~VideoStreamer()
{
  std::cout << "VideoStreamer instance deleted" << std::endl;
}

void VideoStreamer::StreamVideo()
{

  Mat frame;
  //--- INITIALIZE VIDEOCAPTURE
  VideoCapture cap;
  // open the default camera using default API
  // cap.open(0);
  // OR advance usage: select any API backend
  // int deviceID = 0;        // 0 = open default camera
  // int apiID = cv::CAP_ANY; // 0 = autodetect default API
  // open selected camera using selected API
  // cap.open(deviceID, apiID);
  cap.open(-1);
  // check if we succeeded
  if (!cap.isOpened())
  {
    cerr << "ERROR! Unable to open camera\n";
    exit(1);
  }

  // Send frame to server
  streaming::EmptyMessage response;
  ClientContext context;
  std::unique_ptr<grpc::ClientWriter<Frame>> writer(stub_->StreamVideo(&context, &response).release());

  while (cap.read(frame))
  {

    // // check if we succeeded
    if (frame.empty())
    {
      cerr << "ERROR! blank frame grabbed\n";
      break;
    }

  
    // Convert OpenCV Mat to gRPC Frame message
    Frame frame_message;
    std::vector<uchar> buffer;
    cv::imencode(".jpg", frame, buffer); // cv::Mat을 바이트 배열로 변환

    // Set the buffer to the frame message
    frame_message.mutable_data()->assign(buffer.begin(), buffer.end());

    // Send frame to server
    writer->Write(frame_message);

    // imshow("Camera Stream", frame);
    // if (waitKey(1) == 27) // Break the loop on ESC key press
      // break;
  }

  // 클라이언트의 스트리밍 완료
  writer->WritesDone();
  Status status = writer->Finish();

  if (!status.ok())
  {
    std::cerr << "Error streaming video: " << status.error_message() << std::endl;
  }
  // Release the VideoCapture and close OpenCV window
  cap.release();
  destroyAllWindows();
}
