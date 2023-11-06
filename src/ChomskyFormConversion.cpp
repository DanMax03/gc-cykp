#include "GrammarAlgorithms.h"

#include "Grammar.h"

#include <algorithm>
#include <stack>
#include <map>

namespace details {

    namespace extra {
        void deleteUngenerativeAndUnreachableNonterminals(Grammar& g) {
            struct DFSState {
                size_t left_nt_key;
                size_t rrs_index = 0;
                size_t right_nt_index;
            };

            static const int k_Nothing = 0;
            static const int k_Waited = 0b1;
            static const int k_Seen = 0b10;
            static const int k_Generative = 0b100;
            
            std::vector<unsigned short> nt_states(g.tntable.table.size()); // TODO: remove k_Waited, only k_Seen and k_Generative
            std::stack<size_t> dfs_stack;
            size_t cur;
            int indicator;
    
            nt_states[g.start] = k_Waited;
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
    
                g.tntable.erase(it->first, TableValue::k_Nonterminal);
                it = g.multirules.erase(it);
            }
        }

        size_t insertUniqueNonterminal(Grammar& g) {
            static std::string nt_prefix = "unique_nonterminal_";
            static size_t nt_number = 0;

            auto& table = g.tntable.table;
            auto& rtable = g.tntable.rtable;
            std::string s = nt_prefix + std::to_string(nt_number);
            auto it = rtable.find(s);

            while (it != rtable.end() &&
                   (table[it->second].type & TableValue::k_Nonterminal) != 0) {
                ++nt_number;
                s = nt_prefix + std::to_string(nt_number);
                it = rtable.find(s);
            }

            return g.tntable.insert(std::move(s), TableValue::k_Nonterminal);
        }

        void unmixAndShortenRule(size_t rls, size_t rrs_ind, Grammar& g) {
            auto& rrs = g.multirules[rls][rrs_ind];

            if (rrs.sequence.size() == 1 || rrs.nt_indexes.empty() ||
                (rrs.sequence.size() == 2 && rrs.nt_indexes.size() == 2)) return;

            size_t unique_nt_code;
            auto& rtable = g.tntable.rtable;
            std::vector<size_t> new_nt_codes(rrs.sequence.size());

            for (size_t j = 0; j < rrs.nt_indexes[0]; ++j) {
                unique_nt_code = insertUniqueNonterminal(g);
                g.multirules[unique_nt_code] = {RuleRightSide{{rrs.sequence[j]}, {}}};
                new_nt_codes[j] = unique_nt_code;
            }

            new_nt_codes[rrs.nt_indexes[0]] = rrs.sequence[rrs.nt_indexes[0]];

            for (size_t i = 1; i < rrs.nt_indexes.size(); ++i) {
                for (size_t j = rrs.nt_indexes[i - 1] + 1; j < rrs.nt_indexes[i]; ++j) {
                    unique_nt_code = insertUniqueNonterminal(g);
                    g.multirules[unique_nt_code] = {RuleRightSide{{rrs.sequence[j]}, {}}};
                    new_nt_codes[j] = unique_nt_code;
                }
                new_nt_codes[rrs.nt_indexes[i]] = rrs.sequence[rrs.nt_indexes[i]];
            }

            for (size_t j = rrs.nt_indexes.back() + 1; j < rrs.sequence.size(); ++j) {
                unique_nt_code = insertUniqueNonterminal(g);
                g.multirules[unique_nt_code] = {RuleRightSide{{rrs.sequence[j]}, {}}};
                new_nt_codes[j] = unique_nt_code;
            }

            if (rrs.sequence.size() == 2) {
                g.multirules[rls][rrs_ind] = RuleRightSide{{new_nt_codes[0], new_nt_codes[1]}, {0, 1}};
                return;
            }

            size_t cur_nt = insertUniqueNonterminal(g);
            size_t i = new_nt_codes.size() - 2;
            size_t prev_nt = cur_nt;

            g.multirules[cur_nt].push_back(RuleRightSide{{new_nt_codes[i], new_nt_codes[i + 1]}, {0, 1}});
            
            while (i > 1) {
                --i;

                cur_nt = insertUniqueNonterminal(g);
                g.multirules[cur_nt].push_back(RuleRightSide{{new_nt_codes[i], prev_nt}, {0, 1}});
                prev_nt = cur_nt;
            }

            g.multirules[rls][rrs_ind] = RuleRightSide{{new_nt_codes[0], prev_nt}, {0, 1}};
        }

