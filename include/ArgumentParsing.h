#pragma once

#include "ParsedArguments.h"

namespace logic {
    class ExceptionController;
}  // namespace logic

namespace ui {
    ParsedArguments parseArguments(logic::ExceptionController& exceptor, int argc, char* argv[]);
}  // namespace ui
