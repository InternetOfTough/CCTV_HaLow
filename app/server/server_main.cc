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

//set port num for grpc
ABSL_FLAG(uint16_t, port, 10000, "Server port for the service");

int main(int argc, char **argv)
{

  isConnected = false;// for 웹소켓 연결

  // Run the server
  absl::ParseCommandLine(argc, argv);
  // RunServer(absl::GetFlag(FLAGS_port));
  std::thread serverThread(RunServer, absl::GetFlag(FLAGS_port));
  serverThread.detach();


  //websocket for connection brower
  // Set logging
  serverWeb.set_access_channels(websocketpp::log::alevel::all);
  serverWeb.clear_access_channels(websocketpp::log::alevel::frame_payload);

  // Define the callback to handle connections
  serverWeb.set_open_handler([&](connection_hdl ori_hdl)
                             {
                               std::cout << "web browser connected" << std::endl;
                               hdl = ori_hdl;
                               isConnected = true;
                               // Create a separate thread to send data to the client
                               // std::thread(sendData, std::ref(server), hdl).detach();
                             });

  // Initialize the server
  serverWeb.init_asio();
  serverWeb.set_reuse_addr(true);

  // Listen on port 9002
  serverWeb.listen(9002);

  // Start the server accept loop
  serverWeb.start_accept();

  // Start the ASIO io_service run loop
  serverWeb.run();


  return 0;
}