        void deleteMixedAndLongRules(Grammar& g) {
            for (auto& [rls, rrs_vec] : g.multirules) {
                for (size_t rrs_ind = 0; rrs_ind < rrs_vec.size(); ++rrs_ind) {
                    unmixAndShortenRule(rls, rrs_ind, g);
                }
            }
        }

        void addUniqueStart(Grammar& g) {
            size_t unique_start = insertUniqueNonterminal(g);

            g.multirules[unique_start].push_back(RuleRightSide{{g.start}, {0}});
            g.start = unique_start;
        }
    
        void congregateEmptyGeneratingNonterminals(Grammar& g) {
            {
                auto empty_it = g.tntable.rtable.find("");

                if (empty_it == g.tntable.rtable.end() ||
                    (g.tntable.table[empty_it->second].type & TableValue::k_Nonterminal) == 0) {
                    addUniqueStart(g);
                    return;
                }
            }

            static const int k_Nothing = 0b0;
            static const int k_Seen = 0b1;
            static const int k_EmptyGenerating = 0b10;
            static const int k_HasEmptyRule = 0b100;

            const size_t empty_code = g.tntable.rtable[""];
            auto& table = g.tntable.table;

            std::vector<int> nt_flags(g.tntable.table.size());
            std::stack<size_t> dfs_stack;
            size_t cur;
            int indicator;

            dfs_stack.push(g.start);

            while (!dfs_stack.empty()) {
                cur = dfs_stack.top();

                if (nt_flags[cur] & k_Seen) {
                    dfs_stack.pop();

                    for (auto& rrs : g.multirules[cur]) {
                        if (isRuleRightSidesEqual(rrs,
                                                  RuleRightSide{{empty_code}, {}},
                                                  table,
                                                  table)) {
                            nt_flags[cur] |= k_EmptyGenerating | k_HasEmptyRule;
                            break;
                        }

                        if (rrs.nt_indexes.empty()) continue;

                        indicator = k_EmptyGenerating;

                        if (rrs.sequence.size() == 1) {
                            indicator &= nt_flags[rrs.sequence.front()];
                            nt_flags[cur] |= indicator;
                            continue;
                        }

                        if (nt_flags[rrs.sequence.front()] & k_EmptyGenerating) {
                            g.multirules[cur].push_back(RuleRightSide{{rrs.sequence.back()}, {0}});
                        } else {
                            indicator = k_Nothing;
                        }

                        if (nt_flags[rrs.sequence.back()] & k_EmptyGenerating) {
                            g.multirules[cur].push_back(RuleRightSide{{rrs.sequence.front()}, {0}});
                        } else {
                            indicator = k_Nothing;
                        }

                        nt_flags[cur] |= indicator;
                    }
                } else {
                    nt_flags[cur] = k_Seen;

                    for (auto& rrs : g.multirules[cur]) {
                        for (size_t nt_index : rrs.nt_indexes) {
                            if (nt_flags[nt_index] & k_Seen) continue;

                            nt_flags[nt_index] = k_Seen;
                        }
                    }
                }
            }

            std::vector<details::RuleRightSide>::iterator rm_it;

            for (auto it = g.multirules.begin(); it != g.multirules.end(); ++it) {
                if ((nt_flags[it->first] & k_HasEmptyRule) == 0) continue;

                rm_it = std::remove_if(it->second.begin(),
                                       it->second.end(),
                                       [empty_code, &table](const RuleRightSide& rule) {
                    return isRuleRightSidesEqual(rule,
                                                 RuleRightSide{{empty_code}, {}},
                                                 table,
                                                 table);
                });
                it->second.erase(rm_it, it->second.end());
                    
                if (it->second.empty()) {
                    g.tntable.erase(it->first, TableValue::k_Nonterminal);
                    g.multirules.erase(it);
                }
            }

            addUniqueStart(g);
            g.multirules[g.start].push_back(RuleRightSide{{empty_code}, {}});
        }
    
