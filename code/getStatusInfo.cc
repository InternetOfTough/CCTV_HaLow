#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

void runSignalLevel();
void runNetworkTraffic();
void runCamera();

int main() {
    while (true) {
        // sinalLevel.cc ����
        cout << "sinalLevel.cc ���� ��..." << endl;
        thread t1(runSignalLevel);
        // 10�� ���
        this_thread::sleep_for(chrono::seconds(10));

        // networkTraffic.cc ����
        cout << "networkTraffic.cc ���� ��..." << endl;
        thread t2(runNetworkTraffic);
        // 10�� ���
        this_thread::sleep_for(chrono::seconds(10));

        // camera.cc ����
        cout << "camera.cc ���� ��..." << endl;
        thread t3(runCamera);
        // 60�� ���
        this_thread::sleep_for(chrono::seconds(60));

        // ������ ���� ���
        t1.join();
        t2.join();
        t3.join();
        
    }

    return 0;
}

// sinalLevel.cc ���� �Լ�
void runSignalLevel() {
    system("./sinalLevel");
}

// networkTraffic.cc ���� �Լ�
void runNetworkTraffic() {
    system("./networkTraffic");
}

// camera.cc ���� �Լ�
void runCamera() {
    system("./camera");
}
