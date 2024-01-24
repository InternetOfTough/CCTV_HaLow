#include <iostream>
#include <grpcpp/grpcpp.h>
#include "streaming.grpc.pb.h"
#include <opencv2/opencv.hpp>
#include "ServerVideoStreamer.h"
#include "WebSockVariable.h"
// using grpc::Server;
// using grpc::ServerBuilder;
// using grpc::ServerContext;
// using grpc::Status;
// using streaming::Frame;
// using streaming::Streaming;

using namespace ::cv;

Status VideoStreamingImpl::StreamVideo(ServerContext *context, ServerReader<Frame> *reader, EmptyMessage *response)
{
  Frame frame;
  while (reader->Read(&frame))
  {
    if (isConnected)
      serverWeb.send(hdl, frame.data(), websocketpp::frame::opcode::binary);
    Mat decoded_frame;
    Mat data_mat(1, frame.data().size(), CV_8U, (void *)frame.data().data());

    try
    {
      // OpenCV Mat으로 디코딩
      decoded_frame = imdecode(data_mat, IMREAD_UNCHANGED);
    }
    catch (const cv::Exception &ex)
    {
      std::cerr << "Error decoding frame: " << ex.what() << std::endl;
      return Status::CANCELLED;
    }

    if (decoded_frame.empty())
    {
      std::cerr << "Error decoding frame" << std::endl;
      return Status::CANCELLED;
    }

    imshow("Server Stream", decoded_frame);
    if (waitKey(1) == 27) // Break the loop on ESC key press
      break;
  }

  return Status::OK;
}

void RunServer(uint16_t port)
{
  VideoStreamingImpl service;

  std::string server_address = absl::StrFormat("0.0.0.0:%d", port);

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}