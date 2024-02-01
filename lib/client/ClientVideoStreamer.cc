// VideoStreamer.cpp

#include <fstream>
#include "ClientVideoStreamer.h"

using namespace cv;
using namespace std;

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
    streaming::EmptyMessage response;
    ClientContext context;
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
  int frameWidth = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));
  int frameHeight = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));

  cout << "Width : " << frameWidth << endl;
  cout << "Height : " << frameHeight << endl;
  // 비디오 작성자 생성
  unsigned int nameIndex = 1;
  const string firstName("1.mp4");
  const string fileType = ".mp4";
  string outputFileName;
  outputFileName.append(to_string(nameIndex));
  outputFileName.append(fileType);
  // VideoWriter out(outputFileName, VideoWriter::fourcc('H', '2', '6', '4'), 30, Size(frameWidth, frameHeight));
  VideoWriter out(outputFileName, VideoWriter::fourcc('a', 'v', 'c', '1'), 30, Size(frameWidth, frameHeight), true); // fps = 30, avc1-> H.264 코덱의 일종

  // 시작 시간 기록
  const int durationSeconds = 10;
  int64 startTickCount = getTickCount();
  // Send mp4 to server
  streaming::EmptyMessage response;
  ClientContext context;
  unique_ptr<grpc::ClientWriter<Frame>> writer(stub_->StreamVideo(&context, &response).release());

  while (cap.read(frame))
  {

    // // check if we succeeded
    if (frame.empty())
    {
      cerr << "ERROR! blank frame grabbed\n";
      break;
    }

    // 비디오 작성
    out.write(frame);

    // 경과 시간 계산
    double elapsedSeconds = (getTickCount() - startTickCount) / getTickFrequency();

    cout << "경과 시간 :" << elapsedSeconds << endl;
    // 지정된 시간 동안 캡처하고 종료
    if (elapsedSeconds >= durationSeconds)
    {
      out.release(); // VideoWriter의 release 함수 호출

      // Convert OpenCV mp4 to gRPC Frame message
      Frame frame_message;
      vector<char> buffer;

      readFile(outputFileName, buffer);

      // Set the buffer to the frame message
      frame_message.mutable_data()->assign(buffer.begin(), buffer.end());

      // Send mp4 to server
      writer->Write(frame_message);
      nameIndex++;
      outputFileName.clear();
      outputFileName.append(to_string(nameIndex));
      outputFileName.append(fileType);
      // out.open(outputFileName, VideoWriter::fourcc('H', '2', '6', '5'), 30, Size(frameWidth, frameHeight));
      out.open(outputFileName, VideoWriter::fourcc('a', 'v', 'c', '1'), 30, Size(frameWidth, frameHeight), true);
      startTickCount = getTickCount();
    }
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
  // delete out;
  destroyAllWindows();
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