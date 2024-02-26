#include <iostream>
#include <fstream>
#include <regex>
#include <chrono>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <array>
#include <thread>

using namespace std;

// execute command
string executeCommand(const string& command) {
    string result;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        cerr << "Error: Failed to run command." << endl;
    }

    char buffer[128];
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr)
            result += buffer;
    }
    pclose(pipe);

    return result;
}

// parse signal level
string getSignalLevel() {
    string result;
    const string interface = "wlan0";
    const string cmd = "iwconfig " + interface + " 2>&1";

    cout << "Extracting sinal level info..." << endl;

    regex pattern("Tx-Power=(\\d+) dBm");
    smatch matches;

    result = executeCommand(cmd);

    if (regex_search(result, matches, pattern)) {
        return matches[1].str();
    } else {
        cerr << "Failed to get signal level." << endl;
        return "failed";
    }
}

// parse network traffic
string getNetworkTraffic() {
    string result;
    const string cmd = "ip -s -d link show wlan0";

    cout << "Extracting network traffic info..." << endl;

    regex pattern("RX:\\s+bytes\\s+packets\\s+errors\\s+dropped\\s+missed\\s+mcast\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s*"
                  ".*"
                  "TX:\\s+bytes\\s+packets\\s+errors\\s+dropped\\s+carrier\\s+collsns\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s*");
    smatch matches;

    result = executeCommand(cmd);

    string traffic;
    if (regex_search(result, matches, pattern)) {
        traffic += "RX bytes: " + matches[1].str() + "\n";
        //+ ", packets: " + matches[2].str() + ", errors: " + matches[3].str() + ", dropped: " + matches[4].str() + ", missed: " + matches[5].str() + ", mcast: " + matches[6].str() + "\n";
        traffic += "TX bytes: " + matches[7].str() + + "\n";
        //", packets: " + matches[8].str() + ", errors: " + matches[9].str() + ", dropped: " + matches[10].str() + ", missed: " + matches[11].str() + ", macast: " + matches[12].str() + "\n";
        return traffic;
    } else {
        cerr << "Failed to get network traffic information." << endl;
        return "failed";
    }
}

// parse camera state
string getCamera() {
    string result;
    const string cmd = "vcgencmd get_camera";

    cout << "Extracting camera state info..." << endl;

    result = executeCommand(cmd);

    if (result.find("detected=1") != string::npos) {
        return "connected";
    } else {
        cerr << "Camera module is not working." << endl;
        return "failed";
    }
}

int main() {
    // create log file
    ofstream logFile("/home/pi/log/getStatusInfoLog.txt");

    // redirect output and error stream
    streambuf* coutStreamBuf = cout.rdbuf(logFile.rdbuf());
    streambuf* cerrStreamBuf = cerr.rdbuf(logFile.rdbuf());

while (true) {
    try {
        string signalLevel, networkTraffic, cameraStatus, status;

        signalLevel = getSignalLevel();
        cout << "wifi signal: " + signalLevel << endl;

        networkTraffic = getNetworkTraffic();
        cout << "network traffic: " + networkTraffic << endl;

        cameraStatus = getCamera();
        cout << "camera state: " + cameraStatus << endl;

        status += "wifi: " + signalLevel + "\n";
        status += "camera: " + cameraStatus + "\n";
        status += "traffic: " + networkTraffic + "\n";

        cout << "Pi Status:\n" << status << endl;
    } catch (const exception& e) {
        cerr << "error occurred: " << e.what() << endl;
    }
    this_thread::sleep_for(chrono::seconds(30)); 
}
    // recover redirection stream
    cout.rdbuf(coutStreamBuf);
    cerr.rdbuf(cerrStreamBuf);

    // close log file
    logFile.close();

    return 0;
}
