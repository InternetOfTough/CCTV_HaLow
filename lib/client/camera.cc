#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;

// ī�޶� ��� ���� Ȯ��
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

    // ����� "detected=1"�� ���ԵǾ� �ִ��� Ȯ��
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
