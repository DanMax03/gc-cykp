#pragma once

#include "ExceptionController.h"
#include "Talker.h"

namespace logic {
    class Application {
    public:
        int exec(int argc, char* argv[]);

    private:
        void execRecognition();
        void execConvertation();

    private:
        ExceptionController m_exceptor;
        std::shared_ptr<ui::Talker> m_talker;
    };
}  // namespace logic