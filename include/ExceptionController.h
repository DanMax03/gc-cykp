#pragma once

#ifndef EXCEPTION_POLICY_INDEX
#define EXCEPTION_POLICY_INDEX 0
#endif

#include <memory>

namespace ui {
    class Talker;
}

namespace logic {
    class ExceptionController {
    public:
        void setTalker(const std::shared_ptr<ui::Talker>& talker);
        void sendException(const char* msg) noexcept(EXCEPTION_POLICY_INDEX == 0);

    private:
        std::shared_ptr<ui::Talker> m_talker;
    };
}  // namespace logic