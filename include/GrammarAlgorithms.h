#pragma once

#include "Grammar.h"
#include "CYK_Algorithm.h"

namespace fl::algo {
    void convertToChomskyForm(Grammar& g, int end_phase);
    bool isInChomskyForm(const Grammar& g);
}  // namespace fl::algo
