
#include "ServerVideoStreamer.h"

#include <httplib.h>

void RunServer(uint16_t port)
{
  VideoStreamingImpl service;

  std::string server_address = absl::StrFormat("0.0.0.0:%d", port);// 0.0.0.0 -> 모든 주소 다 accept

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
        std::string hlsFilePath = "/home/kho/cpp/cctv/CCTV_HaLow/video/pi5/output.m3u8";

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
    std::string tsFilePath = "/home/kho/cpp/cctv/CCTV_HaLow/video/pi5/output" + wildcardPart + ".ts";


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

  // 원하는 포트로 서버를 시작
  server.listen("172.30.1.27", port);

  std::cout << "Server started at http://172.30.1.27:9002/" << std::endl;
}