#pragma once

#include "Grammar.h"

#include <string_view>

namespace fl::cyk {
    using SubstringVector = std::vector<std::vector<bool>>;
    using MetaVector = std::vector<size_t>;

    namespace cyk_fl {

        void initMetaVector(MetaVector& mt, fl::Grammar& g);

        void initTerminalMultirulesVector(fl::MultirulesMap& mm);

        bool isTerminalRecognized(const std::string_view sv,
                                  const fl::RuleRightSide& rrs,
                                  const fl::TokenTable::Table& table);

        void initRecognitionVector(std::vector<SubstringVector>& v,
                                   std::string& text,
                                   MetaVector& mt,
                                   fl::Grammar& g);

    }  // namespace extra

    bool isRecognized(const std::string& text, const fl::Grammar& g);
}  // namespace fl::cyk
