#pragma once

#include "Grammar.h"

namespace fl::algo {
    using NonterminalTokenKeyTable = std::unordered_map<TokenKey, size_t>;

    void initNonterminalTokenKeyTable(NonterminalTokenKeyTable& nt_table, const Grammar& g);
}  // namespace fl::algo