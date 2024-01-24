#ifndef SERVER_WEBS_H
#define SERVER_WEBS_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

using namespace websocketpp;
// WebSocket server
using websocket_server = websocketpp::server<websocketpp::config::asio>;
using connection_hdl = websocketpp::connection_hdl;

extern websocket_server serverWeb;
extern connection_hdl hdl;
extern bool isConnected;

#endif 
