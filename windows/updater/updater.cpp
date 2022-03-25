// updater.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "DatabaseMgr.h"

namespace khiin::data {
namespace fs = std::filesystem;

int ApplyUpdateToExisting(std::string const &existing_file, std::string const &update_file) {
    auto ret = -1;
    auto old_db = fs::path(existing_file);
    auto new_db = fs::path(update_file);

    if (fs::exists(old_db) && fs::exists(new_db)) {
        std::cout << "Updating database...";
        ret = DatabaseMgr::Update(old_db.string(), new_db.string());
    } else if (fs::exists(new_db)) {
        fs::copy(new_db, old_db);
        ret = 0;
    }

    return ret;
}

} // namespace khiin::data

class ArgParse {
  public:
    ArgParse(int &argc, char **argv) {
        for (auto i = 1; i < argc; ++i) {
            m_args.push_back(std::string(argv[i]));
        }
    }

    template <typename... Args>
    bool HasOptions(Args... rest) {
        return (true && ... && HasOption(rest));
    }

    bool HasOption(std::string const &option) const {
        return FindOption(option) != m_args.cend();
    }

    std::string GetOption(std::string const &option) const {
        auto it = FindOption(option);
        if (it != m_args.cend() && ++it != m_args.cend()) {
            return *it;
        }
        return std::string();
    }

  private:
    std::vector<std::string>::const_iterator FindOption(std::string const &option) const {
        return std::find(m_args.cbegin(), m_args.cend(), option);
    }

    std::vector<std::string> m_args;
};

int main(int argc, char **argv) {
    auto parser = ArgParse(argc, argv);
    if (parser.HasOptions("-old", "-new")) {
        return khiin::data::ApplyUpdateToExisting(parser.GetOption("-old"), parser.GetOption("-new"));
    }
    std::cout << "Please pass correct arguments to update database." << std::endl;
    return 0;
}