        void deleteNonterminalChains(Grammar& g) {
            struct DFSState {
                size_t left_nt_key;
                size_t rrs_index = 0;
                size_t right_nt_index = 0;
            };

            MultirulesMap compressed_multirules;

            std::vector<bool> is_seen(g.tntable.table.size());
            std::vector<DFSState> dfs_vector;
            DFSState cur;
            size_t dividing_nt_key;
            size_t another_nt_key;
            bool is_current_first;
            bool is_divided = false;

            dfs_vector.push_back({g.start, 0, 0});

            while (!dfs_vector.empty()) {
                cur = dfs_vector.top();

                auto& multirule = g.multirules[cur.left_nt_key];

                if (cur.rrs_index == multirule.size()) {
                    dfs_vector.pop_back();

                    if (!is_divided) continue;

                    for (auto& state : dfs_vector) {
                        compressed_multirules[state.left_nt_key].push_back(RuleRightSide{(is_current_first
                                                                                          ? {cur.left_nt_key, another_nt_key}
                                                                                          : {another_nt_key, cur.left_nt_key}),
                                                                                         {0, 1}});
                    }

                    if (dividing_nt_key == cur.left_nt_key) {
                        is_divided = false;
                    }

                    continue;
                }

                for (; cur.rrs_index < multirule.size(); ++cur.rrs_index) {
                    auto& rrs = multirule[cur.rrs_index];

                    if ((rrs.nt_indexes.size() == 2 && is_divided) || rrs.nt_indexes.empty()) continue;

                    for (; cur.right_nt_index < rrs.nt_indexes.size(); ++cur.right_nt_index) {
                        if (is_seen[rrs.sequence[rrs.nt_indexes[cur.right_nt_index]]]) continue;
                        break;
                    }

                    if (cur.right_nt_index == rrs.nt_indexes.size()) {
                        cur.right_nt_index = 0;
                        continue;
                    }

                    if (rrs.nt_indexes.size() == 2) {
                        is_divided = true;
                        is_current_first = cur.right_nt_index == 0;
                        dividing_nt_key = cur.left_nt_key;
                        another_nt_key = (is_current_first ? rrs.sequence[1] : rrs.sequence[0]);
                    }

                    dfs_vector.push_back({rrs.sequence[cur.right_nt_index], 0, 0});
                    ++cur.right_nt_index;
                    break;
                }
            }

            for (auto& [nt_key, rrs_vec] : g.multirules) {
                rrs_vec.erase(std::remove_if(rrs_vec.begin(),
                                             rrs_vec.end(),
                                             [](const RuleRightSide& rrs) { return rrs.nt_indexes.size() == 1; }),
                              rrs_vec.end());
            }

            for (auto& [nt_key, rrs_vec] : compressed_multirules) {
                g.multirules[nt_key].append???
                g.multirules
            }
        }

    }  // namespace details::extra

    void convertToChomskyForm(Grammar& g) {
        using namespace details::extra;

        if (g.multirules.empty()) return;

        deleteUngenerativeAndUnreachableNonterminals(g);
        deleteMixedAndLongRules(g);
        congregateEmptyGeneratingNonterminals(g);
        deleteNonterminalChains(g);
        deleteUngenerativeAndUnreachableNonterminals(g);
    }

}  // namespace details
