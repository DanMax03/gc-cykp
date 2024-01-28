#include "GrammarAlgorithms.h"

#include <algorithm>

namespace {
    bool isChomskyRuleRightSide(const fl::RuleRightSide& rrs) {
        return rrs.nt_indexes.empty() || (rrs.nt_indexes.size() == 2 && rrs.sequence.size() == 2);
    }
}  // namespace

namespace fl::algo {
    bool isInChomskyForm(const Grammar& g) {
        if (g.multirules.empty()) {
            return false;
        }

        for (const auto& rrs : g.multirules.at(g.start)) {
            if (!isChomskyRuleRightSide(rrs) ||
                (rrs.nt_indexes.size() == 2 &&
                 std::find(rrs.sequence.begin(), rrs.sequence.end(), g.start) != rrs.sequence.end())) {
                return false;
            }
        }

        for (const auto& [nt_key, multirrs] : g.multirules) {
            if (nt_key == g.start) {
                continue;
            }

            for (const auto& rrs : multirrs) {
                if (!isChomskyRuleRightSide(rrs)) {
                    return false;
                }
            }
        }

        return true;
    }
}  // namespace fl::algo
