#include "NonterminalCompression.h"

namespace fl::algo {
    void initNonterminalTokenKeyTable(NonterminalTokenKeyTable& nt_table, const Grammar& g) {
        nt_table.reserve(g.tntable.nt_count);

        size_t nonterminal_count = 0;

        for (const auto& [key, entry] : g.tntable.table) {
            if ((entry.type & TokenType::kNonterminal) == TokenType::kNothing) {
                continue;
            }

            nt_table[key] = nonterminal_count++;
        }
    }
}  // namespace fl::algo