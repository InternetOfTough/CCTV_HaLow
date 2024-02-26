#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

void runSignalLevel();
void runNetworkTraffic();
void runCamera();

int main() {
    while (true) {
        // sinalLevel.cc 실행
        cout << "sinalLevel.cc 실행 중..." << endl;
        thread t1(runSignalLevel);
        // 10초 대기
        this_thread::sleep_for(chrono::seconds(10));

        // networkTraffic.cc 실행
        cout << "networkTraffic.cc 실행 중..." << endl;
        thread t2(runNetworkTraffic);
        // 10초 대기
        this_thread::sleep_for(chrono::seconds(10));

        // camera.cc 실행
        cout << "camera.cc 실행 중..." << endl;
        thread t3(runCamera);
        // 60초 대기
        this_thread::sleep_for(chrono::seconds(60));

        // 스레드 종료 대기
        t1.join();
        t2.join();
        t3.join();
        
    }

    return 0;
}

// sinalLevel.cc 실행 함수
void runSignalLevel() {
    system("./sinalLevel");
}

// networkTraffic.cc 실행 함수
void runNetworkTraffic() {
    system("./networkTraffic");
}

// camera.cc 실행 함수
void runCamera() {
    system("./camera");
}
