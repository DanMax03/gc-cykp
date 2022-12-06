#ifndef _INCLUDE_TALKER_H
#define _INCLUDE_TALKER_H

#include <iostream>

namespace ci {

    constexpr const char* const std_term_string = "The program has interrupted its execution: ";
    
    constexpr const char* const std_final_string = "For more information, execute the program with \"-h\" flag.\n";


    struct Talker {
        void sendMessage(const char* c_str, bool add_final_string = false) const;
        void sendHelpMessage() const;
        void sendTerminationMessage(const char* c_str) const;
    
        std::ostream& out = std::cout;
        const char* help_string = nullptr;
        const char* termination_string = std_term_string;
        const char* final_string = std_final_string;
    };

}  // namespace ci

#endif  // _INCLUDE_TALKER_H
