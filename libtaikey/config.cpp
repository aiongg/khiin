#include <boost/property_tree/json_parser.hpp>

#include "config.h"

namespace TaiKey {

Config::Config() {}

Config::Config(std::string configFile) : configFile_(configFile) {
    setConfigFile(configFile_);
}

auto Config::setConfigFile(std::string configFile) -> void {
    configFile_ = configFile;
    boost::property_tree::read_json(configFile_, json_);
}

auto Config::getSettings() -> const Settings& {
    settings_.capitalizeEnglish = json_.get<bool>("capitalizeEnglish");
    settings_.toneMode = json_.get<std::string>("toneMode");
    // TODO
    return const_cast<Settings&>(settings_);
}

} // namespace TaiKey