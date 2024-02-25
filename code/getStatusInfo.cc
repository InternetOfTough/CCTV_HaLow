#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

void runSignalLevel();
void runNetworkTraffic();

int main() {
    while (true) {
        // sinalLevel.cpp 실행
        cout << "sinalLevel.cpp 실행 중..." << endl;
        thread t1(runSignalLevel);

        // networkTraffic.cpp 실행
        cout << "networkTraffic.cpp 실행 중..." << endl;
        thread t2(runNetworkTraffic);

        // 스레드 종료 대기
        t1.join();
        t2.join();

        // 3초 대기
        this_thread::sleep_for(chrono::seconds(3));
    }

    return 0;
}

// sinalLevel.cpp 실행 함수
void runSignalLevel() {
    system("./sinalLevel");
}

// networkTraffic.cpp 실행 함수
void runNetworkTraffic() {
    system("./networkTraffic");
}
