#include <iostream>
#include <string>
#include <regex>

using namespace std;

string getNetworkTraffic();

int main() {
    string traffic = getNetworkTraffic();

    if (!traffic.empty()) {
        cout << "Network Traffic: " << endl;
        cout << traffic << endl;
    } else {
        cerr << "Failed to get network traffic information." << endl;
        return 1;
    }

    return 0;
}

// 네트워크 트래픽 정보 파싱
string getNetworkTraffic() {
    string result;
    string cmd = "ip -s link";
    /*
    트래픽 파싱 패턴 설정
    G1: 인터페이스 번호
    G2: 인터페이스 이름
    G3: 수신된 패킷 수
    G4: 전송된 패킷 수
    */
    regex pattern("\\d+:\\s+(\\w+):.*?RX:.*?\\d+.*?TX:.*?\\d+", regex::extended);
    // 매칭 객체 선언
    smatch matches;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        cerr << "popen() failed!" << endl;
        //return "";
    }

    char buffer[128];
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr)
            result += buffer;
    }
    pclose(pipe);

    string traffic;
    // result에서 패턴과 매칭되는 부분 찾아서 traffic에 추가
    while (regex_search(result, matches, pattern)) {
        cout << traffic << endl;
        traffic += matches[0].str() + "\n";
        cout << traffic << endl;
        result = matches.suffix().str();
    }

    return traffic;
}
