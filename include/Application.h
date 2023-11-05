#pragma once

#include "ExceptionController.h"
#include "Talker.h"
#include "ParsedArguments.h"

namespace logic {
    class Application {
    public:
        int exec(int argc, char* argv[]);

    private:
        void validatePaths(ui::ParsedArguments& pargs);
        void execRecognition(const ui::ParsedArguments& pargs);
        void execConversion(const ui::ParsedArguments& pargs);

    private:
        ExceptionController m_exceptor;
        std::shared_ptr<ui::Talker> m_talker;
    };
}  // namespace logic