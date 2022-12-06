#ifndef _INCLUDE_GRAMMARALGORITHMS_H
#define _INCLUDE_GRAMMARALGORITHMS_H

#include "Grammar.h"

namespace details {

    void convertToChomskyForm(Grammar& g);

    void CYKRecognize(const Grammar& g, const std::string& text);

}  // namespace details

#endif  // _INCLUDE_GRAMMARALGORITHMS_H
