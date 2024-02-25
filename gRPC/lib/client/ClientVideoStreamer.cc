// VideoStreamer.cpp

#include <fstream>
#include "ClientVideoStreamer.h"

VideoStreamer::VideoStreamer(const string &server_address, char *pi_name)
    : stub_(Streaming::NewStub(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()))), pi_name_(pi_name)
{
  cout << "VideoStreamer instance generated" << endl;
  grpc_thread_.reset(
      new thread(bind(&VideoStreamer::GrpcThread, this)));
}

VideoStreamer::~VideoStreamer()
{
  cout << "Shutting down client...." << endl;
  cout << "VideoStreamer instance deleted" << endl;
  cq_.Shutdown();
  grpc_thread_->join();
}

void VideoStreamer::StreamVideo()

{

  // jpg로 압축해서 전송!!
  // Mat frame;
  // //--- INITIALIZE VIDEOCAPTURE
  // VideoCapture cap;
  // // open the default camera using default API
  // // cap.open(0);
  // // OR advance usage: select any API backend
  // // int deviceID = 0;        // 0 = open default camera
  // // int apiID = cv::CAP_ANY; // 0 = autodetect default API
  // // open selected camera using selected API
  // // cap.open(deviceID, apiID);
  // cap.open(-1);
  // // check if we succeeded
  // if (!cap.isOpened())
  // {
  //   cerr << "ERROR! Unable to open camera\n";
  //   exit(1);
  // }

  // // Send frame to server
  // // streaming::ServerMessage response_;
  // // ClientContext context_;
  // unique_ptr<grpc::ClientWriter<Frame>> writer(stub_->streamVideo(&context_, &response_).release());

  // while (cap.read(frame))
  // {

  //   // // check if we succeeded
  //   if (frame.empty())
  //   {
  //     cerr << "ERROR! blank frame grabbed\n";
  //     break;
  //   }

  //   // Convert OpenCV Mat to gRPC Frame message
  //   Frame frame_message_;
  //   vector<uchar> buffer;
  //   cv::imencode(".jpg", frame, buffer); // cv::Mat을 바이트 배열로 변환

  //   context.set_compression_algorithm(GRPC_COMPRESS_DEFLATE);
  //   // Set the buffer to the frame message
  //   frame_message_.mutable_data()->assign(buffer.begin(), buffer.end());

  //   // Send frame to server
  //   writer->Write(frame_message_);

  //   // imshow("Camera Stream", frame);
  //   // if (waitKey(1) == 27) // Break the loop on ESC key press
  //   // break;
  // }

  // // 클라이언트의 스트리밍 완료
  // writer->WritesDone();
  // Status status_ = writer->Finish();

  // if (!status_.ok())
  // {
  //   cerr << "Error streaming video: " << status_.error_message() << endl;
  // }
  // // Release the VideoCapture and close OpenCV window
  // cap.release();
  // destroyAllWindows();

  // // mp4로 압축해서 전송!!

  Mat frame;
  //--- INITIALIZE VIDEOCAPTURE
  VideoCapture cap;
  cap.open(-1);
  // check if we succeeded
  if (!cap.isOpened())
  {
    cerr << "ERROR! Unable to open camera\n";
    exit(1);
  }

  // setup camera
  int first_fps = 10;
  cap.set(CAP_PROP_FPS, first_fps);
  frame_width_ = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH))/2;
  frame_height_ = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT))/2;
  cap.set(CAP_PROP_FRAME_WIDTH, frame_width_);
  cap.set(CAP_PROP_FRAME_HEIGHT, frame_height_);

  cout << "Width : " << frame_width_ << endl;
  cout << "Height : " << frame_height_ << endl;

  // MemoryVideoWriter setup
  memory_video_writer_ = new MemoryVideoWriter(frame_width_, frame_height_, first_fps);

  output_file_name_.append(to_string(name_index_));
  output_file_name_.append(kFileType);

  // 비디오 작성자 생성
  VideoWriter out(output_file_name_, VideoWriter::fourcc('a', 'v', 'c', '1'), first_fps, Size(frame_width_, frame_height_), true); // fps = 30, avc1-> H.264 코덱의 일종

  // 시작 시간 기록
  // Send mp4 to server
  cout << "\nconnecting....\n";
  // unique_ptr<grpc::ClientAsyncWriter<Frame>> writer(stub_->streamVideo(&context_, &response));
  unique_ptr<grpc::ClientAsyncWriter<Frame>> writer(stub_->AsyncstreamVideo(&context_, &response_, &cq_,
                                                                            reinterpret_cast<void *>(Type::CONNECT)));

  start_tick_count_ = getTickCount();

  // writer->Finish(&status_, reinterpret_cast<void *>(Type::FINISH)); // async method to receive server msg(return value)
  while (true)
  {
    cap.read(frame);

    if (frame.empty())
    {
      cerr << "ERROR! blank frame grabbed\n";
      continue;
    }

    if (is_connected_)
    {
      // Write frame to MemoryVideoWriter
      memory_video_writer_->WriteFrame(frame);
      EncodeToMemory(writer);
      frame_count_++;
      continue;
    }
    // 비디오 작성
    // out.write(frame);
    // EncodeToFile(writer, out);
    // frame_count_++;
    cerr << "server disconnected " << endl;
    exit(1);
  }

  // 클라이언트의 스트리밍 완료
  writer->WritesDone(reinterpret_cast<void *>(Type::WRITES_DONE));

  if (!status_.ok())
  {
    cerr << "Error streaming video: " << status_.error_message() << endl;
  }
  // Release the VideoCapture and close OpenCV window
  cap.release();
  destroyAllWindows();
}
void VideoStreamer::EncodeToMemory(unique_ptr<grpc::ClientAsyncWriter<Frame>> &writer)
{
  // 경과 시간 계산
  double elapsed_seconds = (getTickCount() - start_tick_count_) / getTickFrequency();

  cout << "경과 시간 :" << elapsed_seconds << endl;
  // 지정된 시간 동안 캡처
  if (elapsed_seconds >= kDurationSeconds)
  {
    int64 delay_start = getTickCount();

    // Frame frame_message_;
    frame_message_.set_name(pi_name_);
    frame_message_.set_status(CheckPiStatus());

    // Get the memory buffer
    uint8_t *buffer = memory_video_writer_->GetMemoryBuffer();

    // Set the buffer to the frame message
    frame_message_.mutable_data()->assign(reinterpret_cast<const char *>(buffer), memory_video_writer_->GetMemoryBufferSize());
    cout << "buffer_size " << memory_video_writer_->GetMemoryBufferSize() << endl;
    // Send mp4 to server
    // if (!writer->Write(frame_message_))
    // {
    //   cerr << "server disconnected " << endl;
    //   exit(1);
    // }

    writer->Write(frame_message_, reinterpret_cast<void *>(Type::WRITE));
    frame_message_.Clear();

    memory_video_writer_->Reset(frame_count_ / kDurationSeconds, buffer);

    start_tick_count_ = getTickCount();
    frame_count_ = 0;
    cout << "\nmemory writer delay: "
         << (start_tick_count_ - delay_start) / getTickFrequency() << "\n\n";
  }
}

