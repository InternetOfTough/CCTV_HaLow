#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"
#include <stdio.h>
#include <iostream>
#include "ClientVideoStreamer.h"

// using namespace cv;
// using namespace std;

// set server ip addr and port num for grpc
ABSL_FLAG(std::string, target, "172.30.1.27:10000", "Server address");


int main(int argc, char **argv)
{

  if (argc != 2)
  {
    std::cerr << "Usage: program_client <pi_name>\n";
    return 1;
  }

  absl::ParseCommandLine(argc, argv);

  std::string target_str = absl::GetFlag(FLAGS_target);

  VideoStreamer video_streamer(target_str,argv[1]);
  video_streamer.StreamVideo();

  return 0;
}
