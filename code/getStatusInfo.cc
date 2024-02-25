#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

void runSignalLevel();
void runNetworkTraffic();

int main() {
    while (true) {
        // sinalLevel.cpp ����
        cout << "sinalLevel.cpp ���� ��..." << endl;
        thread t1(runSignalLevel);

        // networkTraffic.cpp ����
        cout << "networkTraffic.cpp ���� ��..." << endl;
        thread t2(runNetworkTraffic);

        // ������ ���� ���
        t1.join();
        t2.join();

        // 3�� ���
        this_thread::sleep_for(chrono::seconds(3));
    }

    return 0;
}

// sinalLevel.cpp ���� �Լ�
void runSignalLevel() {
    system("./sinalLevel");
}

// networkTraffic.cpp ���� �Լ�
void runNetworkTraffic() {
    system("./networkTraffic");
}
