#include "Config.h"

#include "proto/proto.h"

namespace khiin::engine {
using namespace khiin::proto;

std::unique_ptr<AppConfig> DefaultAppConfig() {
    auto ret = proto::AppConfig().default_instance().New();
    ret->set_dotted_khin(true);
    ret->set_input_mode(IM_CONTINUOUS);
    auto keyconf = ret->mutable_key_config();
    keyconf->set_nasal("nn");
    keyconf->set_dot_above_right("ou");
    keyconf->set_dots_below("r");
    return std::unique_ptr<AppConfig>(ret);
}

} // namespace khiin::engine
