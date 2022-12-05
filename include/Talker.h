#pragma once

#include <iostream>

namespace details {

    struct Talker {
        void sendMessage(const char* c_str) const;
        void sendHelpMessage() const;
        void sendTerminationMessage(const char* c_str) const;
    
        std::ostream& out = std::cout;
        const char* help_string = nullptr;
        const char* termination_string = nullptr;
    };

}  // namespace details

