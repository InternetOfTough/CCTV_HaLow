// VideoStreamer.cpp

#include <fstream>
#include "ClientVideoStreamer.h"

VideoStreamer::VideoStreamer(const string &server_address)
    : stub_(Streaming::NewStub(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials())))
{
  std::cout << "VideoStreamer instance generated" << std::endl;
}

VideoStreamer::~VideoStreamer()
{
  std::cout << "VideoStreamer instance deleted" << std::endl;
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
    // streaming::EmptyMessage response;
    // ClientContext context;
    std::unique_ptr<grpc::ClientWriter<Frame>> writer(stub_->StreamVideo(&context, &response).release());

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
      std::vector<uchar> buffer;
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
      std::cerr << "Error streaming video: " << status.error_message() << std::endl;
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

  // 카메라 프레임의 너비와 높이 가져오기
  frameWidth = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));
  frameHeight = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));

  cout << "Width : " << frameWidth << endl;
  cout << "Height : " << frameHeight << endl;

  // MemoryVideoWriter setup
  memoryVideoWriter = new MemoryVideoWriter(frameWidth, frameHeight, 10);

  outputFileName.append(to_string(nameIndex));
  outputFileName.append(fileType);

  // 비디오 작성자 생성
  // int capture_fps = 30;
  // cap.set(CAP_PROP_FPS, capture_fps);
  VideoWriter out(outputFileName, VideoWriter::fourcc('a', 'v', 'c', '1'), 10, Size(frameWidth, frameHeight), true); // fps = 30, avc1-> H.264 코덱의 일종

  // 시작 시간 기록
  startTickCount = getTickCount();
  // Send mp4 to server
  // streaming::EmptyMessage response;
  // ClientContext context;
  unique_ptr<grpc::ClientWriter<Frame>> writer(stub_->StreamVideo(&context, &response).release());

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
    std::cerr << "Error streaming video: " << status.error_message() << std::endl;
  }
  // Release the VideoCapture and close OpenCV window
  cap.release();
  destroyAllWindows();
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

    writer->Write(frame_message);
    nameIndex++;
    outputFileName.clear();
    outputFileName.append(to_string(nameIndex));
    outputFileName.append(fileType);
    out.open(outputFileName, VideoWriter::fourcc('a', 'v', 'c', '1'), frameCount / durationSeconds, Size(frameWidth, frameHeight), true);

    startTickCount = getTickCount();
    frameCount = 0;
    double delay = (startTickCount - delayStart) / getTickFrequency();
    cout << "videowrite delay: " << delay << endl;
  }
}

void VideoStreamer::encodeToMemory(unique_ptr<grpc::ClientWriter<Frame>> &writer)
{
  // 경과 시간 계산
  double elapsedSeconds = (getTickCount() - startTickCount) / getTickFrequency();

  cout << "경과 시간 :" << elapsedSeconds << endl;
  // 지정된 시간 동안 캡처하고 종료
  if (elapsedSeconds >= durationSeconds)
  {
    int64 delayStart = getTickCount();
    // Convert OpenCV mp4 to gRPC Frame message
    Frame frame_message;

    // Get the memory buffer
    uint8_t *buffer = memoryVideoWriter->GetMemoryBuffer();

    // Set the buffer to the frame message
    frame_message.mutable_data()->assign(reinterpret_cast<const char *>(buffer), memoryVideoWriter->GetMemoryBufferSize());
    std::cout << "buffer_size " << memoryVideoWriter->GetMemoryBufferSize() << std::endl;
    // Send mp4 to server
    writer->Write(frame_message);

    delete (buffer);
    // delete (memoryVideoWriter);
    std::cout << "here" << std::endl;
    memoryVideoWriter->reset(frameCount / durationSeconds);
    // memoryVideoWriter = new MemoryVideoWriter(frameWidth, frameHeight, frameCount / durationSeconds);

    startTickCount = getTickCount();
    frameCount = 0;
    cout << "memorywriter delay: " << (startTickCount - delayStart) / getTickFrequency() << endl;
  }
}
void VideoStreamer::readFile(std::string &filePath, std::vector<char> &buffer)
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
    if (inputFile.read(buffer.data(), fileSize)) // buffer.data()는 std::vector가 관리하는 데이터 버퍼의 시작 주소
    {
      std::cout << "Read " << fileSize << " bytes from the MP4 file." << std::endl;
    }
    else
    {
      std::cerr << "Error: Failed to read from the " << filePath << " MP4 file." << std::endl;
      cerr << "Read operation status: " << inputFile.rdstate() << endl;
    }
  }
  else
  {
    std::cerr << "Error: File is empty or invalid." << std::endl;
  }

  // 파일 닫기
  inputFile.close();
}