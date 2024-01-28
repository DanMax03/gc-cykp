#include "GrammarAlgorithms.h"

#include "Grammar.h"
#include "NonterminalCompression.h"

#include <algorithm>
#include <stack>
#include <map>
#include <set>
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
    void deleteUnreachableNonterminals(Grammar& g) {
        algo::NonterminalTokenKeyTable nt_table;
        algo::initNonterminalTokenKeyTable(nt_table, g);
        std::vector<bool> is_nt_seen(nt_table.size());

        std::stack<TokenKey> dfs_stack;
        TokenKey cur;

        is_nt_seen[nt_table[g.start]] = true;
        dfs_stack.push(g.start);

        while (!dfs_stack.empty()) {
            cur = dfs_stack.top();
            dfs_stack.pop();

            for (const auto& rrs : g.multirules[cur]) {
                for (const auto rrs_nt_index : rrs.nt_indexes) {
                    const auto rrs_nt_key = rrs.sequence[rrs_nt_index];

                    if (!is_nt_seen[nt_table[rrs_nt_key]]) {
                        is_nt_seen[nt_table[rrs_nt_key]] = true;
                        dfs_stack.push(rrs_nt_key);
                    }
                }
            }
        }

        for (auto it = g.multirules.begin(); it != g.multirules.end();) {
            if (is_nt_seen[nt_table[it->first]]) {
                ++it;
                continue;
            }

            it = g.multirules.erase(it);
        }
    }

    void deleteUngenerativeNonterminals(Grammar& g) {
        // Phase 1: searching generative nonterminals
        using State = unsigned;
        static const State kNothing = 0b00;
        static const State kGenerative = 0b01;

        // Phase 1.1: construct reversed graph on the nonterminals
        //   so that each nonterminal is connected with those which
        //   can produce an output containing the nonterminal
        std::map<TokenKey, std::set<TokenKey>> nt_generating_nts;

        for (const auto& [nt_key, multirrs] : g.multirules) {
            for (const auto& rrs : multirrs) {
                if (rrs.nt_indexes.empty()) {
                    continue;
                }

                for (const auto rrs_nt_index : rrs.nt_indexes) {
                    const auto rrs_nt_key = rrs.sequence[rrs_nt_index];

                    if (rrs_nt_key != nt_key) {
                        nt_generating_nts[rrs_nt_key].insert(nt_key);
                    }
                }
            }
        }

        // Phase 1.2: find all generative nonterminals
        algo::NonterminalTokenKeyTable nt_table;
        algo::initNonterminalTokenKeyTable(nt_table, g);
        std::vector<State> nt_states(nt_table.size(), kNothing);
        size_t ungenerative_nt_count = nt_table.size();

        std::stack<TokenKey> dfs_stack;

        for (const auto& [nt_key, multirrs] : g.multirules) {
            for (const auto& rrs : multirrs) {
                if (rrs.nt_indexes.empty()) {
                    nt_states[nt_table[nt_key]] = kGenerative;
                    --ungenerative_nt_count;
                    dfs_stack.push(nt_key);
                }
            }
        }

        const auto isNonterminalGenerative = [&](TokenKey key) {
            for (const auto& rrs : g.multirules[key]) {
                bool isGenerativeRule = true;

                for (const auto rrs_nt_index : rrs.nt_indexes) {
                    const auto rrs_nt_key = rrs.sequence[rrs_nt_index];

                    isGenerativeRule &= static_cast<bool>(nt_states[nt_table[rrs_nt_key]] & kGenerative);

                    if (!isGenerativeRule) {
                        break;
                    }
                }

                if (isGenerativeRule) {
                    return true;
                }
            }
            return false;
        };
        TokenKey cur;

        while (!dfs_stack.empty()) {
            cur = dfs_stack.top();
            dfs_stack.pop(); 

            for (TokenKey generating_nt_key : nt_generating_nts[cur]) {
                 if (nt_states[nt_table[generating_nt_key]] & kGenerative) {
                     continue;
                 }

                 if (isNonterminalGenerative(generating_nt_key)) {
                     nt_states[nt_table[generating_nt_key]] = kGenerative;
                     --ungenerative_nt_count;
                     dfs_stack.push(generating_nt_key);
                 }
            }
        }


        // Phase 2: erasing ungenerative nonterminals
        // Phase 2.1: erase their multirules
        std::unordered_set<TokenKey> ungenerative_nt_keys;
        ungenerative_nt_keys.reserve(ungenerative_nt_count);

        for (auto it = g.multirules.begin(); it != g.multirules.end();) {
            auto nt_key = it->first;
            if (nt_states[nt_table[nt_key]] & kGenerative) {
                ++it;
                continue;
            }

            // todo: remove debug
            std::cout << g.tntable.table[nt_key].token << std::endl;

            ungenerative_nt_keys.insert(nt_key);
            g.tntable.erase(nt_key, TokenType::kNonterminal);
            it = g.multirules.erase(it);  // todo: handle terminals in g.tntable
        }

        // Phase 2.2: erase all the rules where such nonterminals appear
        for (auto& [nt_key, multirrs] : g.multirules) {
            // todo: remove debug
            std::cout << "Removing ungenerative rules for the nonterminal " << g.tntable.table[nt_key].token << std::endl;

            std::cout << "Before:\n";
            for (const auto& rrs : multirrs) {
                outputRuleRightSide(std::cout, rrs, g.tntable.table);
                std::cout << std::endl;
            }

            auto rm_it = std::remove_if(multirrs.begin(),
                                        multirrs.end(),
                                        [&](const RuleRightSide& rrs) {
                for (const auto rrs_nt_index : rrs.nt_indexes) {
                    const auto rrs_nt_key = rrs.sequence[rrs_nt_index];

                    if (ungenerative_nt_keys.find(rrs_nt_key) != ungenerative_nt_keys.end()) {
                        return true;
                    }
                }

                return false;
            });
            multirrs.erase(rm_it, multirrs.end());

            std::cout << "\nAfter:\n";
            for (const auto& rrs : multirrs) {
                outputRuleRightSide(std::cout, rrs, g.tntable.table);
                std::cout << std::endl;
            }
        }
    }

    TokenKey insertUniqueNonterminal(Grammar& g) {
        // todo: remove global static, it's better to make it depend on a TokenTable object
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

        std::vector<TokenKey> new_nt_keys(rrs.sequence.size());
        auto replaceTerminal = [&](ssize_t j) {
            TokenKey unique_nt_key = insertUniqueNonterminal(g);
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
     * A -> a_1...a_n, where a_i is a terminal and at least one a_j is non-empty
     * A -> BC\n
     * A -> ""\n
     * A -> B\n
     *
     * The function adds a unique start S for the grammar, removes the pattern A -> ""
     * by creating new rules of other types and adds S -> "" only and
     * only if the "" string is generated by the grammar
     */
    void congregateEmptyGeneratingNonterminals(Grammar& g) {
        using State = unsigned short;

        static const State kNothing = 0b00;
        static const State kEmptyGenerative = 0b01;
        static const State kSeen = 0b10;

        // TODO: it's better to do a insertUniqueNonterminal that takes a given prefix
        // so that I can make "unique_start_".
        static const auto addUniqueStart = [](Grammar& g) {
            TokenKey unique_start = insertUniqueNonterminal(g);
            g.multirules[unique_start].push_back(RuleRightSide{{g.start}, {0}});
            g.start = unique_start;
        };

        auto empty_it = g.tntable.rtable.find("");

        if (empty_it == g.tntable.rtable.end() ||
            (g.tntable.table[empty_it->second].type & TokenType::kTerminal) == TokenType::kNothing) {
            addUniqueStart(g);
            return;
        }

        // Phase 1: searching empty generating nonterminals
        // Phase 1.1: construct a reversed graph on the nonterminals
        //   so that each nonterminal is connected with those which
        //   can produce an output containing the nonterminal
        std::map<TokenKey, std::set<TokenKey>> nt_generating_nts;

        for (const auto& [nt_key, multirrs] : g.multirules) {
            for (const auto& rrs : multirrs) {
                if (rrs.nt_indexes.empty()) {
                    continue;
                }

                for (const auto rrs_nt_index : rrs.nt_indexes) {
                    const auto rrs_nt_key = rrs.sequence[rrs_nt_index];

                    if (rrs_nt_key != nt_key) {
                        nt_generating_nts[rrs_nt_key].insert(nt_key);
                    }
                }
            }
        }

        // Phase 1.2: searching basic empty generating nonterminals (which have an empty rule)
        const TokenKey empty_key = empty_it->second;
        const auto isEmptyRule = [empty_key](const RuleRightSide& rrs) {
            return rrs.nt_indexes.empty() && std::all_of(rrs.sequence.begin(), rrs.sequence.end(), [empty_key](TokenKey key) {
                return key == empty_key;
            });
        };

        algo::NonterminalTokenKeyTable nt_table;
        algo::initNonterminalTokenKeyTable(nt_table, g);
        std::vector<State> nt_states(nt_table.size(), kNothing);

        const auto isEmptyGeneratingRule = [&](const RuleRightSide& rrs) {
            return rrs.nt_indexes.empty() && std::all_of(rrs.sequence.begin(), rrs.sequence.end(), [&](TokenKey nt_key) {
                return nt_states[nt_key] & kEmptyGenerative;
            });
        };
        const auto isEmptyGenerativeNonterminal = [&](const TokenKey key) {
            return std::any_of(g.multirules[key].begin(), g.multirules[key].end(), [&](const RuleRightSide& rrs) {
                return isEmptyGeneratingRule(rrs);
            });
        };

        std::unordered_set<TokenKey> nts_with_empty_rule;
        nts_with_empty_rule.reserve(nt_table.size());

        for (const auto& [nt_key, multirrs] : g.multirules) {
            for (const auto& rrs : multirrs) {
                if (isEmptyRule(rrs)) {
                    nt_states[nt_table[nt_key]] = kEmptyGenerative;
                    nts_with_empty_rule.insert(nt_key);
                    break;
                }
            }
        }

        // Phase 1.3: searching other empty generating nonterminals through DFS
        {
            std::stack<TokenKey> dfs_stack;
            TokenKey cur;
    
            for (auto nt_key : nts_with_empty_rule) {
                dfs_stack.push(nt_key);
            }
    
            while (!dfs_stack.empty()) {
                cur = dfs_stack.top();
                dfs_stack.pop();
    
                for (const auto nt_key : nt_generating_nts[cur]) {
                    if (nt_states[nt_table[nt_key]] & kEmptyGenerative) {
                        continue;
                    }
    
                    if (isEmptyGenerativeNonterminal(nt_key)) {
                        nt_states[nt_table[nt_key]] |= kEmptyGenerative;
                        dfs_stack.push(nt_key);
                    }
                }
            }
        }

        // Phase 2: searching new rules for each nonterminal by running DFS from the nonterminal
        std::map<TokenKey, std::set<TokenKey>> multirrs_to_add;

        {
            struct DFSState {
                TokenKey nt_key;
                MultiruleRightSide::iterator rrs_it;
            };

            std::vector<State> dfs_nt_states(nt_states.size(), kNothing);
            std::stack<DFSState> dfs_stack;
            std::pair<TokenKey, RuleRightSide> fixed_rule;
            const auto testChainRule = [&](const RuleRightSide& rrs) -> std::pair<bool, std::vector<TokenKey>> {
                if (rrs.nt_indexes.empty()) {
                    return {false, {}};
                }

                if (rrs.nt_indexes.size() == 1) {
                    auto key = rrs.sequence[0];

                    if (dfs_nt_states[nt_table[key]] & kSeen) {
                        return {false, {}};
                    } else {
                        return {true, {key}};
                    }
                }

                std::vector<TokenKey> accepted_nts;
                accepted_nts.reserve(2);

                for (int i = 0; i < 2; ++i) {
                    if (dfs_nt_states[nt_table[rrs.sequence[i]]] & kSeen) {
                        continue;
                    }

                    if (dfs_nt_states[nt_table[rrs.sequence[1 - i]]] & kEmptyGenerative) {
                        accepted_nts.push_back(rrs.sequence[i]);
                    }
                }

                return {!accepted_nts.empty(), std::move(accepted_nts)};
            };
    
            for (auto& [nt_key, multirrs] : g.multirules) {
                dfs_nt_states = nt_states;
                dfs_nt_states[nt_table[nt_key]] |= kSeen;
                dfs_stack.push({nt_key, multirrs.begin()});
    
                while (!dfs_stack.empty()) {
                    auto& cur = dfs_stack.top();
                    const auto& cur_multirrs = g.multirules[cur.nt_key];
                    
                    if (cur.rrs_it == cur_multirrs.end()) {
                        dfs_stack.pop();
                    } else {
                        if (cur.rrs_it == cur_multirrs.begin()) {
                            auto& rrs_to_add = multirrs_to_add[nt_key];

                            for (const auto& rrs: cur_multirrs) {
                                if (rrs.nt_indexes.size() != 2) {
                                    continue;
                                }

                                for (int i = 0; i < 2; ++i) {
                                    if (dfs_nt_states[nt_table[rrs.sequence[1 - i]]] & kEmptyGenerative) {
                                        rrs_to_add.insert(rrs.sequence[i]);
                                    }
                                }
                            }
                        }

                        for (; cur.rrs_it != cur_multirrs.end(); ++cur.rrs_it) {
                            const auto& [is_chain_rule, next_nt_keys] = testChainRule(*cur.rrs_it);

                            if (!is_chain_rule) {
                                continue;
                            }

                            for (auto next_nt_key : next_nt_keys) {
                                dfs_nt_states[nt_table[next_nt_key]] |= kSeen;
                                dfs_stack.push({next_nt_key, g.multirules[next_nt_key].begin()});
                            }
                        }
                    }
                }
            }
        }
       
        // Phase 3: adding new rules
        for (const auto& [nt_key, rrs_to_add] : multirrs_to_add) {
            for (const auto rrs_nt_key : rrs_to_add) {
                g.multirules[nt_key].push_back({{rrs_nt_key}, {0}});
            }
        }

        // Phase 4: adding a unique grammar start, adding the empty rule if needed
        auto old_start = g.start;

        addUniqueStart(g);

        if (nt_states[nt_table[old_start]] & kEmptyGenerative) {
            g.multirules[g.start].push_back({{empty_key}, {}});
        }

        // Phase 5: deleting empty rules
        for (const auto nt_key : nts_with_empty_rule) {
            auto& multirrs = g.multirules[nt_key];
            auto rm_it = std::remove_if(multirrs.begin(), multirrs.end(), [&](const RuleRightSide& rrs) {
                return rrs.nt_indexes.empty() && isEmptyRule(rrs);
            });
            multirrs.erase(rm_it, multirrs.end());
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

        deleteUnreachableNonterminals(g);
        std::cout << g << "\n*******************************Unreachable" << std::endl;
        deleteUngenerativeNonterminals(g);
        std::cout << g << "\n*******************************Ungenerative" << std::endl;
        deleteUnreachableNonterminals(g);
        std::cout << g << "\n*******************************Unreachable" << std::endl;
        deleteMixedAndLongRules(g);
        std::cout << g << "\n*******************************MixedAndLongRules" << std::endl;
        congregateEmptyGeneratingNonterminals(g);
        std::cout << g << '\n' << g.tntable.table[g.start].token << "\n*******************************EmptyGenerating" << std::endl;
        deleteNonterminalChains(g);
        std::cout << g << "\n*******************************Nonterminal" << std::endl;
        deleteUnreachableNonterminals(g);
        std::cout << g << "\n*******************************Unreachable\n";
        deleteUngenerativeNonterminals(g);
        std::cout << g << "\n*******************************Ungenerative\n";
        deleteUnreachableNonterminals(g);
        std::cout << g << "\n*******************************Unreachable\n";
    }

}  // namespace fl::algo
