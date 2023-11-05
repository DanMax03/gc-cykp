#pragma once

#include "ExceptionController.h"

namespace logic {
    class Application {
    public:
        int exec(int argc, char* argv[]);

    private:
        void execRecognition();
        void execConvertation();

    private:
        ui::Talker m_talker;
        ExceptionController m_exceptor;
    };
}  // namespace logic