#include "Application.h"

#include "ArgumentParsing.h"
#include "ParsedArguments.h"

#include <filesystem>

namespace logic {
    void Application::validatePaths(ui::ParsedArguments& pargs) {
        using namespace std::filesystem;

        auto check_path = [this](const path& p, const std::string& path_name) {
            if (!exists(p)) {
                m_exceptor.sendException("The " + path_name + " doesn't exist.\n");
            }

            if (!is_regular_file(p)) {
                m_exceptor.sendException("The " + path_name + " doesn't point to a regular file.\n");
            }
        };

        check_path(pargs.grammar_filename, "grammar path");

        if (pargs.text_filename.has_value()) {
            check_path(*pargs.text_filename, "text path");
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

        validatePaths(pargs);

        switch (pargs.mode) {
            case ProgramMode::kUnknown:
                m_exceptor.sendException("Incorrect arguments, no mode provided.\n");
            case ProgramMode::kConversion:
                execRecognition(pargs);
                break;
            case ProgramMode::kRecognition:
                execConversion(pargs);
                break;
        }

        return 0;
    }
}  // namespace logic