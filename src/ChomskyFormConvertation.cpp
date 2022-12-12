#include "GrammarAlgorithms.h"

#include "Grammar.h"

#include <stack>
#include <map>
#include <string_view>

namespace details {

    namespace extra {
        void deleteUngenerativeAndUnreachableNonterminals(Grammar& g) {
            using ushort = unsigned short;

            static const ushort k_Nothing = 0;
            static const ushort k_Waited = 0b1;
            static const ushort k_Seen = 0b10;
            static const ushort k_Generative = 0b100;
            
            std::vector<unsigned short> nt_states(g.tdtable.table.size());
            std::stack<size_t> dfs_stack;
            size_t cur;
            ushort indicator;
    
            dfs_stack.push(g.start);
    
            while (!dfs_stack.empty()) {
                cur = dfs_stack.top();
    
                auto& rrs_vec = g.multirules[cur];
    
                if ((nt_states[cur] & k_Seen) == 0) {
                    nt_states[cur] = k_Seen;
    
                    for (auto& rrs : rrs_vec) {
                        for (auto& nt_index : rrs.nt_indexes) {
                            if (nt_states[rrs.sequence[nt_index]] & k_Waited) continue;
                            if (nt_states[rrs.sequence[nt_index]] & k_Seen) continue;
    
                            nt_states[rrs.sequence[nt_index]] = k_Waited;
                            dfs_stack.push(rrs.sequence[nt_index]);
                        }
                    }
                } else {
                    dfs_stack.pop();
    
                    for (auto& rrs : rrs_vec) {
                        indicator = k_Generative;
    
                        for (auto& nt_index : rrs.nt_indexes) {
                            indicator &= nt_states[rrs.sequence[nt_index]];
                        }
    
                        if (indicator != 0) {
                            nt_states[cur] |= k_Generative;
                            break;
                        }
                    }
                }
            }
    
            for (auto it = g.multirules.begin(); it != g.multirules.end();) {
                if (nt_states[it->first] & k_Generative) {
                    ++it;
                    continue;
                }
    
                it = g.multirules.erase(it);
            }
        }

        size_t insertUniqueNonterminal(Grammar& g) {
            static std::string nt_prefix = "unique_nonterminal_";
            static size_t nt_number = 0;

            auto& table = g.tdtable.table;
            auto& rtable = g.tdtable.rtable;
            std::string s = nt_prefix + std::to_string(nt_number);

            while (rtable.find(s) != rtable.end()) {
                ++nt_number;
                s = nt_prefix + std::to_string(nt_number);
            }

            g.tdtable.insert(s, TableValue::k_Nonterminal);

            return nt_number;
        }

        void unmixAndShortenRule(RuleRightSide& rrs, Grammar& g) {
            // idea:
            // while getting unique nonterminals for terminals,
            // fill an array with all indexes
            // after that, we should iteratively add new rules with more unique nonterminals

            size_t unique_nt_index;
            size_t prefix_size = rrs.nt_indexes.empty() ? rrs.sequence.size() : rrs.nt_indexes[0];
            auto& rtable = g.tdtable.rtable;

            // TODO: don't miss unique_nt_indexes

            for (size_t j = 0; j < prefix_size; ++j) {
                unique_nt_index = insertUniqueNonterminal(g);
                g.multirules.insert({unique_nt_index, {RuleRightSide({rrs.sequence[j]}, {})}});
            }

            if (rrs.nt_indexes.empty()) return;

            for (size_t i = 1; i < rrs.nt_indexes.size(); ++i) {
                for (size_t j = rrs.nt_indexes[i - 1] + 1; j < rrs.nt_indexes[i]; ++j) {
                    unique_nt_index = insertUniqueNonterminal(g);
                    g.multirules.insert({unique_nt_index, {RuleRightSide({rrs.sequence[j]}, {})}});
                }
            }

            for (size_t j = rrs.nt_indexes.back() + 1; j < rrs.sequence.size(); ++j) {
                unique_nt_index = insertUniqueNonterminal(g);
                g.multirules.insert({unique_nt_index, {RuleRightSide({rrs.sequence[j]}, {})}});
            }

            if (g.)
        }

        void deleteMixedAndLongRules(Grammar& g) {
            for (auto& [nt_index, rrs_vec] : g.multirules) {
                for (auto& rrs : rrs_vec) {
                    if (rrs.sequence.size() == 1 || rrs.nt_indexes.empty()) continue;

                    unmixAndShortenRule(rrs, g);
                }
            }
        }
    
        void deleteEmptyGeneratingNonterminals(Grammar& g) {
            
        }
    
        void deleteRulesWithEmpty(Grammar& g) {
            
        }
    
        void deleteAloneRules(Grammar& g) {
            
        }

    }  // namespace details::extra

    void convertToChomskyForm(Grammar& g) {
        using namespace details::extra;

        if (g.multirules.empty()) return;

        // deleteUngenerativeAndUnreachableNonterminals(g);
        // deleteMixedAndLongRules(g);
        // deleteEmptyGenerating(g);
    }

}  // namespace details
