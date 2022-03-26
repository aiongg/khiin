#pragma once

#include "pch.h"

#include "tip/Utils.h"

#include <string>
#include <vector>

namespace khiin::win32::settings {

class ArgParse {
  public:
    inline ArgParse(int &argc, wchar_t **argv) {
        for (auto i = 1; i < argc; ++i) {
            m_args.push_back(Utils::Narrow(argv[i]));
        }
    }

    template <typename... Args>
    inline bool HasOptions(Args... args) {
        return (true && ... && HasOption(args));
    }

    inline bool HasOption(std::string const &option) const {
        return FindOption(option) != m_args.cend();
    }

    inline std::string GetOption(std::string const &option) const {
        auto it = FindOption(option);
        if (it != m_args.cend() && ++it != m_args.cend()) {
            return *it;
        }
        return std::string();
    }

  private:
    inline std::vector<std::string>::const_iterator FindOption(std::string const &option) const {
        return std::find(m_args.cbegin(), m_args.cend(), option);
    }

    std::vector<std::string> m_args;
};

}; // namespace khiin::win32::settings
