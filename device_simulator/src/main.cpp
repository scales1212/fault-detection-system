#include "DeviceSimulator.hpp"
#include "GarageDoor.hpp"
#include "TemperatureControlUnit.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

static void print_usage(const char* prog) {
    std::cerr << "Usage: " << prog
              << " --type <TemperatureControlUnit|GarageDoor>"
              << " --uid <uid>"
              << " --port <port>\n";
}

int main(int argc, char* argv[]) {
    std::string type_str, uid_str;
    uint16_t    port = 0;

    for (int i = 1; i < argc - 1; ++i) {
        std::string arg = argv[i];
        if (arg == "--type") type_str = argv[++i];
        else if (arg == "--uid")  uid_str  = argv[++i];
        else if (arg == "--port") port     = static_cast<uint16_t>(std::stoi(argv[++i]));
    }

    if (type_str.empty() || uid_str.empty() || port == 0) {
        print_usage(argv[0]);
        return 1;
    }

    std::unique_ptr<IDevice> device;
    if (type_str == "TemperatureControlUnit")
        device = std::make_unique<TemperatureControlUnit>(uid_str);
    else if (type_str == "GarageDoor")
        device = std::make_unique<GarageDoor>(uid_str);
    else {
        std::cerr << "Unknown device type: " << type_str << "\n";
        print_usage(argv[0]);
        return 1;
    }

    try {
        DeviceSimulator sim(std::move(device), port);
        sim.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
