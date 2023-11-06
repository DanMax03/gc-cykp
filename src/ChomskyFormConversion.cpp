#include "GrammarAlgorithms.h"

#include "Grammar.h"

#include <algorithm>
#include <stack>
#include <map>

namespace {
    using namespace fl;
    
    void deleteUngenerativeAndUnreachableNonterminals(Grammar& g) {
        struct DFSState {
            size_t left_nt_key;
            size_t rrs_index = 0;
            size_t right_nt_index;
        };

        static const unsigned short kNothing = 0b000;
        static const unsigned short kWaited = 0b001;
        static const unsigned short kSeen = 0b010;
        static const unsigned short kGenerative = 0b100;

        std::vector<unsigned short> nt_states(g.tntable.table.size()); // TODO: remove kWaited, only kSeen and kGenerative
        std::stack<TokenKey> dfs_stack;
        TokenKey cur;

        nt_states[g.start] = kWaited;
        dfs_stack.push(g.start);

        while (!dfs_stack.empty()) {
            cur = dfs_stack.top();

            auto& multirrs = g.multirules[cur];

            if ((nt_states[cur] & kSeen) == 0) {
                nt_states[cur] = kSeen;

                for (auto& rrs : multirrs) {
                    for (auto& nt_index : rrs.nt_indexes) {
                        auto nt_token = rrs.sequence[nt_index];
                        auto& nt_state = nt_states[nt_token];

                        if (nt_state & kWaited) continue;
                        if (nt_state & kSeen) continue;

                        nt_state = kWaited;
                        dfs_stack.push(nt_token);
                    }
                }
            } else {
                dfs_stack.pop();

                for (auto& rrs : multirrs) {
                    int indicator = kGenerative;

                    for (auto& nt_index : rrs.nt_indexes) {
                        indicator &= nt_states[rrs.sequence[nt_index]];
                    }

                    if (indicator != 0) {
                        nt_states[cur] |= kGenerative;
                        break;
                    }
                }
            }
        }

        // todo: handle deleting ungenerative or unreachable nonterminals on condition
        for (auto it = g.multirules.begin(); it != g.multirules.end();) {
            if (nt_states[it->first] & kGenerative) {
                ++it;
                continue;
            }

            g.tntable.erase(it->first, TokenType::kNonterminal);
            it = g.multirules.erase(it);  // todo: handle terminals in g.tntable
        }
    }

    TokenKey insertUniqueNonterminal(Grammar& g) {
        static std::string nt_prefix = "unique_nonterminal_";
        static size_t nt_number = 0;

        auto& table = g.tntable.table;
        auto& rtable = g.tntable.rtable;
        std::string s = nt_prefix + std::to_string(nt_number);
        auto it = rtable.find(s);

        while (it != rtable.end() && (table[it->second].type & TokenType::kNonterminal) != TokenType::kNothing) {
            s = nt_prefix + std::to_string(nt_number++);
            it = rtable.find(s);
        }

        return g.tntable.insert(std::move(s), TokenType::kNonterminal);
    }

