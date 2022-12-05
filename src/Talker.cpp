#include "Talker.h"

#include <exception>

namespace ci {
    
    void Talker::sendMessage(const char* c_str, bool add_final_string) const {
        if (c_str == nullptr) throw std::invalid_argument("Talker::sendMessage cannot take nullptr argument");

        out << c_str;

        if (add_final_string && final_string != nullptr) {
            out << final_string;
        }
    }

    void Talker::sendHelpMessage() const {
        if (help_string) {
            out << help_string;
        }
    }

    void Talker::sendTerminationMessage(const char* c_str) const {
        if (c_str == nullptr) throw std::invalid_argument("Talker::sendTerminationMessage cannot take nullptr argument");

        if (termination_string) {
            out << termination_string;
        }

        out << c_str;

        if (final_string != nullptr) {
            out << final_string;
        }
    }

}  // namespace ci

