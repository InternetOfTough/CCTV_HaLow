// VideoStreamer.cpp

#include <fstream>
#include "ClientVideoStreamer.h"

VideoStreamer::VideoStreamer(const string &server_address, char *pi_name)
    : stub_(Streaming::NewStub(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()))), pi_name_(pi_name)
{
  cout << "VideoStreamer instance generated" << endl;
}

VideoStreamer::~VideoStreamer()
{
  cout << "VideoStreamer instance deleted" << endl;
}

void VideoStreamer::StreamVideo()
{

  /*   jpg로 압축해서 전송!!
    Mat frame;
    //--- INITIALIZE VIDEOCAPTURE
    VideoCapture cap;
    // open the default camera using default API
    // cap.open(0);
    // OR advance usage: select any API backend
    // int deviceID = 0;        // 0 = open default camera
    // int apiID = cv::CAP_ANY; // 0 = autodetect default API
    // open selected camera using selected API
    // cap.open(deviceID, apiID);
    cap.open(-1);
    // check if we succeeded
    if (!cap.isOpened())
    {
      cerr << "ERROR! Unable to open camera\n";
      exit(1);
    }

    // Send frame to server
    // streaming::ServerMessage response;
    // ClientContext context;
    unique_ptr<grpc::ClientWriter<Frame>> writer(stub_->StreamVideo(&context, &response).release());

    while (cap.read(frame))
    {

      // // check if we succeeded
      if (frame.empty())
      {
        cerr << "ERROR! blank frame grabbed\n";
        break;
      }


      // Convert OpenCV Mat to gRPC Frame message
      Frame frame_message;
      vector<uchar> buffer;
      cv::imencode(".jpg", frame, buffer); // cv::Mat을 바이트 배열로 변환

      // Set the buffer to the frame message
      frame_message.mutable_data()->assign(buffer.begin(), buffer.end());

      // Send frame to server
      writer->Write(frame_message);

      // imshow("Camera Stream", frame);
      // if (waitKey(1) == 27) // Break the loop on ESC key press
        // break;
    }

    // 클라이언트의 스트리밍 완료
    writer->WritesDone();
    Status status = writer->Finish();

    if (!status.ok())
    {
      cerr << "Error streaming video: " << status.error_message() << endl;
    }
    // Release the VideoCapture and close OpenCV window
    cap.release();
    destroyAllWindows();

    */

  // mp4로 압축해서 전송!!

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
  int first_fps = 30;
  cap.set(CAP_PROP_FPS, first_fps);
  frameWidth = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));
  frameHeight = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));

  cout << "Width : " << frameWidth << endl;
  cout << "Height : " << frameHeight << endl;

  // MemoryVideoWriter setup
  memoryVideoWriter = new MemoryVideoWriter(frameWidth, frameHeight, first_fps);

  outputFileName.append(to_string(nameIndex));
  outputFileName.append(kFileType);

  // 비디오 작성자 생성
  VideoWriter out(outputFileName, VideoWriter::fourcc('a', 'v', 'c', '1'), first_fps, Size(frameWidth, frameHeight), true); // fps = 30, avc1-> H.264 코덱의 일종

  // 시작 시간 기록
  // Send mp4 to server
  cout << "\nconnecting....\n";
  unique_ptr<grpc::ClientWriter<Frame>> writer(stub_->streamVideo(&context, &response).release());

  startTickCount = getTickCount();
  while (true)
  {
    cap.read(frame);
    // check if we succeeded
    if (frame.empty())
    {
      cerr << "ERROR! blank frame grabbed\n";
      continue;
    }

    // 비디오 작성
    // out.write(frame);
    // encodeToFile(writer, out);

    // Write frame to MemoryVideoWriter
    memoryVideoWriter->WriteFrame(frame);
    encodeToMemory(writer);

    frameCount++;
  }

  // 클라이언트의 스트리밍 완료
  writer->WritesDone();
  Status status = writer->Finish();

  if (!status.ok())
  {
    cerr << "Error streaming video: " << status.error_message() << endl;
  }
  // Release the VideoCapture and close OpenCV window
  cap.release();
  destroyAllWindows();
}
void VideoStreamer::encodeToMemory(unique_ptr<grpc::ClientWriter<Frame>> &writer)
{
  // 경과 시간 계산
  double elapsedSeconds = (getTickCount() - startTickCount) / getTickFrequency();

  cout << "경과 시간 :" << elapsedSeconds << endl;
  // 지정된 시간 동안 캡처
  if (elapsedSeconds >= durationSeconds)
  {
    int64 delayStart = getTickCount();

    Frame frame_message;
    frame_message.set_name(pi_name_);
    frame_message.set_status(checkPiStatus());

    // Get the memory buffer
    uint8_t *buffer = memoryVideoWriter->GetMemoryBuffer();

    // Set the buffer to the frame message
    frame_message.mutable_data()->assign(reinterpret_cast<const char *>(buffer), memoryVideoWriter->GetMemoryBufferSize());
    cout << "buffer_size " << memoryVideoWriter->GetMemoryBufferSize() << endl;
    // Send mp4 to server
    if (!writer->Write(frame_message))
    {
      cerr << "server disconnected " << endl;
      exit(1);
    }

    memoryVideoWriter->reset(frameCount / durationSeconds, buffer);

    startTickCount = getTickCount();
    frameCount = 0;
    cout << "\nmemory writer delay: "
         << (startTickCount - delayStart) / getTickFrequency() << "\n\n";
  }
}

