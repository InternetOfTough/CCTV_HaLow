#include <iostream>
#include <thread>
#include <chrono>
#include <cstdio>
#include <memory>
#include <array>
#include "ClientVideoStreamer.h"

using namespace std;

// 명령어 실행 후 결과를 캡처하여 문자열로 반환하는 함수
string executeCommand(const char* command) {
    array<char, 128> buffer;
    string result;
    shared_ptr<FILE> pipe(popen(command, "r"), pclose);
    if (!pipe) {
        throw runtime_error("popen() failed!");
    }
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr) {
            result += buffer.data();
        }
    }
    return result;
}

// sinalLevel.cc 실행 함수
void runSignalLevel(string& signalLevel) {
    cout << "sinalLevel.cc 실행 중..." << endl;
    while (true) {
        try {
            signalLevel = executeCommand("./sinalLevel");
            cout << signalLevel << endl;
            this_thread::sleep_for(chrono::seconds(30));
        } catch (const exception& e) {
            // 에러 발생 시, 해당 스레드 종료
            cerr << "sinalLevel.cc 실행 중 오류 발생: " << e.what() << endl;
            return;
        }
    }
}

// networkTraffic.cc 실행 함수
void runNetworkTraffic(string& networkTraffic) {
    cout << "networkTraffic.cc 실행 중..." << endl;
    while (true) {
        try {
            networkTraffic = executeCommand("./networkTraffic");
            cout << networkTraffic << endl;
            this_thread::sleep_for(chrono::seconds(30));
        } catch (const exception& e) {
            cerr << "networkTraffic.cc 실행 중 오류 발생: " << e.what() << endl;
            return;
        }
    }
}

// camera.cc 실행 함수
void runCamera(string& cameraStatus) {
    cout << "camera.cc 실행 중..." << endl;
    while (true) {
        try {
            cameraStatus = executeCommand("./camera");
            cout << cameraStatus << endl;
            this_thread::sleep_for(chrono::minutes(1));
        } catch (const exception& e) {
            cerr << "camera.cc 실행 중 오류 발생: " << e.what() << endl;
            return;
        }
    }
}

string VideoStreamer::CheckPiStatus()
{
    string signalLevel, networkTraffic, cameraStatus, status;
    thread t1(runSignalLevel, ref(signalLevel));
    thread t2(runNetworkTraffic, ref(networkTraffic));
    thread t3(runCamera, ref(cameraStatus));

    // 각 스레드가 종료될 때까지 대기
    t1.join();
    t2.join();
    t3.join();

    status += "wifi: " + signalLevel + "\n";
    status += "camera: " + cameraStatus + "\n";
    status += "traffic: " + networkTraffic + "\n";
    
    return status;
}

int main() {
    VideoStreamer streamer;
    string status = streamer.CheckPiStatus();
    cout << "Pi Status:\n" << status << endl;

    return 0;
}
