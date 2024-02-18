#include <iostream>
#include <fstream>
#include <string>
#include <regex>

using namespace std;

int getSignalLevel(const string& interface);

int main() {
    string interface = "wlan0"; // ������ �������̽�

    int signalLevel = getSignalLevel(interface);

    if (signalLevel != -1) {
        cout << "Signal Level: " << signalLevel << " dBm" << endl;
    } else {
        cerr << "Failed to get signal level." << endl;
        return 1;
    }

    return 0;
}

// ��ȣ���� ���� �Ľ�
int getSignalLevel(const string& interface) {
    string result;
    string cmd = "iwconfig " + interface + " 2>&1";
    // ��ȣ���� ���ϼ���: ���� ������ �� ����, �ϳ� �̻��� ���� �����ؾ��� 
    regex pattern(".*Signal level=(-?\\d+) dBm.*");
    // ���ϰ� ��Ī ����� ������ ��ü ����
    smatch matches;

    // ������ ��� �������� ���Ϸ� �б����� ������ ����
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        cerr << "popen() failed!" << endl;
        return -1;
    }

    // ���������� �а� result�� ����
    char buffer[128];
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr)
            result += buffer;
    }
    pclose(pipe);

    // result���� ��ȣ���� ���� ã��, ��ġ�ϴ� �κ� ��������
    if (regex_search(result, matches, pattern)) {
        return stoi(matches[1]);
    } else {
        return -1; // ��ȣ ���⸦ ã�� �� ����
    }
}
