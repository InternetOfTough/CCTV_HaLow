#include <iostream>
#include <grpcpp/grpcpp.h>
#include "streaming.grpc.pb.h"
#include <opencv2/opencv.hpp>
#include "ServerVideoStreamer.h"
#include "WebSockVariable.h"
#include "directoryAPI.h"

using namespace ::cv;
using namespace ::std;

Status VideoStreamingImpl::streamVideo(ServerContext *context, ServerReader<Frame> *reader, ServerMessage *response)
{
  // JPG로 압축해서 영상 전송!
  // Frame frame;
  // while (reader->Read(&frame))
  // {
  //   if (is_connected)
  //     server_web.send(hdl, frame.data(), websocketpp::frame::opcode::binary);
  //   Mat decoded_frame;
  //   Mat data_mat(1, frame.data().size(), CV_8U, (void *)frame.data().data());

  //   try
  //   {
  //     // OpenCV Mat으로 디코딩
  //     decoded_frame = imdecode(data_mat, IMREAD_UNCHANGED);
  //   }
  //   catch (const cv::Exception &ex)
  //   {
  //     std::cerr << "Error decoding frame: " << ex.what() << std::endl;
  //     return Status::CANCELLED;
  //   }

  //   if (decoded_frame.empty())
  //   {
  //     std::cerr << "Error decoding frame" << std::endl;
  //     return Status::CANCELLED;
  //   }

  //   imshow("Server Stream", decoded_frame);
  //   if (waitKey(1) == 27) // Break the loop on ESC key press
  //     break;
  // }

  // return Status::OK;

  // MP4로 압축해서 전송 받음!
  unsigned int index_name = 1;

  Frame frame;
  
  response->set_msg("reboot");

  while (reader->Read(&frame))
  {
    string *pi_name = frame.release_name();
    std::cout << '\n'
              << *pi_name << "\n\n";

    string *status = frame.release_status();
    std::cout << '\n'
              << *status << "\n\n";
    // 디렉터리 생성 여부 확인
    if (CreateDirectoryIfNotExists(kDirectoryPath, pi_name))
    {
      // 디렉터리가 성공적으로 생성되면 파일에 버퍼 내용 쓰기

      const string filePath = kDirectoryPath + *pi_name + "/" + to_string(index_name) + kFileType;
      if (WriteMsgToFile(frame.release_data(), filePath)) // release_data() 메서드는 필드의 값을 반환하는 것이 아니라 해당 필드의 메모리를 해제하고 포인터를 반환
      {
        std::cout << "File successfully created: " << index_name << ".mp4" << std::endl;
        UpdateM3u8(index_name, pi_name);
      }
      else
      {
        std::cerr << "Failed to write buffer to file" << std::endl;
      }
    }
    else
    {
      std::cerr << "Failed to create directory" << std::endl;
    }
    delete pi_name;
    delete status;
    index_name++;
    frame.Clear();
  }

  return Status::OK;
}

void VideoStreamingImpl::UpdateM3u8(unsigned int index_name, string *pi_name)
{
  int result;
  if (index_name == 1)
  {
    const string KCommand1 = kFirstCommand + kDirectoryPath + *pi_name + "/" + to_string(index_name) + kAfterCommand1 + *pi_name + kM38uName;

    result = system(KCommand1.c_str());
  }
  else
  {
    const string KCommand2 = kFirstCommand + kDirectoryPath + *pi_name + "/" + to_string(index_name) + kAfterCommand2 + *pi_name + kM38uName;
    result = system(KCommand2.c_str());
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
