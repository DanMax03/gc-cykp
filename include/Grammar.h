#ifndef _INCLUDE_GRAMMAR_H
#define _INCLUDE_GRAMMAR_H

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <exception>

// I widely use the following short forms:
// t = terminal
// nt = nonterminal
namespace details {

    struct TableValue {
        using TokenType = int;

        static const TokenType k_Nothing = 0b0;
        static const TokenType k_Terminal = 0b1;
        static const TokenType k_Nonterminal = 0b10;

        std::string token;
        TokenType type = k_Nothing;
    };

    // rtable stays for a reversed table
    struct TwoDirectionsTable {
        using Table = std::map<size_t, TableValue>;
        using ReversedTable = std::map<std::string, size_t>;

        Table table;
        ReversedTable rtable;

        size_t insert(std::string&& s, TableValue::TokenType type);
        void erase(size_t x);
        void clear() noexcept;
    };

    struct RuleRightSide {
        std::vector<size_t> sequence;
        std::vector<size_t> nt_indexes;
    };

    bool isRuleRightSidesEqual(const RuleRightSide& a,
                               const RuleRightSide& b,
                               const std::map<size_t, TableValue>& table);
    
    void outputRuleRightSide(std::ostream& out,
                             const RuleRightSide& rrs,
                             const TwoDirectionsTable& tdtable);

    struct Grammar {
        TwoDirectionsTable tdtable;
        std::map<size_t, std::vector<RuleRightSide>> multirules;
        size_t start;

        void clear() noexcept;
    };

    bool operator==(const Grammar& a, const Grammar& b);

    class rule_violation : public std::exception {
    public:
        using std::exception::exception;
        rule_violation(const char* message);

        const char* what() const noexcept override;

    private:
        const char* m_message{nullptr};
    };

    std::istream& operator>>(std::istream& in, Grammar& g);
    std::ostream& operator<<(std::ostream& out, const Grammar& g);

}  // namespace details

#endif  // _INCLUDE_GRAMMAR_H
