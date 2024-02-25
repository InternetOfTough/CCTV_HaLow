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
    string cmd = "ip -s -d link show wlan0";

    regex pattern("RX:\\s+bytes\\s+packets\\s+errors\\s+dropped\\s+missed\\s+mcast\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s*"
                  ".*"
                  "TX:\\s+bytes\\s+packets\\s+errors\\s+dropped\\s+carrier\\s+collsns\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s*");
    smatch matches;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        cerr << "popen() failed!" << endl;
        return "";
    }

    char buffer[128];
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr)
            result += buffer;
    }
    pclose(pipe);

    string traffic;
    // result에서 패턴과 매칭되는 부분 찾아서 traffic에 추가
    if (regex_search(result, matches, pattern)) {
        traffic += "RX bytes: " + matches[1].str() + ", packets: " + matches[2].str() + ", errors: " + matches[3].str() +
		", dropped: " + matches[4].str() + ", missed: " + matches[5].str() + ", mcast: " + matches[6].str() + "\n";
        traffic += "TX bytes: " + matches[7].str() + ", packets: " + matches[8].str() + ", errors: " + matches[9].str() +
		", dropped: " + matches[10].str() + ", missed: " + matches[11].str() + ", macast: " + matches[12].str() + "\n";
    }

    return traffic;
}