#ifndef _INCLUDE_PARSEDARGUMENTS_H
#define _INCLUDE_PARSEDARGUMENTS_H

#include <optional>

namespace ci {

    struct ParsedArguments {
        enum class ProgramMode {
            k_Unknown,
            k_Recognition,
            k_Convertation
        };
    
        bool need_help = false;
        bool is_already_converted = false;
        ProgramMode mode = ProgramMode::k_Unknown;
        std::optional<int> convertation_end_phase = std::nullopt;
        std::optional<std::string> text_filename = std::nullopt;
        std::optional<std::string> grammar_filename = std::nullopt;
        std::optional<std::string> converted_grammar_filename = std::nullopt;
    };
   
}  // namespace ci

#endif  // _INCLUDE_PARSEDARGUMENTS_H
