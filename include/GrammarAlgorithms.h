#pragma once

#include "Grammar.h"
#include "CYK_Algorithm.h"

namespace fl {
    bool isChomskyRuleRightSide(const RuleRightSide& rrs);

    void convertToChomskyForm(Grammar& g);

    bool isInChomskyForm(const Grammar& g);
}  // namespace fl
