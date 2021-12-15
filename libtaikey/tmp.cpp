#include <iostream>
#include <memory>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/filesystem/string_file.hpp>

#include "tmp.h"

namespace fs = boost::filesystem;

namespace TaiKey {

std::unique_ptr<TNode> tmpGetSylTrieFromFile() {
    std::unique_ptr<TNode> root = std::make_unique<TNode>();

    if (!fs::exists("syllables.dat")) {
        return root;
    }

    fs::ifstream fileHandler("syllables.dat");
    std::string syl;

    while (std::getline(fileHandler, syl)) {
        root->insert(syl);
    }

    return root;
}

}
