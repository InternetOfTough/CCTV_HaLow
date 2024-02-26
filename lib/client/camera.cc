#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;

// 카메라 모듈 상태 확인
bool checkCamera() {
    string command = "vcgencmd get_camera";

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        cerr << "Error: Failed to run command." << endl;
        return false;
    }

    char buffer[128];
    string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr)
            result += buffer;
    }
    pclose(pipe);

    // 결과에 "detected=1"이 포함되어 있는지 확인
    return result.find("detected=1") != string::npos;
}

int main() {
    if (checkCamera()) {
        cout << "Camera module is working properly." << endl;
    } else {
        cout << "Camera module is not working." << endl;
    }

    return 0;
}