    void unmixAndShortenRule(TokenKey nt_key, size_t rrs_ind, Grammar& g) {
        auto& rrs = g.multirules[nt_key][rrs_ind];
        
        if (rrs.sequence.size() == 1 || rrs.nt_indexes.empty() ||
            (rrs.sequence.size() == 2 && rrs.nt_indexes.size() == 2)) {
            return;
        }

        TokenKey unique_nt_key;
        auto& rtable = g.tntable.rtable;
        std::vector<TokenKey> new_nt_keys(rrs.sequence.size());
        auto replaceTerminal = [&](ssize_t j) {
            unique_nt_key = insertUniqueNonterminal(g);
            g.multirules[unique_nt_key] = {RuleRightSide{{rrs.sequence[j]}, {}}};
            new_nt_keys[j] = unique_nt_key;
        };

        for (ssize_t j = 0; j < rrs.nt_indexes[0]; ++j) {
            replaceTerminal(j);
        }

        new_nt_keys[rrs.nt_indexes[0]] = rrs.sequence[rrs.nt_indexes[0]];

        for (ssize_t i = 1; i < rrs.nt_indexes.size(); ++i) {
            for (ssize_t j = static_cast<ssize_t>(rrs.nt_indexes[i - 1]) + 1; j < rrs.nt_indexes[i]; ++j) {
                replaceTerminal(j);
            }
            new_nt_keys[rrs.nt_indexes[i]] = rrs.sequence[rrs.nt_indexes[i]];
        }

        for (ssize_t j = static_cast<ssize_t>(rrs.nt_indexes.back()) + 1; j < rrs.sequence.size(); ++j) {
            replaceTerminal(j);
        }

        if (rrs.sequence.size() == 2) {
            g.multirules[nt_key][rrs_ind] = RuleRightSide{{new_nt_keys[0], new_nt_keys[1]}, {0, 1}};
            return;
        }

        TokenKey cur_nt = insertUniqueNonterminal(g);
        TokenKey prev_nt = cur_nt;
        size_t i = new_nt_keys.size() - 2;

        g.multirules[cur_nt].push_back(RuleRightSide{{new_nt_keys[i], new_nt_keys[i + 1]}, {0, 1}});

        while (i > 1) {
            --i;

            cur_nt = insertUniqueNonterminal(g);
            g.multirules[cur_nt].push_back(RuleRightSide{{new_nt_keys[i], prev_nt}, {0, 1}});
            prev_nt = cur_nt;
        }

        g.multirules[nt_key][rrs_ind] = RuleRightSide{{new_nt_keys[0], prev_nt}, {0, 1}};
    }

    void deleteMixedAndLongRules(Grammar& g) {
        for (auto& [nt_key, multirrs] : g.multirules) {
            for (ssize_t rrs_ind = 0; rrs_ind < multirrs.size(); ++rrs_ind) {
                unmixAndShortenRule(nt_key, rrs_ind, g);
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
                (g.tntable.table[empty_it->second].type & TokenType::kNonterminal) == 0) {
                addUniqueStart(g);
                return;
            }
        }

        static const int kNothing = 0b0;
        static const int kSeen = 0b1;
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

            if (nt_flags[cur] & kSeen) {
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
                        indicator = kNothing;
                    }

                    if (nt_flags[rrs.sequence.back()] & k_EmptyGenerating) {
                        g.multirules[cur].push_back(RuleRightSide{{rrs.sequence.front()}, {0}});
                    } else {
                        indicator = kNothing;
                    }

                    nt_flags[cur] |= indicator;
                }
            } else {
                nt_flags[cur] = kSeen;

                for (auto& rrs : g.multirules[cur]) {
                    for (size_t nt_index : rrs.nt_indexes) {
                        if (nt_flags[nt_index] & kSeen) continue;

                        nt_flags[nt_index] = kSeen;
                    }
                }
            }
        }

        std::vector<fl::RuleRightSide>::iterator rm_it;

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
                g.tntable.erase(it->first, TokenType::kNonterminal);
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

        for (auto& [nt_key, multirrs] : g.multirules) {
            multirrs.erase(std::remove_if(multirrs.begin(),
                                         multirrs.end(),
                                         [](const RuleRightSide& rrs) { return rrs.nt_indexes.size() == 1; }),
                          multirrs.end());
        }

        for (auto& [nt_key, multirrs] : compressed_multirules) {
            g.multirules[nt_key].append???
            g.multirules
        }
    }
}  // namespace

namespace fl::algo {
    void convertToChomskyForm(Grammar& g, int end_phase) {
        std::ignore = end_phase;

        if (g.multirules.empty()) {
            return;
        }

        deleteUngenerativeAndUnreachableNonterminals(g);
        deleteMixedAndLongRules(g);
        congregateEmptyGeneratingNonterminals(g);
        deleteNonterminalChains(g);
        deleteUngenerativeAndUnreachableNonterminals(g);
    }

}  // namespace fl::algo
