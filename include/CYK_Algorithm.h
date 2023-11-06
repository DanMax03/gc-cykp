#pragma once

#include "Grammar.h"

#include <string_view>

namespace fl::cyk {
    namespace cyk_details {
        using NonterminalTokenKeyTable = std::unordered_map<TokenKey, size_t>;
        using SubstringVector = std::vector<std::vector<bool>>;

        void initMetaVector(MetaVector& mt, fl::Grammar& g);

        void initTerminalMultirulesVector(fl::MultirulesMap& mm);

        void initRecognitionVector(std::vector<SubstringVector>& v,
                                   std::string& text,
                                   MetaVector& mt,
                                   fl::Grammar& g);

    }  // namespace extra

    bool isRecognized(const std::string& text, const fl::Grammar& g);
}  // namespace fl::cyk
