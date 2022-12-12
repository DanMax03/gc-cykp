#ifndef _INCLUDE_CYK_ALGORITHM_H
#define _INCLUDE_CYK_ALGORITHM_H

#include "Grammar.h"

#include <string_view>

namespace cyk_details {

    using SubstringVector = std::vector<std::vector<bool>>;
    using MetaVector = std::vector<size_t>;

    namespace extra {

        void initMetaVector(MetaVector& mt, details::Grammar& g);

        void initTerminalMultirulesVector(details::MultirulesMap& mm);

        bool isTerminalRecognized(const std::string_view sv,
                                  const details::RuleRightSide& rrs,
                                  const details::TokenTable::Table& table);

        void initRecognitionVector(std::vector<SubstringVector>& v,
                                   std::string& text,
                                   MetaVector& mt,
                                   details::Grammar& g);
    
    }  // namespace extra

    bool isRecognized(const std::string& text, const details::Grammar& g);

}  // namespace cyk_details

#endif  // _INCLUDE_CYK_ALGORITHM_H
