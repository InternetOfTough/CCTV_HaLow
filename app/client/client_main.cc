#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"
#include <stdio.h>
#include <iostream>
#include "ClientVideoStreamer.h"

// set server ip addr and port num for grpc
// ABSL_FLAG(std::string, target, "172.30.1.27:10000", "Server address");

int main(int argc, char **argv)
{
  // absl::ParseCommandLine(argc, argv);
  if (argc != 3)
  {
    std::cerr << "Usage: program_client <Ip:Port> <Pi_Name>\nex: program_client 127.0.0.1:10000 pi5\n";
    return 1;
  }

  absl::ParseCommandLine(argc, argv);

  VideoStreamer video_streamer(std::string(argv[1]), argv[2]);
  video_streamer.StreamVideo();

  return 0;
}