void VideoStreamer::EncodeToFile(unique_ptr<grpc::ClientAsyncWriter<Frame>> &writer, VideoWriter &out)
// void VideoStreamer::encodeToFile(unique_ptr<grpc::ClientWriter<Frame>> &writer, VideoWriter &out)
{
  // 경과 시간 계산
  double elapsed_seconds = (getTickCount() - start_tick_count_) / getTickFrequency();

  cout << "경과 시간 :" << elapsed_seconds << endl;
  // 지정된 시간 동안 캡처하고 종료
  if (elapsed_seconds >= kDurationSeconds)
  {
    int64 delay_start = getTickCount();
    out.release(); // VideoWriter의 release 함수 호출

    // Convert OpenCV mp4 to gRPC Frame message
    // Frame frame_message;
    vector<char> buffer;

    ReadFile(output_file_name_, buffer);

    // Set the buffer to the frame message
    frame_message_.mutable_data()->assign(buffer.begin(), buffer.end());
    frame_message_.Clear();

    // if (!writer->Write(frame_message_))
    // {
    //   cerr << "server disconnected " << endl;
    // }

    writer->Write(frame_message_, reinterpret_cast<void *>(Type::WRITE));

    name_index_++;
    output_file_name_.clear();
    output_file_name_.append(to_string(name_index_));
    output_file_name_.append(kFileType);
    out.open(output_file_name_, VideoWriter::fourcc('a', 'v', 'c', '1'), frame_count_ / kDurationSeconds, Size(frame_width_, frame_height_), true);

    start_tick_count_ = getTickCount();
    frame_count_ = 0;
    double delay = (start_tick_count_ - delay_start) / getTickFrequency();
    cout << "videowrite delay: " << delay << endl;
  }
}

