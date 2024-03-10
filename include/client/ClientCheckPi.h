#ifndef CHECK_PI_STATUS_H
#define CHECK_PI_STATUS_H

#include <string>
#include <iostream>

using namespace std;

class ClientCheckPi
{

public:
    string CheckPiStatus();

private:
    string executeCommand(const char *command);
    string getWifiESSID();
    string getWifiSignalLevel();
    string getIPv4Address();
    string getIPv6Address();
    string getWifiChannel();
    string getWifiFrequency();
    string getWifiWidth();
    string getWifiInfo();
    string getNetworkTraffic();
    string getCamera();


    // cmd for status
    const char *kCmdSignal = "iwconfig wlan0";
    const char *kCmdIp = "ifconfig";
    const char *KCmdChannel = "iw wlan0 info";
    const char *KCmdTraffic = "ip -s -d link show wlan0";
    const char *kCmdCamera = "vcgencmd get_camera";
};

#endif