#include "Talker.h"

#include <exception>

namespace details {
    
    void Talker::sendMessage(const char* c_str) const {
        if (c_str == nullptr) throw std::invalid_argument("");

        out << c_str;
    }

    void Talker::sendHelpMessage() const {
        if (help_string) {
            out << help_string;
        }
    }

    void Talker::sendTerminationMessage(const char* c_str) const {
        if (c_str == nullptr) throw std::invalid_argument;

        if (termination_string) {
            out << termination_string;
        }

        out << c_str;
    }

}  // namespace details
