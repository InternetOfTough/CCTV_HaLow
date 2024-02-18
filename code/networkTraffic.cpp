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

// ��Ʈ��ũ Ʈ���� ���� �Ľ�
string getNetworkTraffic() {
    string result;
    string cmd = "ip -s link";
    /*
    Ʈ���� �Ľ� ���� ����
    G1: �������̽� ��ȣ
    G2: �������̽� �̸�
    G3: ���ŵ� ��Ŷ ��
    G4: ���۵� ��Ŷ ��
    */
    regex pattern("\\d+:\\s+(\\w+):.*?RX:.*?\\d+.*?TX:.*?\\d+", regex::extended);
    // ��Ī ��ü ����
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
    // result���� ���ϰ� ��Ī�Ǵ� �κ� ã�Ƽ� traffic�� �߰�
    while (regex_search(result, matches, pattern)) {
        cout << traffic << endl;
        traffic += matches[0].str() + "\n";
        cout << traffic << endl;
        result = matches.suffix().str();
    }

    return traffic;
}
