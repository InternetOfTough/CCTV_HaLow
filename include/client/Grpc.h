#ifndef CLIENT_GPRC_H
#define CLIENT_GRPC_H

#include <thread>
#include <iostream>
#include <string>
#include <grpcpp/grpcpp.h>
#include "streaming.grpc.pb.h" // gRPC 프로토콜 파일을 포함해야 합니다
using grpc::Channel;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using streaming::Frame;
using streaming::Streaming;
using streaming::ServerMessage;

using namespace std;

class Grpc
{

public:
    Grpc(const string server_address, char *pi_name);
    ~Grpc();
    void SetOther(string pi_status, string &is_emergency);
    void SetVideoFromMemory(uint8_t *buffer, int &buffer_size);
    void SetVideoFromFile(vector<char> &buffer);
    void Write();
    void WriteDone();
    void Finish();
    bool GetIsConnected();
    void SetIsConnected(bool is_connected);
    void GrpcThread();

private:
    enum class Type
    {
        READ = 1,
        WRITE = 2,
        CONNECT = 3,
        WRITES_DONE = 4,
        FINISH = 5
    };

    CompletionQueue cq_;
    ServerMessage response_;
    Frame frame_message_;
    ClientContext context_;
    Status status_;
    unique_ptr<Streaming::Stub> stub_;
    unique_ptr<thread> grpc_thread_;
    unique_ptr<grpc::ClientAsyncWriter<Frame>> writer_;
    const string kPiName;

    bool is_connected_ = true;
};

#endif