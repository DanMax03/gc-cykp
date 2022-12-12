#ifndef _INCLUDE_GRAMMARALGORITHMS_H
#define _INCLUDE_GRAMMARALGORITHMS_H

#include "Grammar.h"
#include "CYK_Algorithm.h"

namespace details {

    bool isChomskyRuleRightSide(const RuleRightSide& rrs);

    void convertToChomskyForm(Grammar& g);

    bool isInChomskyForm(const Grammar& g);

}  // namespace details

#endif  // _INCLUDE_GRAMMARALGORITHMS_H
