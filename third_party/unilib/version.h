// This file is part of UniLib <http://github.com/ufal/unilib/>.
//
// Copyright 2014 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// UniLib version: 3.2.0
// Unicode version: 13.0.0

#pragma once

#include <string>

namespace ufal {
namespace unilib {

struct version {
  unsigned major;
  unsigned minor;
  unsigned patch;
  std::string prerelease;

  // Returns current version.
  static version current();
};

// Returns current version.
version version::current() {
  return {3, 2, 0, ""};
}

} // namespace unilib
} // namespace ufal
