#include "GrammarAlgorithms.h"

#include <algorithm>

namespace details {

    bool isChomskyRuleRightSide(const RuleRightSide& rrs) {
        if (rrs.nt_indexes.size() > 2) return false;

        if (rrs.nt_indexes.size() == 1) return false;

        if (rrs.nt_indexes.size() == 2 &&
            rrs.sequence.size() != 2) return false;

        return true;
    }
   
    bool isInChomskyForm(const Grammar& g) {
        if (g.multirules.empty()) return false;

        for (auto& rrs : g.multirules.at(g.start)) {
            if (!isChomskyRuleRightSide(rrs) ||
                (rrs.nt_indexes.size() == 2 &&
                 std::find(rrs.sequence.begin(),
                           rrs.sequence.end(),
                           g.start) != rrs.sequence.end())) return false;
        }

        for (auto& [nt_code, rrs_vec] : g.multirules) {
            if (nt_code == g.start) continue;

            for (auto& rrs : rrs_vec) {
                if (!isChomskyRuleRightSide(rrs)) return false;
            }
        }

        return true;
    }

}  // namespace details
