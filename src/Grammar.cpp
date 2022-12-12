#include "Grammar.h"

#include <queue>
#include <set>
#include <string_view>

namespace {

    enum class LastToken {
        k_Nothing,
        k_Nonterminal,
        k_Terminal
    };

    bool isValidNonterminal(size_t l, size_t r, std::string& s) {
        for (size_t i = l; i < r; ++i) {
            if (s[i] == ':' ||
                s[i] == ';' ||
                s[i] == '"' ||
                s[i] == '\\' ||
                s[i] == '|') return false;
        }

        return true;
    }

    // it must be guaranteed that the last symbol is not backslash
    std::string substr_without_escape_sequences(std::string& s, size_t l, size_t r) {
        std::string res;
        res.reserve(r - l);

        for (size_t i = l; i < r; ++i) {
            if (s[i] != '\\') {
                res.push_back(s[i]);
                continue;
            }

            ++i;

            switch (s[i]) {
                case 'a': res.push_back('\a'); break;
                case 'b': res.push_back('\b'); break;
                case 'f': res.push_back('\f'); break;
                case 'n': res.push_back('\n'); break;
                case 'r': res.push_back('\r'); break;
                case 't': res.push_back('\t'); break;
                case 'v': res.push_back('\v'); break;
                case '\\': res.push_back('\\'); break;
                case '\'': res.push_back('\''); break;
                case '"': res.push_back('"'); break;
                case '?': res.push_back('?'); break;
                default: throw std::invalid_argument("met illegal escape sequence.\n");
            }
        }

        return res;
    }

}

namespace details {

    size_t TwoDirectionsTable::insert(std::string&& s, TableValue::TokenType token_type) {
        auto it1 = rtable.find(s);

        if (it1 != rtable.end()) {
            table[it1->second].type |= token_type;
            return it1->second;
        }
        
        auto it2 = table.emplace(table.size(), TableValue{std::move(s), token_type}).first;

        rtable.emplace(it2->second.token, rtable.size());

        return it2->first;
    }

    void TwoDirectionsTable::erase(size_t x) {
        auto it = table.find(x);
        rtable.erase(it->second.token);
        table.erase(it);
    }

    void TwoDirectionsTable::clear() noexcept {
        table.clear();
        rtable.clear();
    }

    bool isRuleRightSidesEqual(const RuleRightSide& a,
                               const RuleRightSide& b,
                               const TwoDirectionsTable& a_table,
                               const TwoDirectionsTable& b_table) {
        if (a.nt_indexes != b.nt_indexes) return false;

        for (size_t nt_ind : a.nt_indexes) {
            if (a_table.table.at(a.sequence[nt_ind]).token != b_table.table.at(b.sequence[nt_ind]).token) return false;
        }

        if (a.nt_indexes.empty()) {
            for (size_t i = 0; i < a.sequence.size(); ++i) {
                if (a_table.table.at(a.sequence[i]).token != a_table.table.at(b.sequence[i]).token) return false;
            }

            return true;
        }

        for (size_t i = 0; i < a.nt_indexes[0]; ++i) {
            if (a_table.table.at(a.sequence[i]).token != a_table.table.at(b.sequence[i]).token) return false;
        }

        for (size_t i = 1; i < a.nt_indexes.size(); ++i) {
            for (size_t j = a.nt_indexes[i - 1] + 1; j < a.nt_indexes[i]; ++j) {
                if (a_table.table.at(a.sequence[j]).token != b_table.table.at(b.sequence[j]).token) return false;
            }
        }

        for (size_t i = a.nt_indexes.back() + 1; i < a.sequence.size(); ++i) {
            if (a_table.table.at(a.sequence[i]).token != b_table.table.at(b.sequence[i]).token) return false;
        }

        return true;
    }

