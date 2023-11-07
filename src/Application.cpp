#include "Application.h"

#include "ArgumentParsing.h"
#include "ParsedArguments.h"

#include <filesystem>

namespace logic {
    Application::Application()
        : m_talker(std::make_unique<ui::Talker>(m_exceptor)) {
        m_exceptor.setTalker(m_talker);
    }

    void Application::preparePaths(ui::ParsedArguments& pargs) {
        using namespace std::filesystem;

        auto preparePath = [this](path& p, const std::string& path_name) {
            if (!exists(p)) {
                m_exceptor.sendException("The " + path_name + " doesn't exist.\n");
            }

            if (!is_regular_file(p)) {
                m_exceptor.sendException("The " + path_name + " doesn't point to a regular file.\n");
            }
        };

        preparePath(pargs.grammar_filename, "grammar path");

        if (pargs.text_filename.has_value()) {
            preparePath(*pargs.text_filename, "text path");
        }

        if (pargs.converted_grammar_filename.has_value()) {
            if (!exists(pargs.converted_grammar_filename->parent_path())) {
                m_exceptor.sendException("The save directory for the converted grammar doesn't exist");
            }
        }
    }

    int Application::exec(int argc, char** argv) {
        using ProgramMode = ui::ParsedArguments::ProgramMode;
        auto pargs = ui::parseArguments(m_exceptor, argc, argv);

        if (pargs.need_help) {
            m_talker->sendHelpMessage();
            return 0;
        }

        preparePaths(pargs);

        switch (pargs.mode) {
            case ProgramMode::kUnknown:
                m_exceptor.sendException("Incorrect arguments, no mode provided.\n");
            case ProgramMode::kConversion:
                execConversion(pargs);
                break;
            case ProgramMode::kRecognition:
                execRecognition(pargs);
                break;
        }

        return 0;
    }
}  // namespace logic