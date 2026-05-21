#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <memory>
#include <array>
#include <sstream>
#include <algorithm>
#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

struct Device {
    string ip;
    string mac;
    string vendor;
};

mutex data_mutex;
vector<Device> devices;
bool is_scanning = false;
string last_scan_time = "Never";

string exec(const char* cmd) {
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) return "";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

string getCurrentTime() {
    auto now = chrono::system_clock::now();
    time_t now_time = chrono::system_clock::to_time_t(now);
    char buf[80];
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now_time));
    return string(buf);
}

void runScan() {
    {
        lock_guard<mutex> lock(data_mutex);
        if (is_scanning) return;
        is_scanning = true;
    }

    string output = exec("arp-scan --localnet -q");
    
    vector<Device> new_devices;
    istringstream stream(output);
    string line;
    
    while (getline(stream, line)) {
        if (line.find(":") != string::npos && line.find(".") != string::npos) {
            Device dev;
            istringstream line_stream(line);
            string ip, mac, vendor_part;
            
            line_stream >> dev.ip >> dev.mac;
            
            string vendor = "";
            while (line_stream >> vendor_part) {
                vendor += vendor_part + " ";
            }
            if (!vendor.empty()) vendor.pop_back();
            
            dev.vendor = vendor.empty() ? "Unknown Device" : vendor;
            new_devices.push_back(dev);
        }
    }

    lock_guard<mutex> lock(data_mutex);
    devices = new_devices;
    last_scan_time = getCurrentTime();
    is_scanning = false;
}

void autoScanLoop() {
    while (true) {
        runScan();
        this_thread::sleep_for(chrono::minutes(2));
    }
}

int main() {
    thread t_scan(autoScanLoop);
    t_scan.detach();

    httplib::Server svr;

    svr.Get("/", [](const httplib::Request &, httplib::Response &res) {
        string html = exec("cat index.html");
        res.set_content(html, "text/html");
    });

    svr.Get("/api/radar", [](const httplib::Request &, httplib::Response &res) {
        lock_guard<mutex> lock(data_mutex);
        json response;
        
        json dev_arr = json::array();
        for (const auto& d : devices) {
            dev_arr.push_back({{"ip", d.ip}, {"mac", d.mac}, {"vendor", d.vendor}});
        }
        
        response["devices"] = dev_arr;
        response["total_devices"] = devices.size();
        response["is_scanning"] = is_scanning;
        response["last_scan"] = last_scan_time;
        
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(response.dump(), "application/json");
    });

    svr.Post("/api/force-scan", [](const httplib::Request &, httplib::Response &res) {
        thread t(runScan);
        t.detach();
        res.set_content("{\"status\":\"scanning\"}", "application/json");
    });

    cout << "Network Radar listening on http://0.0.0.0:8080" << endl;
    svr.listen("0.0.0.0", 8080);
    return 0;
}
