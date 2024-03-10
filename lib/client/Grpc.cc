#include "Grpc.h"

Grpc::Grpc(const string server_address, char *pi_name) : stub_(Streaming::NewStub(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()))),
                                                         grpc_thread_(make_unique<thread>(&Grpc::GrpcThread, this)),
                                                         writer_(stub_->AsyncstreamVideo(&context_, &response_, &cq_, reinterpret_cast<void *>(Type::CONNECT))),
                                                         kPiName(pi_name)
{
    cout << "Grpc instance generated" << endl;
}
Grpc::~Grpc()
{
    cout << "Shutting down client...." << endl;
    cout << "Grpc instance deleted" << endl;
    cq_.Shutdown();
    grpc_thread_->join();
}

void Grpc::SetOther(string pi_status, string &is_emergency)
{
    frame_message_.set_name(kPiName);
    frame_message_.set_status(pi_status);
    frame_message_.set_vision(is_emergency);
}

void Grpc::SetVideoFromMemory(uint8_t *buffer, int &buffer_size)
{
    frame_message_.mutable_data()->assign(reinterpret_cast<const char *>(buffer), buffer_size);
    cout << "buffer_size " << buffer_size << endl;
}
void Grpc::SetVideoFromFile(vector<char> &buffer)
{
    frame_message_.mutable_data()->assign(buffer.begin(), buffer.end());
}
void Grpc::Write()
{
    // if (!writer_->Write(frame_message_)) //동기식
    // {
    //   cerr << "server disconnected " << endl;
    //   exit(1);
    // }

    writer_->Write(frame_message_, reinterpret_cast<void *>(Type::WRITE)); // 비동기식
    frame_message_.Clear();
}

void Grpc::WriteDone()
{
    // 클라이언트의 스트리밍 완료
    writer_->WritesDone(reinterpret_cast<void *>(Type::WRITES_DONE));
}

void Grpc::Finish()
{
    writer_->Finish(&status_, reinterpret_cast<void *>(Type::FINISH)); // async method to receive server msg(return value)
}

bool Grpc::GetIsConnected()
{
    return is_connected_;
}

void Grpc::SetIsConnected(bool is_connected)
{
    is_connected_ = is_connected;
}

void Grpc::GrpcThread()
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

        if (!ok && static_cast<Type>(reinterpret_cast<long>(got_tag)) == Type::CONNECT)
        {
            cout << "server disconnected..." << endl;
            exit(1);
            // system("sudo reboot");
            continue;
        }

        if (!ok && static_cast<Type>(reinterpret_cast<long>(got_tag)) == Type::WRITE)
        {
            is_connected_ = false;
            cout << "write failed. \nvideo is stored in local storage now." << endl;
            // system("sudo reboot");
            continue;
        }
    }
}