#include "ExceptionController.h"

#include "Talker.h"

#include <string>
#include <stdexcept>

namespace logic {
    void ExceptionController::setTalker(const std::shared_ptr<ui::Talker>& talker) {
        m_talker = talker;
    }

    void ExceptionController::sendException(const char* msg) noexcept(EXCEPTION_POLICY_INDEX == 0) {
        switch (EXCEPTION_POLICY_INDEX) {
            case 0:
                m_talker->sendTerminationMessage(msg);
                std::terminate();
            case 1:
                throw std::runtime_error(msg);
        }
    }

    void ExceptionController::sendException(std::string_view msg) noexcept(EXCEPTION_POLICY_INDEX == 0) {
        sendException(msg.data());
    }
}  // namespace logic