void VideoStreamer::ReadFile(string &filePath, vector<char> &buffer)
{
  // 파일을 바이너리 모드로 읽기
  ifstream input_file(filePath, ios::binary);
  input_file.clear(); // failbit를 재설정

  if (!input_file.is_open())
  {
    cerr << "Error: Unable to open the MP4 file." << endl;
    return;
  }

  // 파일 크기 확인
  input_file.seekg(0, ios::end);
  streampos fileSize = input_file.tellg();
  input_file.seekg(0, ios::beg);

  cout << "file size : " << fileSize << endl;

  // 파일 내용을 버퍼에 읽어오기
  if (fileSize > 0)
  {
    buffer.resize(fileSize);                     // 버퍼 크기 조절
    if (input_file.read(buffer.data(), fileSize)) // buffer.data()는 vector가 관리하는 데이터 버퍼의 시작 주소
    {
      cout << "Read " << fileSize << " bytes from the MP4 file." << endl;
    }
    else
    {
      cerr << "Error: Failed to read from the " << filePath << " MP4 file." << endl;
      cerr << "Read operation status: " << input_file.rdstate() << endl;
    }
  }
  else
  {
    cerr << "Error: File is empty or invalid." << endl;
  }

  // 파일 닫기
  input_file.close();
}

void VideoStreamer::GrpcThread()
{
  while (true)
  {
    void *got_tag;
    bool ok = false;

    // blocking until next event occur
    if (!cq_.Next(&got_tag, &ok))
    {
      cerr << "Client stream closed. Quitting" << endl;
      break;
    }

    if (ok)
    {
      cout << endl
           << "**** Processing completion queue tag " << got_tag
           << endl;
      switch (static_cast<Type>(reinterpret_cast<long>(got_tag)))
      {

      case Type::WRITE:
        is_connected_ = true;
        cout << "Sending video chunk (async)." << endl;
        break;
      case Type::CONNECT:
        cout << "Server connected." << endl;
        break;
      case Type::WRITES_DONE:
        cout << "Server disconnecting." << endl;
        break;
      case Type::FINISH:
        // cout << "Client finish; status = "
        //           << (finish_status_.ok() ? "ok" : "cancelled")
        //           << endl;
        cout << "finish \nresponse:" << response_.msg() << endl;
        context_.TryCancel();
        cq_.Shutdown();
        break;
      default:
        cerr << "Unexpected tag " << got_tag << endl;
        GPR_ASSERT(false);
      }
      continue;
    }

    if (!ok && static_cast<Type>(reinterpret_cast<long>(got_tag)) == Type::WRITE)
    {
      is_connected_ = false;
      cout << "write failed. \n video is stored in local storage now." << endl;
    }
  }
}