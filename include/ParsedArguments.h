#pragma once

#include <optional>

namespace ui {
    struct ParsedArguments {
        enum class ProgramMode {
            kUnknown,
            kRecognition,
            kConvertation
        };
    
        bool need_help = false;
        bool is_already_converted = false;
        ProgramMode mode = ProgramMode::kUnknown;
        std::optional<int> convertation_end_phase = std::nullopt;
        std::optional<std::string> text_filename = std::nullopt;
        std::string grammar_filename;
        std::optional<std::string> converted_grammar_filename = std::nullopt;
    };
}  // namespace ui
