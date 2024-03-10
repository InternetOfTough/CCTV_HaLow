// VideoStreamer.cpp

#include <fstream>
#include "ClientVideoStreamer.h"

VideoStreamer::VideoStreamer(const string server_address, char *pi_name)
    : grpc_(server_address, pi_name)
{
  cout << "VideoStreamer instance generated" << endl;
  // grpc_thread_.reset(
  //     new thread(bind(&VideoStreamer::GrpcThread, this)));
}

VideoStreamer::~VideoStreamer()
{
  cout << "VideoStreamer instance deleted" << endl;
  // cq_.Shutdown();
  // grpc_thread_->join();
}
void VideoStreamer::Initialize(VideoCapture &cap, int &first_fps)
{
  // check if we succeeded
  if (!cap.isOpened())
  {
    cerr << "ERROR! Unable to open camera\n";
    exit(1);
  }

  // setup camera
  cap.set(CAP_PROP_FPS, first_fps);
  cap.set(CAP_PROP_FRAME_WIDTH, 320);
  cap.set(CAP_PROP_FRAME_HEIGHT, 240);
  frame_width_ = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));
  frame_height_ = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));

  cout << "Width : " << frame_width_ << endl;
  cout << "Height : " << frame_height_ << endl;

  // MemoryVideoWriter setup
  memory_video_writer_ = new MemoryVideoWriter(frame_width_, frame_height_, first_fps);

  output_file_name_.append(to_string(name_index_));
  output_file_name_.append(kFileType);
}

void VideoStreamer::StreamVideo()
{
  Mat frame;
  //--- INITIALIZE VIDEOCAPTURE
  VideoCapture cap(-1);
  int first_fps = 10;
  Initialize(cap, first_fps);

  // 비디오 작성자 생성
  VideoWriter out(output_file_name_, VideoWriter::fourcc('a', 'v', 'c', '1'), first_fps, Size(frame_width_, frame_height_), true); // fps = 30, avc1-> H.264 코덱의 일종
  // 시작 시간 기록
  start_tick_count_ = getTickCount();
  // grpc_.Finish();
  while (true)
  {
    cap.read(frame);

    if (frame.empty())
    {
      cerr << "ERROR! blank frame grabbed\n";
      continue;
    }

    CheckVisionEmergency(frame);

    if (grpc_.GetIsConnected())
    {
      // Write frame to MemoryVideoWriter
      memory_video_writer_->WriteFrame(frame);
      EncodeToMemory();
      frame_count_++;
      continue;
    }
    // 비디오 작성
    // out.write(frame);
    // EncodeToFile(out);
    // frame_count_++;
    cerr << "server disconnected " << endl;
    exit(1);
  }

  // 클라이언트의 스트리밍 완료
  grpc_.WriteDone();

  // Release the VideoCapture and close OpenCV window
  delete memory_video_writer_;
  cap.release();
}

void VideoStreamer::EncodeToMemory()
{
  // 경과 시간 계산
  double elapsed_seconds = (getTickCount() - start_tick_count_) / getTickFrequency();

  cout << "경과 시간 :" << elapsed_seconds << endl;
  // 지정된 시간 동안 촬영 후 전송
  if (elapsed_seconds >= kDurationSeconds)
  {
    int64 delay_start = getTickCount();

    grpc_.SetOther(pi_status_.CheckPiStatus(), is_emergency_);

    // Get the memory buffer
    uint8_t *buffer = memory_video_writer_->GetMemoryBuffer();

    // Set the buffer to the frame message
    grpc_.SetVideoFromMemory(buffer, memory_video_writer_->GetMemoryBufferSize());

    // Send mp4 to server
    grpc_.Write();

    memory_video_writer_->Reset(frame_count_ / kDurationSeconds, buffer);

    start_tick_count_ = getTickCount();
    frame_count_ = 0;
    cout << "\nmemory writer delay: "
         << (start_tick_count_ - delay_start) / getTickFrequency() << "\n\n";
  }
}

void VideoStreamer::EncodeToFile(VideoWriter &out)
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
    grpc_.SetVideoFromFile(buffer);

    grpc_.Write();

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
    buffer.resize(fileSize);                      // 버퍼 크기 조절
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
