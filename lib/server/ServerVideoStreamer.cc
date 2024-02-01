#include <iostream>
#include <grpcpp/grpcpp.h>
#include "streaming.grpc.pb.h"
#include <opencv2/opencv.hpp>
#include "ServerVideoStreamer.h"
#include "WebSockVariable.h"

#include <fstream>
#include <httplib.h>

// using grpc::Server;
// using grpc::ServerBuilder;
// using grpc::ServerContext;
// using grpc::Status;
// using streaming::Frame;
// using streaming::Streaming;

using namespace ::cv;
using namespace ::std;

Status VideoStreamingImpl::StreamVideo(ServerContext *context, ServerReader<Frame> *reader, EmptyMessage *response)
{
  /* JPG로 압축해서 영상 전송!
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
  */

  // MP4로 압축해서 전송!
  Frame frame;
  int index = 1;
  while (reader->Read(&frame))
  {
    updateM3u8(index++);
  }

  return Status::OK;
}

void VideoStreamingImpl::updateM3u8(int index)
{

  // ffmpeg 명령어를 실행
  string firstCommand1 = "ffmpeg -i ";
  string afterCommand1 = ".mp4 -c:v copy -f hls -hls_time 5 -hls_list_size 5 -hls_delete_threshold 1 -hls_flags delete_segments+omit_endlist output.m3u8";

  string firstCommand2 = "ffmpeg -i ";
  string afterCommand2 = ".mp4 -c:v copy -f hls -hls_time 5 -hls_list_size 5 -hls_delete_threshold 1 -hls_flags delete_segments+append_list+omit_endlist output.m3u8";

  int result;
  if (index == 1)
  {
    firstCommand1.append(to_string(index));
    firstCommand1.append(afterCommand1);
    result = system(firstCommand1.c_str());
  }
  else
  {
    firstCommand2.append(to_string(index));
    firstCommand2.append(afterCommand2);
    result = system(firstCommand2.c_str());
  }

  // 결과 확인
  if (result == 0)
  {
    // 성공
    cout << "ffmpeg command executed successfully." << endl;
    return;
  }

  // 실패
  cerr << "Error executing ffmpeg command." << endl;
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

void runServerHLS(const std::string &address, int port)
{
  httplib::Server server;

  server.Get("/output.m3u8", [](const httplib::Request &req, httplib::Response &res)
             {
        // HLS 스트리밍 파일에 대한 경로
        std::string hlsFilePath = "/home/kho/cpp/cctv/CCTV_HaLow/build/bin/output.m3u8";

        // 파일을 읽어와서 응답으로 전송
        std::ifstream file(hlsFilePath);
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            res.set_content(content, "application/vnd.apple.mpegurl");
            // CORS 허용 헤더 추가
            res.set_header("Access-Control-Allow-Origin", "*");
        } else {
            res.status = 404;
            res.set_content("Not Found", "text/plain");
        } });

  server.Get(R"(/output(\d+).ts)", [](const httplib::Request &req, httplib::Response &res)
             {
    // Extract the wildcard part from the URL
    std::string wildcardPart = req.matches[1].str();

    // Form the complete path with the wildcard part
    std::string tsFilePath = "/home/kho/cpp/cctv/CCTV_HaLow/build/bin/output" + wildcardPart + ".ts";


    // 파일을 읽어와서 응답으로 전송
    std::ifstream file(tsFilePath);
    if (file.is_open()) {
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        res.set_content(content, "video/MP2T");  // .ts 파일의 MIME 타입

        // CORS 허용 헤더 추가 (특정 도메인으로 설정)
        res.set_header("Access-Control-Allow-Origin", "*");
        // res.set_header("Access-Control-Allow-Origin", "http://localhost:9002"); // 와일드카드 사용시 특정 도메인으로 한정하지 않으면 cross 정책 위반 에러
    } else {
        res.status = 404;
        res.set_content("Not Found", "text/plain");
    } });

  // server.Get("/output*.ts", [](const httplib::Request &req, httplib::Response &res)
  //            {
  // // .ts 파일에 대한 경로
  // // Extract the wildcard part from the URL
  // std::string wildcardPart = req.matches[0].str();

  // // Form the complete path with the wildcard part
  // std::string tsFilePath = "/home/kho/cpp/cctv/CCTV_HaLow/build/bin/output" + wildcardPart + ".ts";

  // // 파일을 읽어와서 응답으로 전송
  // std::ifstream file(tsFilePath);
  // if (file.is_open()) {
  //     std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  //     res.set_content(content, "video/MP2T");  // .ts 파일의 MIME 타입
  //     // CORS 허용 헤더 추가
  //     res.set_header("Access-Control-Allow-Origin", "*");
  // } else {
  //     res.status = 404;
  //     res.set_content("Not Found", "text/plain");
  // } });

  // 원하는 포트로 서버를 시작
  server.listen("localhost", port);

  std::cout << "Server started at http://localhost:9002/" << std::endl;
}