    // TODO: escape sequences are not handled
    void outputRuleRightSide(std::ostream& out,
                             const RuleRightSide& rrs,
                             const std::map<size_t, TableValue>& table) {
        size_t prefix_size = rrs.nt_indexes.empty() ? rrs.sequence.size() : rrs.nt_indexes.front();

        for (size_t i = 0; i < prefix_size; ++i) {
            out << "\"" << table.at(rrs.sequence[i]).token << "\" ";
        }

        if (rrs.nt_indexes.empty()) return;

        for (size_t i = 1; i < rrs.nt_indexes.size(); ++i) {
            out << table.at(rrs.sequence[rrs.nt_indexes[i - 1]]).token << " ";
            for (size_t j = rrs.nt_indexes[i - 1] + 1; j < rrs.nt_indexes[i]; ++j) {
                out << "\"" << table.at(rrs.sequence[j]).token << "\" ";
            }
        }

        out << table.at(rrs.sequence[rrs.nt_indexes.back()]).token << " ";

        for (size_t i = rrs.nt_indexes.back() + 1; i < rrs.sequence.size(); ++i) {
            out << "\"" << table.at(rrs.sequence[i]).token << "\" ";
        }
    }

    rule_violation::rule_violation(const char* message)
            : m_message(message) {}

    const char* rule_violation::what() const noexcept {
        return m_message;
    }

    void Grammar::clear() noexcept {
        tdtable.clear();
        multirules.clear();
    }

    bool operator==(const Grammar& a, const Grammar& b) {
        if (a.multirules.size() != b.multirules.size()) return false;

        auto& a_table = a.tdtable;
        auto& b_table = b.tdtable;

        for (auto& multirule : a.multirules) {
            auto& a_rrs_vec = multirule.second;
            auto& b_rrs_vec = b.multirules.at(b_table.rtable.at(a_table.table.at(multirule.first).token));

            if (a_rrs_vec.size() != b_rrs_vec.size()) return false;

            for (size_t i = 0; i < a_rrs_vec.size(); ++i) {
                if (!isRuleRightSidesEqual(a_rrs_vec[i], b_rrs_vec[i], a_table, b_table)) return false;
            }
        }

        return true;
    }

    void flushTerminalBuffer(std::string& tbuf, size_t cur_nt, Grammar& g) {
        const size_t terminal_index = g.tdtable.insert(std::move(tbuf), TableValue::k_Terminal);

        g.multirules[cur_nt].back().sequence.push_back(terminal_index);
        tbuf.clear();
    }

    size_t findTerminalEnd(size_t pos, std::string& s) {
        bool is_escaped = false;
        
        while (pos < s.size() && (s[pos] != '"' || is_escaped)) {
            if (s[pos] == '\\') {
                is_escaped = !is_escaped;
            } else {
                is_escaped = false;
            }

            ++pos;
        }

        return pos;
    }

