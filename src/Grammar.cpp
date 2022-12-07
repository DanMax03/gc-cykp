#include "Grammar.h"

#include <stack>
#include <set>
#include <string_view>

namespace {

    enum class LastToken {
        k_Nothing,
        k_Nonterminal,
        k_Terminal
    };

    const char* const left_side_violation_string = "a rule must start from a nonterminal and"
                                                   "contain in its left side the nonterminal only.\n";

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

    bool operator==(const RuleRightSide& a, const RuleRightSide& b) {
        return a.sequence == b.sequence && a.nonterminal_indexes == b.nonterminal_indexes;
    }

    std::ostream& operator<<(std::ostream& out, const RuleRightSide& rrs) {
        size_t prefix_size = rrs.nonterminal_indexes.empty() ? rrs.sequence.size() : rrs.nonterminal_indexes.front();

        for (size_t i = 0; i < prefix_size; ++i) {
            out << "\"" << rrs.sequence[i] << "\" ";
        }

        if (rrs.nonterminal_indexes.empty()) return out;

        for (size_t i = 1; i < rrs.nonterminal_indexes.size(); ++i) {
            out << rrs.sequence[rrs.nonterminal_indexes[i - 1]] << " ";
            for (size_t j = rrs.nonterminal_indexes[i - 1] + 1; j < rrs.nonterminal_indexes[i]; ++j) {
                out << "\"" << rrs.sequence[j] << "\" ";
            }
        }

        out << rrs.sequence[rrs.nonterminal_indexes.back()] << " ";

        for (size_t i = rrs.nonterminal_indexes.back() + 1; i < rrs.sequence.size(); ++i) {
            out << "\"" << rrs.sequence[i] << "\" ";
        }

        return out;
    }

    rule_violation::rule_violation(const char* message)
            : m_message(message) {}

    const char* rule_violation::what() const noexcept {
        return m_message;
    }

    void Grammar::clear() noexcept {
        multirules.clear();
        start.clear();
    }

    std::istream& operator>>(std::istream& in, Grammar& g) {
        g.clear();

        std::string s;
        std::string cur_nonterminal;
        LastToken last_token = LastToken::k_Nothing;
        bool isRuleRightSide = false;
        bool isEscaped;
        size_t l, r;
        size_t line_index = 0;  // todo: provide line_index for a user

        while (std::getline(in, s)) {
            if (s.back() == '\n') {
                s.pop_back();
            }

            for (l = 0, r = 0; r < s.size(); ++r) {
                if (s[r] == ' ') continue;

                if (s[r] == '#') break;

                if (s[r] == ':') {
                    if (last_token != LastToken::k_Nonterminal) {
                        throw rule_violation(left_side_violation_string);
                    }

                    if (isRuleRightSide) {
                        throw rule_violation("the ':' symbol must appear only "
                                             "once during a rule definition.\n");
                    }

                    isRuleRightSide = true;
                    g.multirules[cur_nonterminal].push_back(RuleRightSide());
                    continue;
                }

                if (s[r] == '|') {
                    if (!isRuleRightSide) {
                        throw rule_violation("the '|' symbol cannot be used before ':'.\n");
                    }

                    if (g.multirules[cur_nonterminal].back().sequence.empty()) {
                        throw rule_violation("the right side of a rule cannot be empty.\n");
                    }

                    g.multirules[cur_nonterminal].push_back(RuleRightSide());
                    last_token = LastToken::k_Nothing;
                    continue;
                }

                if (s[r] == ';') {
                    if (!isRuleRightSide) {
                        throw rule_violation("expected the ':' symbol, but met ';'.\n");
                    }

                    if (g.multirules[cur_nonterminal].back().sequence.empty()) {
                        throw rule_violation("a rule cannot be empty this way. "
                                             "Add \"\" to the right side.\n");
                    }

                    last_token = LastToken::k_Nothing;
                    isRuleRightSide = false;
                    cur_nonterminal.clear();
                    continue;
                }

                if (s[r] == '"') {
                    isEscaped = false;
                    l = r;
                    ++r;

                    while (r < s.size() && (s[r] != '"' || isEscaped)) {
                        if (s[r] == '\\') {
                            isEscaped = !isEscaped;
                        } else {
                            isEscaped = false;
                        }

                        ++r;
                    }

                    if (r == s.size()) {
                        throw rule_violation("every sequence in \"\"-quotes must "
                                             "be closed on the same line.\n");
                    }

                    if (!isRuleRightSide) {
                        throw rule_violation(left_side_violation_string);
                    }

                    auto& cur_rule = g.multirules[cur_nonterminal].back();

                    if (last_token == LastToken::k_Terminal) {
                        cur_rule.sequence.back().append(substr_without_escape_sequences(s, l + 1, r));
                    } else {
                        cur_rule.sequence.push_back(substr_without_escape_sequences(s, l + 1, r));
                    }

                    last_token = LastToken::k_Terminal;

                    continue;
                }

                l = r;
                ++r;

                while (r < s.size() && s[r] != ' ') {
                    ++r;
                }
                
                if (!isValidNonterminal(l, r, s)) {
                    throw rule_violation("invalid nonterminal. Probably, you put "
                                         "illegal letters inside of it.\n");
                }

                if (cur_nonterminal.empty()) {
                    cur_nonterminal = s.substr(l, r - l);
                    g.multirules.insert({cur_nonterminal, {}});

                    if (g.start.empty()) {
                        g.start = cur_nonterminal;
                    }
                } else {
                    if (!isRuleRightSide) {
                        throw rule_violation("expected ':' symbol, but found a nonterminal.\n");
                    }

                    auto& cur_rule = g.multirules[cur_nonterminal].back();
                    cur_rule.sequence.push_back(s.substr(l, r - l));
                    cur_rule.nonterminal_indexes.push_back(cur_rule.sequence.size() - 1);
                }

                last_token = LastToken::k_Nonterminal;
            }

            ++line_index;
        }

        if (isRuleRightSide || last_token != LastToken::k_Nothing) {
            throw rule_violation("the last rule is not finished.\n");
        }

        return in;
    }

    std::ostream& operator<<(std::ostream& out, const Grammar& g) {
        if (g.start.empty()) return out;

        std::stack<std::string_view> bfs_stack;
        std::set<std::string_view> shown;
        std::string s;

        bfs_stack.push(g.start);

        while (!bfs_stack.empty()) {
            s = bfs_stack.top();
            bfs_stack.pop();

            out << s << "\n";

            auto& rrs_set = g.multirules.find(s)->second;

            out << ": " << rrs_set.front() << "\n";

            for (size_t i = 1; i < rrs_set.size(); ++i) {
                out << "| " << rrs_set[i] << "\n";
            }

            out << ";\n";

            for (auto& rrs : rrs_set) {
                for (auto& index : rrs.nonterminal_indexes) {
                    if (shown.find(rrs.sequence[index]) != shown.end()) continue;

                    shown.insert(rrs.sequence[index]);
                    bfs_stack.push(rrs.sequence[index]);
                }
            }
        }

        return out;
    }
}
