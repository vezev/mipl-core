#pragma once

#include <string>

namespace mipl::domain {

struct ModuleId {
    std::string value;

    bool operator==(const ModuleId&) const = default;
};

struct ModuleDescriptor {
    ModuleId id;
    std::string display_name;
    std::string command_prefix;
    std::string description;
    bool playable{false};
};

} // namespace mipl::domain
