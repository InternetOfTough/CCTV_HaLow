#include <iostream>
#include "ServerVideoStreamer.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"
#include <grpcpp/grpcpp.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <thread>
#include <WebSockVariable.h>

// set port num for grpc
// ABSL_FLAG(uint16_t, port, 10000, "Server port for the service");

int main(int argc, char **argv)
{

  if (argc != 2)
  {
    std::cerr << "Usage: program_server <Port>\n";
    return 1;
  }

  isConnected = false; // for 웹소켓 연결

  // Run the gRPC server
  // absl::ParseCommandLine(argc, argv);
  // RunServer(absl::GetFlag(FLAGS_port));
  std::thread serverThread(RunServer, atoi(argv[1]));
  serverThread.detach();

  // // websocket for connection brower
  // //  Set logging
  //  serverWeb.set_access_channels(websocketpp::log::alevel::all);
  //  serverWeb.clear_access_channels(websocketpp::log::alevel::frame_payload);

  // // Define the callback to handle connections
  // serverWeb.set_open_handler([&](connection_hdl ori_hdl)
  //                            {
  //                              std::cout << "web browser connected" << std::endl;
  //                              hdl = ori_hdl;
  //                              isConnected = true;
  //                              // Create a separate thread to send data to the client
  //                              // std::thread(sendData, std::ref(server), hdl).detach();
  //                            });

  // // Initialize the server
  // serverWeb.init_asio();
  // serverWeb.set_reuse_addr(true);

  // // Listen on port 9002
  // serverWeb.listen(9002);

  // // Start the server accept loop
  // serverWeb.start_accept();

  // // Start the ASIO io_service run loop
  // serverWeb.run();

  // HLS 서버
  try
  {
    // 서버 설정
    std::string address = "0.0.0.0";
    std::string port = "9002";

    // 서버 시작
    runServerHLS(address, 9002);
  }
  catch (const std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  // return 0;
}
