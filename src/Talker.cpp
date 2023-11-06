#include "Talker.h"

#include "ExceptionController.h"

namespace ui {
    Talker::Talker(logic::ExceptionController& exceptor)
        : m_exceptor(exceptor) {
    }

    void Talker::sendMessage(const char* msg, bool add_final_string) const {
        if (msg == nullptr) {
            m_exceptor.sendException("Talker::sendMessage cannot take nullptr argument");
        }

        out << msg;

        if (add_final_string) {
            sendFinalString();
        }
    }

    void Talker::sendMessage(const std::string& msg, bool add_final_string) const {
        sendMessage(msg.c_str(), add_final_string);
    }

    void Talker::sendHelpMessage() const {
        if (!help_string) {
            m_exceptor.sendException("Talker::sendHelpMessage is called, but help string is not set");
        }

        out << help_string;
    }

    void Talker::sendTerminationMessage(const char* msg) const {
        if (msg == nullptr) {
            m_exceptor.sendException("Talker::sendTerminationMessage cannot take nullptr argument");
        }

        sendTerminationString();

        out << msg;

        sendFinalString();
    }

    void Talker::sendTerminationString() const {
        if (!termination_string) {
            m_exceptor.sendException("Talker::sendTerminationString is called, but termination string is not set");
        }
        out << termination_string;
    }

    void Talker::sendFinalString() const {
        if (!final_string) {
            m_exceptor.sendException("Talker::sendFinalString is called, but final string is not set");
        }
        out << final_string;
    }
}  // namespace ui

