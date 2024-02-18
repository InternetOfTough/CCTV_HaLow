#include <iostream>
#include <fstream>
#include <string>
#include <regex>

using namespace std;

int getSignalLevel(const string& interface);

int main() {
    string interface = "wlan0"; // 무선랜 인터페이스

    int signalLevel = getSignalLevel(interface);

    if (signalLevel != -1) {
        cout << "Signal Level: " << signalLevel << " dBm" << endl;
    } else {
        cerr << "Failed to get signal level." << endl;
        return 1;
    }

    return 0;
}

// 신호세기 정보 파싱
int getSignalLevel(const string& interface) {
    string result;
    string cmd = "iwconfig " + interface + " 2>&1";
    // 신호세기 패턴설정: 음수 존재할 수 있음, 하나 이상의 숫자 존재해야함 
    regex pattern(".*Signal level=(-?\\d+) dBm.*");
    // 패턴과 매칭 결과를 저장할 객체 선언
    smatch matches;

    // 리눅스 명령 실행결과를 파일로 읽기위한 파이프 생성
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        cerr << "popen() failed!" << endl;
        return -1;
    }

    // 파이프에서 읽고 result에 저장
    char buffer[128];
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr)
            result += buffer;
    }
    pclose(pipe);

    // result에서 신호세기 패턴 찾고, 일치하는 부분 가져오기
    if (regex_search(result, matches, pattern)) {
        return stoi(matches[1]);
    } else {
        return -1; // 신호 세기를 찾을 수 없음
    }
}
