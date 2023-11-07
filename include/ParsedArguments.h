#pragma once

#include <optional>
#include <filesystem>

namespace ui {
    struct ParsedArguments {
        using Path = std::filesystem::path;

        enum class ProgramMode {
            kUnknown,
            kRecognition,
            kConversion
        };
    
        bool need_help = false;
        bool is_already_converted = false;
        ProgramMode mode = ProgramMode::kUnknown;
        std::optional<int> conversion_end_phase;
        std::optional<Path> text_filename;
        Path grammar_filename;
        std::optional<Path> converted_grammar_filename;
    };
}  // namespace ui
