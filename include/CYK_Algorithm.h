#pragma once

#include "Grammar.h"

#include <string_view>

namespace fl::algo::cyk {
    bool isRecognized(const std::string& text, const Grammar& g);
}  // namespace fl::cyk
