#include <iostream>
#include <fstream>
#include <regex>
#include <chrono>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <array>

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

// parse wifi ESSID
string getWifiESSID() {
    const string cmd = "iwconfig wlan0";
    const regex essidRegex("ESSID:\"([^\"]+)\"");
    string result = executeCommand(cmd);
    smatch matches;
    return regex_search(result, matches, essidRegex) ? matches[1].str() : "failed";
}

// parse wifi signal level
string getWifiSignalLevel() {
    const string cmd = "iwconfig wlan0";
    const regex signalLevelRegex("Signal level=(-?\\d+) dBm");
    string result = executeCommand(cmd);
    smatch matches;
    return regex_search(result, matches, signalLevelRegex) ? matches[1].str() : "failed";
}

// parse IPv4 address
string getIPv4Address() {
    const string cmd = "ifconfig";
    const regex ipv4Regex("inet (\\S+)");
    string result = executeCommand(cmd);
    smatch matches;
    return regex_search(result, matches, ipv4Regex) ? matches[1].str() : "failed";
}

// parse IPv6 address
string getIPv6Address() {
    const string cmd = "ifconfig";
    const regex ipv6Regex("inet6 (\\S+)");
    string result = executeCommand(cmd);
    smatch matches;
    return regex_search(result, matches, ipv6Regex) ? matches[1].str() : "failed";
}

// parse wifi channel
string getWifiChannel() {
    const string cmd = "iw wlan0 info";
    const regex channelRegex("channel (\\d+) \\((\\d+) MHz\\), width: (\\d+) MHz");
    string result = executeCommand(cmd);
    smatch matches;
    return regex_search(result, matches, channelRegex) ? matches[1].str() : "failed";
}

// parse wifi frequency
string getWifiFrequency() {
    const string cmd = "iw wlan0 info";
    const regex channelRegex("channel (\\d+) \\((\\d+) MHz\\), width: (\\d+) MHz");
    string result = executeCommand(cmd);
    smatch matches;
    return regex_search(result, matches, channelRegex) ? matches[2].str() : "failed";
}

// parse wifi width
string getWifiWidth() {
    const string cmd = "iw wlan0 info";
    const regex channelRegex("channel (\\d+) \\((\\d+) MHz\\), width: (\\d+) MHz");
    string result = executeCommand(cmd);
    smatch matches;
    return regex_search(result, matches, channelRegex) ? matches[3].str() : "failed";
}

// get wifi halow info
string getWifiInfo() {
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
    const string cmd = "ip -s -d link show wlan0";

    cout << "Extracting network traffic info..." << endl;

    static string prevResult; // Previous traffic data
    static chrono::steady_clock::time_point prevTime = chrono::steady_clock::now(); // Time of previous measurement
    static long long prevRxBytes = 0; // Previous RX bytes
    static long long prevTxBytes = 0; // Previous TX bytes

    string result = executeCommand(cmd);

    // Parse RX and TX bytes
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
        double rxKbps = (rxBytes - prevRxBytes) * 8.0 / elapsedTime; // 8 bits in a byte
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

/*
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
        traffic += "TX bytes: " + matches[7].str() + "\n";
        //", packets: " + matches[8].str() + ", errors: " + matches[9].str() + ", dropped: " + matches[10].str() + ", missed: " + matches[11].str() + ", macast: " + matches[12].str() + "\n";
        return traffic;
    } else {
        cerr << "Failed to get network traffic information." << endl;
        return "failed";
    }
}
*/

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

    try {
        string wifiInfo, networkTraffic, cameraStatus, status;

        wifiInfo = getWifiInfo();
        cout << "wifi info: \n" + wifiInfo << endl;

        networkTraffic = getNetworkTraffic();
        cout << "network traffic: " + networkTraffic << endl;

        cameraStatus = getCamera();
        cout << "camera state: " + cameraStatus << endl;

        status += wifiInfo + "\n";
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

    return 0;
}
