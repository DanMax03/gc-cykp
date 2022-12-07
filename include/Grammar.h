#ifndef _INCLUDE_GRAMMAR_H
#define _INCLUDE_GRAMMAR_H

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <exception>

namespace details {

    struct RuleRightSide {
        std::vector<std::string> sequence;
        std::vector<size_t> nonterminal_indexes;

        friend bool operator==(const RuleRightSide& a, const RuleRightSide& b);
    };

    std::ostream& operator<<(std::ostream& out, const RuleRightSide& rrs);

    struct Grammar {
        std::map<std::string, std::vector<RuleRightSide>> multirules;
        std::string start;

        void clear() noexcept;
    };

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
