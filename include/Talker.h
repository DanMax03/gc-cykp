#ifndef _INCLUDE_TALKER_H
#define _INCLUDE_TALKER_H

#include <iostream>

namespace ci {

    struct Talker {
        void sendMessage(const char* c_str, bool add_final_string = false) const;
        void sendHelpMessage() const;
        void sendTerminationMessage(const char* c_str) const;
    
        std::ostream& out = std::cout;
        const char* help_string = nullptr;
        const char* termination_string = nullptr;
        const char* final_string = nullptr;
    };

}  // namespace ci

#endif  // _INCLUDE_TALKER_H