void VideoStreamer::encodeToFile(unique_ptr<grpc::ClientWriter<Frame>> &writer, VideoWriter &out)
{
  // 경과 시간 계산
  double elapsedSeconds = (getTickCount() - startTickCount) / getTickFrequency();

  cout << "경과 시간 :" << elapsedSeconds << endl;
  // 지정된 시간 동안 캡처하고 종료
  if (elapsedSeconds >= durationSeconds)
  {
    int64 delayStart = getTickCount();
    out.release(); // VideoWriter의 release 함수 호출

    // Convert OpenCV mp4 to gRPC Frame message
    Frame frame_message;
    vector<char> buffer;

    readFile(outputFileName, buffer);

    // Set the buffer to the frame message
    frame_message.mutable_data()->assign(buffer.begin(), buffer.end());

    if (!writer->Write(frame_message))
    {
      cerr << "server disconnected " << endl;
    }

    nameIndex++;
    outputFileName.clear();
    outputFileName.append(to_string(nameIndex));
    outputFileName.append(kFileType);
    out.open(outputFileName, VideoWriter::fourcc('a', 'v', 'c', '1'), frameCount / durationSeconds, Size(frameWidth, frameHeight), true);

    startTickCount = getTickCount();
    frameCount = 0;
    double delay = (startTickCount - delayStart) / getTickFrequency();
    cout << "videowrite delay: " << delay << endl;
  }
}

void VideoStreamer::readFile(string &filePath, vector<char> &buffer)
{
  // 파일을 바이너리 모드로 읽기
  ifstream inputFile(filePath, ios::binary);
  inputFile.clear(); // failbit를 재설정

  if (!inputFile.is_open())
  {
    cerr << "Error: Unable to open the MP4 file." << endl;
    return;
  }

  // 파일 크기 확인
  inputFile.seekg(0, ios::end);
  streampos fileSize = inputFile.tellg();
  inputFile.seekg(0, ios::beg);

  cout << "file size : " << fileSize << endl;

  // 파일 내용을 버퍼에 읽어오기
  if (fileSize > 0)
  {
    buffer.resize(fileSize);                     // 버퍼 크기 조절
    if (inputFile.read(buffer.data(), fileSize)) // buffer.data()는 vector가 관리하는 데이터 버퍼의 시작 주소
    {
      cout << "Read " << fileSize << " bytes from the MP4 file." << endl;
    }
    else
    {
      cerr << "Error: Failed to read from the " << filePath << " MP4 file." << endl;
      cerr << "Read operation status: " << inputFile.rdstate() << endl;
    }
  }
  else
  {
    cerr << "Error: File is empty or invalid." << endl;
  }

  // 파일 닫기
  inputFile.close();
}