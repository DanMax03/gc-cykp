#include "ArgumentParsing.h"

#include "ExceptionController.h"

#include <cstring>

namespace ui {
    ParsedArguments parseArguments(logic::ExceptionController& exceptor, int argc, char* argv[]) {
        if (argc <= 1) {
            exceptor.sendException("no arguments provided.\n");
        }

        using ProgramMode = ui::ParsedArguments::ProgramMode;

        auto argument_exists = [=](int i) {
            return i < argc;
        };
        auto is_argument_flag = [=](int i) {
            return argv[i][0] == '-';
        };
        auto is_flag_short = [=](const char* flag) {
            return flag[1] == '\0' || flag[2] == '\0';
        };
        ParsedArguments pargs;

        for (int i = 1; i < argc; ++i) {
            const char* arg = argv[i];

            if (arg[0] != '-') {
                if (i + 1 < argc) {
                    exceptor.sendException("The " + std::to_string(i) + " argument breaks the arguments pattern.\n");
                } else if (is_argument_flag(i)) {
                    exceptor.sendException("Expected the last argument to be a grammar path.\n");
                }

                try {
                    pargs.grammar_filename = arg;
                }
                catch (...) {
                    exceptor.sendException("Failed to assign a grammar path to std::filesystem::path.\n");
                }

                continue;
            }

            if (!is_flag_short(arg)) {
                exceptor.sendException("Long flags (with more than 1 letter) are not allowed and not used.\n");
            }

            switch (arg[1]) {
                case 'h': {
                    pargs.need_help = true;
                    break;
                }

                case 'R': {
                    pargs.mode = ProgramMode::kRecognition;
                    ++i;

                    if (argument_exists(i) && !is_argument_flag(i)) {
                        try {
                            pargs.text_filename = argv[i];
                        }
                        catch (...) {
                            exceptor.sendException("Failed to assign a text path to std::filesystem::path.\n");
                        }
                    } else {
                        exceptor.sendException("Expected a path after the '-R' flag.\n");
                    }

                    break;
                }

                case 'n': {
                    pargs.is_already_converted = true;
                    break;
                }

                case 'C': {
                    pargs.mode = ProgramMode::kConversion;
                    ++i;

                    if (argument_exists(i) && !is_argument_flag(i)) {
                        // todo: check the length of the argument
                        pargs.conversion_end_phase = std::stoi(argv[i]);

                        if (*pargs.conversion_end_phase < 0) {
                            exceptor.sendException("Expected a positive number after the '-C' flag. Got negative.\n");
                        }
                    } else {
                        exceptor.sendException("Expected a positive number after the '-C' flag.\n");
                    }

                    break;
                }

                case 's': {
                    ++i;

                    if (argument_exists(i) && !is_argument_flag(i)) {
                        try {
                            pargs.converted_grammar_filename = argv[i];
                        }
                        catch (...) {
                            exceptor.sendException("Failed to assign a saved grammar path to std::filesystem::path.\n");
                        }
                    } else {
                        exceptor.sendException("Expected a path after the '-s' flag.\n");
                    }

                    break;
                }

                default: {
                    exceptor.sendException("Got unexpected flag in the arguments.\n");
                    break;
                }
            }
        }
        
        return pargs;
    }
}  // namespace ui