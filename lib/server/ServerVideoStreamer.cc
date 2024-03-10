#include <iostream>
#include <grpcpp/grpcpp.h>
#include "streaming.grpc.pb.h"
#include <opencv2/opencv.hpp>
#include "ServerVideoStreamer.h"
#include "WebSockVariable.h"
#include "directoryAPI.h"

// using namespace ::cv;
using namespace ::std;

const string VideoStreamingImpl::CallBack::kFirstCommand = "ffmpeg -i ";
const string VideoStreamingImpl::CallBack::kAfterCommand1 = ".mp4 -c:v copy -f hls -hls_time 10 -hls_list_size 6 -hls_delete_threshold 1 -hls_flags delete_segments+omit_endlist ./../../video/";

const string VideoStreamingImpl::CallBack::kAfterCommand2 = ".mp4 -c:v copy -f hls -hls_time 10 -hls_list_size 6 -hls_delete_threshold 1 -hls_flags delete_segments+append_list+omit_endlist ./../../video/";
const string VideoStreamingImpl::CallBack::kM38uName = "/output.m3u8";

const string VideoStreamingImpl::CallBack::kDirectoryPath = "./../../video/";
const string VideoStreamingImpl::CallBack::kFileType = ".mp4";

// Status VideoStreamingImpl::streamVideo(ServerContext *context, ServerReader<Frame> *reader, ServerMessage *response)
ServerReadReactor<Frame> *VideoStreamingImpl::streamVideo(CallbackServerContext *context, ServerMessage *response)
{
  return new CallBack(response);
}

VideoStreamingImpl::CallBack::CallBack(ServerMessage *response)
    : response_(response)
{
  cout << "One Callback instance created " << '\n';
  StartRead(&frame_);
}

VideoStreamingImpl::CallBack::~CallBack()
{
  cout << "One Callback instance deleted " << '\n';
}

void VideoStreamingImpl::CallBack::OnDone()
{
  delete this;
}

void VideoStreamingImpl::CallBack::OnReadDone(bool ok)
{
  if (ok)
  {
    string *pi_name = frame_.release_name();
    std::cout << '\n'
              << *pi_name << "\n\n";

    string *status = frame_.release_status();
    std::cout << '\n'
              << *status << "\n\n";
    string *vision = frame_.release_vision();
    std::cout << '\n'
              << *vision << "\n\n";
    // 디렉터리 생성 여부 확인
    if (CreateDirectoryIfNotExists(kDirectoryPath, pi_name))
    {
      // 디렉터리가 성공적으로 생성되면 파일에 버퍼 내용 쓰기

      const string filePath = kDirectoryPath + *pi_name + "/" + to_string(index_name_) + kFileType;
      if (WriteMsgToFile(frame_.release_data(), filePath)) // release_data() 메서드는 필드의 값을 반환하는 것이 아니라 해당 필드의 메모리를 해제하고 포인터를 반환
      {
        std::cout << "File successfully created: " << index_name_ << ".mp4" << std::endl;
        UpdateM3u8(index_name_, pi_name);
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
    delete vision;
    index_name_++;
    frame_.Clear();
    StartRead(&frame_);
  }
  else
  {
    response_->set_msg("reboot");
    Finish(Status::OK);
  }
}

void VideoStreamingImpl::CallBack::UpdateM3u8(unsigned int index_name, string *pi_name)
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