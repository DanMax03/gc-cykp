#ifndef _INCLUDE_GRAMMAR_H
#define _INCLUDE_GRAMMAR_H

#include <iostream>
#include <map>
#include <vector>
#include <string>

namespace details {

    struct RuleRightSide {
        std::vector<std::string> rrs;
        std::vector<size_t> nonterminal_indexes;
    };

    struct Grammar {
        std::map<std::string, RuleRightSide> rules;
    };

    std::istream& operator>>(std::istream& in, Grammar& g);
    std::ostream& operator<<(std::ostream& out, const Grammar& g);

}  // namespace details

#endif  // _INCLUDE_GRAMMAR_H
