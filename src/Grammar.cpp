#include "Grammar.h"

#include <algorithm>
#include <queue>
#include <set>
#include <string_view>

namespace {
    bool isValidNonterminal(std::string_view sv) {
        return std::all_of(sv.begin(), sv.end(), [](char ch) {
            return ch != ':' &&
                   ch != ';' &&
                   ch != '"' &&
                   ch != '\\' &&
                   ch != '|';
        });
    }

    // It must be guaranteed that the last symbol is not backslash
    std::string getCollapsedEscapeSequences(std::string_view sv) {
        std::string res;
        res.reserve(sv.size());

        for (ssize_t i = 0; i < sv.size(); ++i) {
            if (sv[i] != '\\') {
                res.push_back(sv[i]);
                continue;
            }

            ++i;

            switch (sv[i]) {
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
}  // namespace

namespace fl {
    TokenType operator^(TokenType a, TokenType b) {
        return static_cast<TokenType>(static_cast<int>(a) ^ static_cast<int>(b));
    }

    TokenType operator|(TokenType a, TokenType b) {
        return static_cast<TokenType>(static_cast<int>(a) | static_cast<int>(b));
    }

    void assignXor(fl::TokenType& ref, fl::TokenType type) {
        ref = ref ^ type;
    }

    void assignOr(fl::TokenType& ref, fl::TokenType type) {
        ref = ref | type;
    }

    TokenKey TokenTable::insert(std::string&& s, TokenType type) {
        auto it1 = rtable.find(s);

        if (it1 != rtable.end()) {
            assignOr(table[it1->second].type, type);
            return it1->second;
        }
        
        auto it2 = table.emplace(table.size(), TableEntry{std::move(s), type}).first;

        rtable.emplace(it2->second.token, rtable.size());

        return it2->first;
    }

    void TokenTable::erase(TokenKey key, TokenType type) {
        auto it = table.find(key);

        assignXor(it->second.type, type);

        if (it->second.type != TokenType::kNothing) return;

        rtable.erase(it->second.token);
        table.erase(it);
    }

    void TokenTable::clear() noexcept {
        table.clear();
        rtable.clear();
    }

    void RuleRightSide::pushTerminal(const TokenKey key) {
        sequence.push_back(key);
    }

    void RuleRightSide::pushNonterminal(const TokenKey key) {
        nt_indexes.push_back(sequence.size());
        sequence.push_back(key);
    }

    bool isRuleRightSidesEqual(const RuleRightSide& a,
                               const RuleRightSide& b,
                               const TokenTable::Table& a_table,
                               const TokenTable::Table& b_table) {
        if (a.nt_indexes != b.nt_indexes || a.sequence.size() != b.sequence.size()) {
            return false;
        }

        for (size_t nt_ind : a.nt_indexes) {
            if (a_table.at(a.sequence[nt_ind]).token != b_table.at(b.sequence[nt_ind]).token) {
                return false;
            }
        }

        if (a.nt_indexes.empty()) {
            for (ssize_t i = 0; i < a.sequence.size(); ++i) {
                if (a_table.at(a.sequence[i]).token != b_table.at(b.sequence[i]).token) {
                    return false;
                }
            }

            return true;
        }

        for (ssize_t i = 0; i < a.nt_indexes[0]; ++i) {
            if (a_table.at(a.sequence[i]).token != a_table.at(b.sequence[i]).token) {
                return false;
            }
        }

        for (ssize_t i = 1; i < a.nt_indexes.size(); ++i) {
            for (ssize_t j = a.nt_indexes[i - 1] + 1; j < a.nt_indexes[i]; ++j) {
                if (a_table.at(a.sequence[j]).token != b_table.at(b.sequence[j]).token) {
                    return false;
                }
            }
        }

        for (ssize_t i = a.nt_indexes.back() + 1; i < a.sequence.size(); ++i) {
            if (a_table.at(a.sequence[i]).token != b_table.at(b.sequence[i]).token) {
                return false;
            }
        }

        return true;
    }

    // TODO: escape sequences are not handled
    void outputRuleRightSide(std::ostream& out,
                             const RuleRightSide& rrs,
                             const std::map<size_t, TableEntry>& table) {
        const bool has_nt_indexes = !rrs.nt_indexes.empty();
        const size_t prefix_size = !has_nt_indexes ? rrs.sequence.size() : rrs.nt_indexes.front();

        for (ssize_t i = 0; i < prefix_size; ++i) {
            out << "\"" << table.at(rrs.sequence[i]).token << "\" ";
        }

        if (!has_nt_indexes) return;

        for (ssize_t i = 1; i < rrs.nt_indexes.size(); ++i) {
            out << table.at(rrs.sequence[rrs.nt_indexes[i - 1]]).token << " ";

            for (ssize_t j = rrs.nt_indexes[i - 1] + 1; j < rrs.nt_indexes[i]; ++j) {
                out << "\"" << table.at(rrs.sequence[j]).token << "\" ";
            }
        }

        out << table.at(rrs.sequence[rrs.nt_indexes.back()]).token << " ";

        for (ssize_t i = rrs.nt_indexes.back() + 1; i < rrs.sequence.size(); ++i) {
            out << "\"" << table.at(rrs.sequence[i]).token << "\" ";
        }
    }

    GrammarInputException::GrammarInputException(const char* message)
            : m_message(message) {}

    const char* GrammarInputException::what() const noexcept {
        return m_message;
    }

    void Grammar::clear() noexcept {
        tntable.clear();
        multirules.clear();
    }

    GrammarBuilder::GrammarBuilder()
        : m_owned_g(Grammar{})
        , m_g(*m_owned_g) {
    }

    GrammarBuilder::GrammarBuilder(Grammar& g)
        : m_g(std::ref(g)) {
    }

    void GrammarBuilder::addRule(std::string&& nonterminal) {
        auto& g = m_g.get();
        m_cur_nt_key = g.tntable.insert(std::move(nonterminal), TokenType::kNonterminal);

        if (g.multirules.empty()) {
            g.start = m_cur_nt_key;
        }

        g.multirules.insert({m_cur_nt_key, {}});
    }

    void GrammarBuilder::addRuleRightSide() {
        m_g.get().multirules[m_cur_nt_key].emplace_back();
    }

    void GrammarBuilder::pushToken(std::string&& token, TokenType type) {
        auto& g = m_g.get();
        auto key = g.tntable.insert(std::move(token), type);
        auto& cur_rule = g.multirules[m_cur_nt_key].back();

        switch (type) {
            case TokenType::kNothing:
                throw std::invalid_argument("tried to push a token with kNothing type.\n");
            case TokenType::kNonterminal:
                cur_rule.pushNonterminal(key);
                break;
            case TokenType::kTerminal:
                cur_rule.pushTerminal(key);
                break;
        }
    }

    Grammar&& GrammarBuilder::getGrammar() && {
        if (m_owned_g.has_value()) {
            return std::move(*m_owned_g);
        } else {
            throw std::runtime_error("Tried to call GrammarBuilder::getGrammar for a non-owning builder.\n");
        }
    }

    bool operator==(const Grammar& a, const Grammar& b) {
        if (a.multirules.size() != b.multirules.size()) {
            return false;
        }

        for (auto& multirule : a.multirules) {
            const auto& a_multirrs = multirule.second;
            const auto& a_multirule_token = a.tntable.table.at(multirule.first).token;
            const auto& b_multirrs = b.multirules.at(b.tntable.rtable.at(a_multirule_token));

            if (a_multirrs.size() != b_multirrs.size()) {
                return false;
            }

            for (ssize_t i = 0; i < a_multirrs.size(); ++i) {
                if (!isRuleRightSidesEqual(a_multirrs[i],
                                           b_multirrs[i],
                                           a.tntable.table,
                                           b.tntable.table)) {
                    return false;
                }
            }
        }

        return true;
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

    size_t findNonterminalEnd(size_t pos, std::string& s) {
        while (pos < s.size() && s[pos] != ' ') {
            ++pos;
        }

        return pos;
    }

    std::istream& operator>>(std::istream& in, Grammar& g) {
        enum class LastToken {
            kNothing,
            kNonterminal,
            kTerminal
        };

        g.clear();

        GrammarBuilder builder(g);
        std::string s;
        std::string terminal_buf;
        LastToken last_token = LastToken::kNothing;
        bool is_rule_right_side = false;
        size_t l, r;
        size_t line_index = 0;  // TODO: provide line_index for a user in exceptions

        auto flushTerminalBuffer = [&]() {
            builder.pushToken(std::move(terminal_buf), TokenType::kTerminal);
            terminal_buf.clear();
        };

        while (std::getline(in, s)) {
            if (s.back() == '\n') {
                s.pop_back();
            }

            for (l = 0, r = 0; r < s.size(); ++r) {
                if (s[r] == ' ') continue;
                if (s[r] == '#') break;

                if (s[r] == ':') {
                    if (is_rule_right_side) {
                        throw GrammarInputException("the ':' symbol must appear only "
                                             "once during a rule definition.\n");
                    }

                    if (last_token == LastToken::kNothing) {
                        throw GrammarInputException("expected a nonterminal, but met ':'.\n");
                    }

                    is_rule_right_side = true;
                    builder.addRuleRightSide();
                    last_token = LastToken::kNothing;
                    continue;
                }

                if (s[r] == '|') {
                    if (!is_rule_right_side) {
                        throw GrammarInputException("the '|' symbol cannot be used before ':'.\n");
                    }

                    if (last_token == LastToken::kNothing) {
                        throw GrammarInputException("the right side of a rule cannot be empty.\n");
                    }

                    if (last_token == LastToken::kTerminal) {
                        flushTerminalBuffer();
                    }

                    builder.addRuleRightSide();
                    last_token = LastToken::kNothing;
                    continue;
                }

                if (s[r] == ';') {
                    if (!is_rule_right_side) {
                        throw GrammarInputException("expected ':' symbol, but met ';'.\n");
                    }

                    if (last_token == LastToken::kNothing) {
                        throw GrammarInputException("a rule cannot be empty this way. "
                                                    "Add \"\" to the right side.\n");
                    }

                    if (last_token == LastToken::kTerminal) {
                        flushTerminalBuffer();
                    }

                    is_rule_right_side = false;
                    last_token = LastToken::kNothing;
                    continue;
                }

                if (s[r] == '"') {
                    l = r;
                    r = findTerminalEnd(r + 1, s);

                    if (r == s.size()) {
                        throw GrammarInputException("every sequence in \"\"-quotes must "
                                                    "be closed in the same line.\n");
                    }

                    if (!is_rule_right_side) {
                        throw GrammarInputException("expected ':' symbol, but met a terminal.\n");
                    }

                    terminal_buf += getCollapsedEscapeSequences(std::string_view(s.c_str() + l + 1, r - l - 1));
                    last_token = LastToken::kTerminal;
                    continue;
                }

                // We get there if only and only when we encounter a nonterminal
                if (last_token == LastToken::kTerminal) {
                    flushTerminalBuffer();
                }

                l = r;
                r = findNonterminalEnd(r + 1, s);
                
                if (!isValidNonterminal(std::string_view(s.c_str() + l, r - l))) {
                    throw GrammarInputException("an invalid nonterminal. Probably, you put "
                                                "illegal letters inside of it.\n");
                }

                if (!is_rule_right_side && last_token == LastToken::kNothing) {
                    builder.addRule(s.substr(l, r - l));
                } else {
                    if (!is_rule_right_side) {
                        throw GrammarInputException("expected ':' symbol, but found a nonterminal.\n");
                    }

                    builder.pushToken(s.substr(l, r - l), TokenType::kNonterminal);
                }

                last_token = LastToken::kNonterminal;
            }

            ++line_index;
        }

        if (is_rule_right_side || last_token != LastToken::kNothing) {
            throw GrammarInputException("the last rule is not finished.\n");
        }

        return in;
    }

    std::ostream& operator<<(std::ostream& out, const Grammar& g) {
        if (g.multirules.empty()) {
            return out;
        }

        std::queue<size_t> bfs_queue;
        std::set<size_t> shown;
        size_t cur;
        auto& table = g.tntable.table;

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
}  // namespace fl