    std::istream& operator>>(std::istream& in, Grammar& g) {
        g.clear();

        std::string s;
        std::string terminal_buf;
        size_t cur_nonterminal;
        LastToken last_token = LastToken::k_Nothing;
        bool is_rule_right_side = false;
        size_t l, r;
        size_t line_index = 0;  // TODO: provide line_index for a user in exceptions

        while (std::getline(in, s)) {
            if (s.back() == '\n') {
                s.pop_back();
            }

            for (l = 0, r = 0; r < s.size(); ++r) {
                if (s[r] == ' ') continue;

                if (s[r] == '#') break;

                if (s[r] == ':') {
                    if (is_rule_right_side) {
                        throw rule_violation("the ':' symbol must appear only "
                                             "once during a rule definition.\n");
                    }

                    if (last_token == LastToken::k_Nothing) {
                        throw rule_violation("expected a nonterminal, but met ':'.\n");
                    }

                    is_rule_right_side = true;
                    g.multirules[cur_nonterminal].push_back(RuleRightSide());
                    last_token = LastToken::k_Nothing;
                    continue;
                }

                if (s[r] == '|') {
                    if (!is_rule_right_side) {
                        throw rule_violation("the '|' symbol cannot be used before ':'.\n");
                    }

                    if (last_token == LastToken::k_Nothing) {
                        throw rule_violation("the right side of a rule cannot be empty.\n");
                    }

                    if (last_token == LastToken::k_Terminal) {
                        flushTerminalBuffer(terminal_buf, cur_nonterminal, g);
                    }

                    g.multirules[cur_nonterminal].push_back(RuleRightSide());
                    last_token = LastToken::k_Nothing;
                    continue;
                }

                if (s[r] == ';') {
                    if (!is_rule_right_side) {
                        throw rule_violation("expected ':' symbol, but met ';'.\n");
                    }

                    if (last_token == LastToken::k_Nothing) {
                        throw rule_violation("a rule cannot be empty this way. "
                                             "Add \"\" to the right side.\n");
                    }

                    if (last_token == LastToken::k_Terminal) {
                        flushTerminalBuffer(terminal_buf, cur_nonterminal, g);
                    }

                    is_rule_right_side = false;
                    last_token = LastToken::k_Nothing;
                    continue;
                }

                if (s[r] == '"') {
                    l = r;
                    r = findTerminalEnd(r + 1, s);

                    if (r == s.size()) {
                        throw rule_violation("every sequence in \"\"-quotes must "
                                             "be closed on the same line.\n");
                    }

                    if (!is_rule_right_side) {
                        throw rule_violation("expected ':' symbol, but met a terminal.\n");
                    }

                    auto& cur_rule = g.multirules[cur_nonterminal].back();

                    terminal_buf += substr_without_escape_sequences(s, l + 1, r);
                    last_token = LastToken::k_Terminal;
                    continue;
                }

                if (last_token == LastToken::k_Terminal) {
                    flushTerminalBuffer(terminal_buf, cur_nonterminal, g);
                }

                l = r;
                ++r;

                while (r < s.size() && s[r] != ' ') {
                    ++r;
                }
                
                if (!isValidNonterminal(l, r, s)) {
                    throw rule_violation("an invalid nonterminal. Probably, you put "
                                         "illegal letters inside of it.\n");
                }

                if (!is_rule_right_side && last_token == LastToken::k_Nothing) {
                    cur_nonterminal = g.tdtable.insert(s.substr(l, r - l),
                                                       TableValue::k_Nonterminal);

                    if (g.multirules.size() == 0) {
                        g.start = cur_nonterminal;
                    }

                    g.multirules.insert({cur_nonterminal, {}});
                } else {
                    if (!is_rule_right_side) {
                        throw rule_violation("expected ':' symbol, but found a nonterminal.\n");
                    }

                    auto& cur_rule = g.multirules[cur_nonterminal].back();

                    cur_rule.sequence.push_back(g.tdtable.insert(s.substr(l, r - l),
                                                                 TableValue::k_Nonterminal));
                    cur_rule.nt_indexes.push_back(cur_rule.sequence.size() - 1);
                }

                last_token = LastToken::k_Nonterminal;
            }

            ++line_index;
        }

        if (is_rule_right_side || last_token != LastToken::k_Nothing) {
            throw rule_violation("the last rule is not finished.\n");
        }

        return in;
    }

    std::ostream& operator<<(std::ostream& out, const Grammar& g) {
        if (g.multirules.empty()) return out;

        std::queue<size_t> bfs_queue;
        std::set<size_t> shown;
        size_t cur;
        auto& table = g.tdtable.table;

        for (auto& multirule : g.multirules) {
            if (shown.find(multirule.first) != shown.end()) continue;

            bfs_queue.push(multirule.first);
            
            while (!bfs_queue.empty()) {
                cur = bfs_queue.front();
                bfs_queue.pop();
                shown.insert(cur);
    
                out << table.at(cur).token << "\n";
    
                auto& rrs_vec = g.multirules.find(cur)->second;
    
                out << ": ";
                outputRuleRightSide(out, rrs_vec.front(), table);
                out << "\n";
    
                for (size_t i = 1; i < rrs_vec.size(); ++i) {
                    out << "| ";
                    outputRuleRightSide(out, rrs_vec[i], table);
                    out << "\n";
                }
    
                out << ";\n";
    
                for (auto& rrs : rrs_vec) {
                    for (auto& index : rrs.nt_indexes) {
                        if (shown.find(rrs.sequence[index]) != shown.end()) continue;
    
                        shown.insert(rrs.sequence[index]);
                        bfs_queue.push(rrs.sequence[index]);
                    }
                }
            }
        }

        return out;
    }
}
