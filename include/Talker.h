#pragma once

#include <iostream>

namespace logic {
    class ExceptionController;
}

namespace ui {
    constexpr const char* const std_help_string =
            "gc-cykp: Grammar Converter and CYK Parser\n"
            "USAGE:\n"
            "   gc-cykp -C <phase_number> [-s <converted_grammar_file>] <grammar_file>\n"
            "   gc-cykp -R <text_file> [-s <converted_grammar_file>] [-n] <grammar_file>\n"
            "OPTIONS:\n"
            "   -R - recognition mode\n"
            "       -n - do not convert a grammar, the grammar must be already in the Chomsky form\n"
            "   -C - convertation only mode\n"
            "   -s - save a converted grammar in a <converted_grammar_file>\n";

    constexpr const char* const std_term_string = "The program has interrupted its execution: ";
    
    constexpr const char* const std_final_string = "For more information, execute the program with \"-h\" flag.\n";


    class Talker {
    public:
        explicit Talker(logic::ExceptionController& exceptor);

        void sendMessage(const char* msg, bool add_final_string = false) const;
        void sendMessage(const std::string& msg, bool add_final_string = false) const;
        void sendHelpMessage() const;
        void sendTerminationMessage(const char* msg) const;

    private:
        void sendTerminationString() const;
        void sendFinalString() const;

    private:
        logic::ExceptionController& m_exceptor;

        std::ostream& out = std::cout;
        const char* help_string = std_help_string;
        const char* termination_string = std_term_string;
        const char* final_string = std_final_string;
    };
}  // namespace ui
