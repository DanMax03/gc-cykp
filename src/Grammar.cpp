#include "Grammar.h"

namespace details {
    std::istream& operator>>(std::istream& in, Grammar& g) {
        std::string s;

        while (in >> s) {
            
        }

        return in;
    }

    std::ostream& operator<<(std::ostream& out, const Grammar& g) {
        return out;
    }
}
