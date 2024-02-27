#include <fstream>
#include <regex>
#include <chrono>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <array>
#include "ClientVideoStreamer.h"

// execute command
string VideoStreamer::executeCommand(const char* command) {
    array<char, 128> buffer;
    string result;
    shared_ptr<FILE> pipe(popen(command, "r"), pclose);
    if (!pipe) {
        throw runtime_error("popen() failed!");
    }
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr) {
            result += buffer.data();
        }
    }
    return result;
}

// parse signal level
string VideoStreamer::getSignalLevel() {
    string result;

    cout << "Extracting signal level info..." << endl;

    regex pattern("Signal level=(-?\\d+) dBm");
    smatch matches;

    result = executeCommand(cmd_signal);

    if (regex_search(result, matches, pattern)) {
        return matches[1].str();
    } else {
        cerr << "Failed to get signal level." << endl;
        return "failed";
    }
}

// parse network traffic
string VideoStreamer::getNetworkTraffic() {
    string result;

    cout << "Extracting network traffic info..." << endl;

    regex pattern("RX:\\s+bytes\\s+packets\\s+errors\\s+dropped\\s+missed\\s+mcast\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s*"
                  ".*"
                  "TX:\\s+bytes\\s+packets\\s+errors\\s+dropped\\s+carrier\\s+collsns\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s*");
    smatch matches;

    result = executeCommand(cmd_traffic);

    string traffic;
    if (regex_search(result, matches, pattern)) {
        traffic += "RX bytes: " + matches[1].str() + "\n";
        //+ ", packets: " + matches[2].str() + ", errors: " + matches[3].str() + ", dropped: " + matches[4].str() + ", missed: " + matches[5].str() + ", mcast: " + matches[6].str() + "\n";
        traffic += "TX bytes: " + matches[7].str() + "\n";
        //", packets: " + matches[8].str() + ", errors: " + matches[9].str() + ", dropped: " + matches[10].str() + ", missed: " + matches[11].str() + ", macast: " + matches[12].str() + "\n";
        return traffic;
    } else {
        cerr << "Failed to get network traffic information." << endl;
        return "failed";
    }
}

// parse camera state
string VideoStreamer::getCamera() {
    string result;

    cout << "Extracting camera state info..." << endl;

    result = executeCommand(cmd_camera);

    if (result.find("detected=1") != string::npos) {
        return "connected";
    } else {
        cerr << "Camera module is not working." << endl;
        return "failed";
    }
}

string VideoStreamer::CheckPiStatus()
{
    string status, signalLevel, networkTraffic, cameraStatus;
    // create log file
    ofstream logFile("/home/pi/log/getStatusInfoLog.txt");

    // redirect output and error stream
    streambuf* coutStreamBuf = cout.rdbuf(logFile.rdbuf());
    streambuf* cerrStreamBuf = cerr.rdbuf(logFile.rdbuf());

    try {
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

    // recover redirection stream
    cout.rdbuf(coutStreamBuf);
    cerr.rdbuf(cerrStreamBuf);

    // close log file
    logFile.close();

    return status;
}
