#include "GrammarAlgorithms.h"

#include "Grammar.h"
#include "NonterminalCompression.h"

#include <algorithm>
#include <stack>
#include <map>
#include <unordered_set>

namespace {
    using namespace fl;

    /**
     * @param g - context-free grammar without any restrictions
     *
     * Removes all the nonterminals from the grammar that are
     * ungenerative (e.g. if an output passes a state containing such a nonterminal,
     * then it's impossible to get into another state with terminals only) and
     * unreachable (there's no output of the grammar that passes a state with a such nonterminal).
     */
    void deleteUngenerativeAndUnreachableNonterminals(Grammar& g) {
        struct DFSState {
            size_t left_nt_key;
            size_t rrs_index = 0;
            size_t right_nt_index;
        };

        using State = unsigned short;
        static const State kNothing = 0b000;
        static const State kWaited = 0b001;
        static const State kSeen = 0b010;
        static const State kGenerative = 0b100;

        std::vector<State> nt_states(g.tntable.table.size()); // TODO: remove kWaited, only kSeen and kGenerative
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
                    State indicator = kGenerative;

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

    /**
     *
     * @param nt_key - a key of a nonterminal on the left side of the rule
     * @param rrs_ind - an index of a RuleRightSide for the nonterminal
     * @param g - context-free grammar without any restrictions
     *
     * Example of what the function does:\n
     * A -> a B c D e f\n
     * The function firstly turns it in the next set of rules:\n
     * A -> U1 B U2 D U3 U4\n
     * U1 -> a\n
     * U2 -> c\n
     * U3 -> e\n
     * U4 -> f\n
     * Finally, the function additionaly transforms the first rule into the other set:\n
     * A -> U1 T1\n
     * T1 -> B T2\n
     * T2 -> U2 T3\n
     * T3 -> D T4\n
     * T4 -> U3 U4\n
     */
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

    /**
     * Applies unmixAndShortenRule for every rule in the grammar
     *
     * @param g - context-free grammar without any restrictions
     */
    void deleteMixedAndLongRules(Grammar& g) {
        for (auto& [nt_key, multirrs] : g.multirules) {
            for (ssize_t rrs_ind = 0; rrs_ind < multirrs.size(); ++rrs_ind) {
                unmixAndShortenRule(nt_key, rrs_ind, g);
            }
        }
    }

    /**
     *
     * @param g - context-free grammar which rules follow only the next patterns:\n
     * A -> a_1 ... a_n, where a_i is a non-empty terminal\n
     * A -> BC\n
     * A -> ""\n
     * A -> B\n
     *
     * The function adds a unique start S for the grammar, removes the pattern A -> ""
     * and adds S -> "" only and only if the "" string is generated by the grammar
     */
    void congregateEmptyGeneratingNonterminals(Grammar& g) {
        using State = unsigned short;

        static const State kNothing = 0b0000;
        static const State kWaited = 0b0001;
        static const State kSeen = 0b0010;
        static const State kEmptyGenerating = 0b0100;
        static const State kHasEmptyRule = 0b1000;

        static const auto addUniqueStart = [](Grammar& g) {
            TokenKey unique_start = insertUniqueNonterminal(g);
            g.multirules[unique_start].push_back(RuleRightSide{{g.start}, {0}});
            g.start = unique_start;
        };

        auto empty_it = g.tntable.rtable.find("");

        if (empty_it == g.tntable.rtable.end()) {
            addUniqueStart(g);
            return;
        }

        // Need to:
        // 1) Check if "" is generated
        // 2) Find all the rules A -> ""
        // 3) Delete all the rules A -> ""
        // 4) Add a unique start S and S -> "" rule if needed

        const TokenKey empty_key = empty_it->second;
        const auto isEmptyRule = [&, empty_key](const RuleRightSide& rrs) {
            return std::all_of(rrs.sequence.begin(), rrs.sequence.end(), [&, empty_key](TokenKey t_key) {
                return t_key == empty_key;
            });
        };
        auto& table = g.tntable.table;
        std::vector<State> nt_states(table.size());
        const auto isEmptyGeneratingRule = [&](const RuleRightSide& rrs) {
            return std::all_of(rrs.sequence.begin(), rrs.sequence.end(), [&](TokenKey nt_key) {
                return nt_states[nt_key] & kEmptyGenerating;
            });
        };
        std::stack<TokenKey> dfs_stack;
        TokenKey cur;

        nt_states[g.start] = kWaited;
        dfs_stack.push(g.start);

        while (!dfs_stack.empty()) {
            cur = dfs_stack.top();

            if (nt_states[cur] & kSeen) {
                dfs_stack.pop();

                for (auto& rrs : g.multirules[cur]) {
                    if (rrs.nt_indexes.empty()) {
                        if (isEmptyRule(rrs)) {
                            nt_states[cur] |= kEmptyGenerating | kHasEmptyRule;
                        }
                        break;
                    }

                    if (isEmptyGeneratingRule(rrs)) {
                        nt_states[cur] |= kEmptyGenerating;
                        break;
                    }
                }
            } else {
                nt_states[cur] = kSeen;

                for (auto& rrs : g.multirules[cur]) {
                    if (rrs.nt_indexes.empty()) {
                        continue;
                    }

                    for (TokenKey nt_key : rrs.sequence) {
                        if (nt_states[nt_key] & (kWaited | kSeen)) {
                            continue;
                        }

                        nt_states[nt_key] = kWaited;
                        dfs_stack.push(nt_key);
                    }
                }
            }
        }

        MultiruleRightSide::iterator rm_it;

        for (auto it = g.multirules.begin(); it != g.multirules.end(); ++it) {
            if ((nt_states[it->first] & kHasEmptyRule) == 0) {
                continue;
            }

            rm_it = std::remove_if(it->second.begin(),
                                   it->second.end(),
                                   [&](const RuleRightSide& rule) {
                                       return isEmptyRule(rule);
                                   });
            it->second.erase(rm_it, it->second.end());

            if (it->second.empty()) {
                g.tntable.erase(it->first, TokenType::kNonterminal);
                g.multirules.erase(it);
            }
        }

        auto old_start = g.start;
        addUniqueStart(g);

        if (nt_states[old_start] & kEmptyGenerating) {
            g.multirules[g.start].emplace_back(RuleRightSide{{empty_key}, {}});
        }
    }

    /**
     *
     * @param g - context-free grammar which follows the restrictions:\n
     * 1) The grammar's start S is unique, e.g. it doesn't appear on the right side of any rule\n
     * 2) All the rules have one of the next patterns:\n
     * S -> ""\n
     * A -> a_1 ... a_n, where a_i is a non-empty terminal\n
     * A -> BC\n
     * A -> B\n
     *
     * The function removes the rules that match the last pattern
     */
    void deleteNonterminalChains(Grammar& g) {
        using ParentsSet = std::unordered_set<TokenKey>;

        static const auto isChainRule = [](const RuleRightSide& rrs) {
            return rrs.nt_indexes.size() == 1;
        };

        // Phase 1: divide the old multirules to the chain multirules and the remaining
        MultirulesMap chain_multirules;

        for (auto& multirule : g.multirules) {
            auto nt_key = multirule.first;
            auto& multirrs = multirule.second;
            auto rm_it = std::remove_if(multirrs.begin(), multirrs.end(), [&](auto& rrs) {
                bool result = isChainRule(rrs);

                if (result) {
                    chain_multirules[nt_key].push_back(std::move(rrs));
                }

                return result;
            });

            multirrs.erase(rm_it, multirrs.end());
        }

        // Phase 2: find all the parents for each nonterminal
        algo::NonterminalTokenKeyTable nt_table;
        algo::initNonterminalTokenKeyTable(nt_table, g);

        std::vector<ParentsSet> parents(nt_table.size());
        std::vector<unsigned> chain_color(nt_table.size());
        unsigned available_color = 1;

        // todo: potentially can be done in "one" efficient DFS with colour backtracing
        for (auto& [nt_key, multirrs] : chain_multirules) {
            auto nt_code = nt_table[nt_key];
            std::stack<TokenKey> dfs_stack;
            TokenKey cur;

            chain_color[nt_code] = available_color;
            dfs_stack.push(nt_key);

            while (!dfs_stack.empty()) {
                cur = dfs_stack.top();
                dfs_stack.pop();

                for (auto& rrs : chain_multirules[cur]) {
                    auto next_nt_key = rrs.sequence[0];
                    auto next_nt_code = nt_table[next_nt_key];
                    auto& next_nt_color = chain_color[next_nt_code];

                    if (next_nt_color == available_color) {
                        continue;
                    }

                    next_nt_color = available_color;
                    parents[next_nt_code].insert(nt_key);
                    dfs_stack.push(next_nt_key);
                }
            }

            ++available_color;
        }

        // Phase 3: bring new rules to the remaining
        for (auto& [nt_key, multirrs] : g.multirules) {
            auto nt_code = nt_table[nt_key];

            if (parents[nt_code].empty()) {
                continue;
            }

            for (auto& rrs : multirrs) {
                if (isChainRule(rrs)) {
                    continue;
                }

                for (auto parent : parents[nt_code]) {
                    g.multirules[parent].push_back(rrs);
                }
            }
        }
    }
}  // namespace

namespace fl::algo {
    void convertToChomskyForm(Grammar& g, int end_phase) {
        // todo: use end_phase
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
