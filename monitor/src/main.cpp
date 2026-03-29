#include "Monitor.hpp"
#include <csignal>
#include <iostream>
#include <string>

static Monitor* g_monitor = nullptr;

static void signal_handler(int /*sig*/) {
    if (g_monitor) g_monitor->stop();
}

int main(int argc, char* argv[]) {
    std::string config_path = "config/devices.json";
    std::string log_path    = "fault_monitor.log";

    for (int i = 1; i < argc - 1; ++i) {
        std::string arg = argv[i];
        if (arg == "--config") config_path = argv[++i];
        else if (arg == "--log") log_path   = argv[++i];
    }

    std::signal(SIGINT,  signal_handler);
    std::signal(SIGTERM, signal_handler);

    try {
        Monitor monitor(config_path, log_path);
        g_monitor = &monitor;
        monitor.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
