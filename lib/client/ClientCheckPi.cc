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

// parse wifi ESSID
string VideoStreamer::getWifiESSID() {
    const regex essidRegex("ESSID:\"([^\"]+)\"");
    string result = executeCommand(cmd_signal);
    smatch matches;
    return regex_search(result, matches, essidRegex) ? matches[1].str() : "failed";
}

// parse wifi signal level
string VideoStreamer::getWifiSignalLevel() {
    const regex signalLevelRegex("Signal level=(-?\\d+) dBm");
    string result = executeCommand(cmd_signal);
    smatch matches;
    return regex_search(result, matches, signalLevelRegex) ? matches[1].str() : "failed";
}

// parse IPv4 address
string VideoStreamer::getIPv4Address() {
    const regex ipv4Regex("inet (\\S+)");
    string result = executeCommand(cmd_ip);
    smatch matches;
    return regex_search(result, matches, ipv4Regex) ? matches[1].str() : "failed";
}

// parse IPv6 address
string VideoStreamer::getIPv6Address() {
    const regex ipv6Regex("inet6 (\\S+)");
    string result = executeCommand(cmd_ip);
    smatch matches;
    return regex_search(result, matches, ipv6Regex) ? matches[1].str() : "failed";
}

// parse wifi channel
string VideoStreamer::getWifiChannel() {
    const regex channelRegex("channel (\\d+) \\((\\d+) MHz\\), width: (\\d+) MHz");
    string result = executeCommand(cmd_channel);
    smatch matches;
    return regex_search(result, matches, channelRegex) ? matches[1].str() : "failed";
}

// parse wifi frequency
string VideoStreamer::getWifiFrequency() {
    const regex channelRegex("channel (\\d+) \\((\\d+) MHz\\), width: (\\d+) MHz");
    string result = executeCommand(cmd_channel);
    smatch matches;
    return regex_search(result, matches, channelRegex) ? matches[2].str() : "failed";
}

// parse wifi width
string VideoStreamer::getWifiWidth() {
    const regex channelRegex("channel (\\d+) \\((\\d+) MHz\\), width: (\\d+) MHz");
    string result = executeCommand(cmd_channel);
    smatch matches;
    return regex_search(result, matches, channelRegex) ? matches[3].str() : "failed";
}

// get wifi halow info
string VideoStreamer::getWifiInfo() {
    cout << "Extracting wifi halow info..." << endl;

    string wifi_info;
    wifi_info += "ESSID: " + getWifiESSID() + "\n";
    wifi_info += "Signal level: " + getWifiSignalLevel() + "\n";
    wifi_info += "IPv4: " + getIPv4Address() + "\n";
    wifi_info += "IPv6: " + getIPv6Address() + "\n";
    wifi_info += "Channel: " + getWifiChannel() + "\n";
    wifi_info += "Frequency: " + getWifiFrequency() + "\n";
    wifi_info += "Width: " + getWifiWidth() + "\n";
    return wifi_info;
}

// parse network traffic
string getNetworkTraffic() {
    cout << "Extracting network traffic info..." << endl;

    static string prevResult;
    static chrono::steady_clock::time_point prevTime = chrono::steady_clock::now();
    static long long prevRxBytes = 0;
    static long long prevTxBytes = 0;

    string result = executeCommand(cmd_traffic);

    // parse RX and TX bytes
    regex pattern("RX:\\s+bytes\\s+packets\\s+errors\\s+dropped\\s+missed\\s+mcast\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s*"
                  ".*"
                  "TX:\\s+bytes\\s+packets\\s+errors\\s+dropped\\s+carrier\\s+collsns\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s*");    
    smatch matches;
    if (regex_search(result, matches, pattern)) {
        long long rxBytes = stoll(matches[1].str());
        long long txBytes = stoll(matches[2].str());

        auto currentTime = chrono::steady_clock::now();
        // convert milliseconds to seconds
        auto elapsedTime = chrono::duration_cast<chrono::milliseconds>(currentTime - prevTime).count() / 1000.0;

        // calculate Kbps for RX and TX
        double rxKbps = (rxBytes - prevRxBytes) * 8.0 / elapsedTime;
        double txKbps = (txBytes - prevTxBytes) * 8.0 / elapsedTime;

        // update previous values
        prevResult = result;
        prevTime = currentTime;
        prevRxBytes = rxBytes;
        prevTxBytes = txBytes;

        return "RX Kbps: " + to_string(rxKbps) + "\nTX Kbps: " + to_string(txKbps) + "\n";
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
    string status, wifiInfo, networkTraffic, cameraStatus;
    // create log file
    ofstream logFile("/home/pi/log/getStatusInfoLog.txt");

    // redirect output and error stream
    streambuf* coutStreamBuf = cout.rdbuf(logFile.rdbuf());
    streambuf* cerrStreamBuf = cerr.rdbuf(logFile.rdbuf());

    try {
        wifiInfo = getWifiInfo();
        cout << "wifi info: \n" + wifiInfo << endl;

        networkTraffic = getNetworkTraffic();
        cout << "network traffic: \n" + networkTraffic << endl;

        cameraStatus = getCamera();
        cout << "camera state: " + cameraStatus << endl;

        status += wifiInfo + "\n";
        status += "camera: " + cameraStatus + "\n";
        status += networkTraffic + "\n";

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
