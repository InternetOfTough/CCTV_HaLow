#include <iostream>
#include "ServerVideoStreamer.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"
#include <thread>

// set port num for grpc
// ABSL_FLAG(uint16_t, port, 10000, "Server port for the service");

int main(int argc, char **argv)
{

  if (argc != 2)
  {
    std::cerr << "Usage: program_server <Port>\n";
    return 1;
  }


  // Run the gRPC server
  // absl::ParseCommandLine(argc, argv);
  // RunServer(absl::GetFlag(FLAGS_port));
  std::thread serverThread(RunServer, atoi(argv[1]));
  serverThread.detach();

  // HLS 서버
  try
  {
    // 서버 설정
    std::string address = "0.0.0.0";
    std::string port = "9002";

    // 서버 시작
    RunServerHLS(address, 9002);
  }
  catch (const std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  // return 0;
}
