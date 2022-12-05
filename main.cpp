#include <iostream>
#include <vector>
#include <string>
#include <optional>

constexpr const char* const help_string =
"gc-cykp: Grammar Converter and CYK Parser\n"
"USAGE:\n"
"   gc-cykp -C <phase_number> [-s <converted_grammar_file>] <grammar_file>\n"
"   gc-cykp -R <text_file> [-s <converted_grammar_file>] [-n] <grammar_file>\n"
"OPTIONS:\n"
"   -R - recognition mode\n"
"       -n - no grammar convertation, a grammar must be already in the Chomsky form\n"
"   -C - convertation only mode\n"
"   -s - save a converted grammar in a file\n";

struct ParsedArguments {
    enum class ProgramMode {
        k_Unknown,
        k_Recognition,
        k_Convertation
    };

    bool need_help = false;
    bool is_already_converted = false;
    ProgramMode mode;
    std::optional<int> convertation_end_phase = std::nullopt;
    std::optional<std::string> text_filename = std::nullopt;
    std::optional<std::string> grammar_filename = std::nullopt;
    std::optional<std::string> converted_grammar_filename = std::nullopt;
};

void parseArguments(ParsedArguments& pargs, std::vector<std::string> &args) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i][0] != '-' || args[i].size() < 2) {
            pargs.mode = ParsedArguments::ProgramMode::k_Unknown;
            return;
        }

        switch (args[i][1]) {
            case 'h': {
                pargs.need_help = true;
                break;
            }

            case 'R': {
                pargs.mode = ParsedArguments::ProgramMode::k_Recognition;

                try {
                    pargs.text_filename = args.at(i + 1);
                }
                catch (...) {
                    pargs.mode = ParsedArguments::ProgramMode::k_Unknown;
                    return;
                }

                break;
            }

            case 'C': {
                pargs.mode = ParsedArguments::ProgramMode::k_Convertation;

                try {
                    pargs.convertation_end_phase = stoi(args.at(i + 1));
                }
                catch (...) {
                    pargs.mode = ParsedArguments::ProgramMode::k_Unknown;
                    return;
                }

                break;
            }

            case 's': {
                break;
            }

            default: {
                pargs.mode = ParsedArguments::ProgramMode::k_Unknown;
                return;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cout << "The program interrupted its execution: no arguments provided.\n"
                "For more information, execute the program with \"-h\" flag.\n";
        return 0;
    }

    std::vector<std::string> args;
    args.reserve(argc);

    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    ParsedArguments pargs;

    parseArguments(pargs, args);

    if (pargs.need_help) {
        std::cout << help_string;
        return 0;
    }

    switch (pargs.mode) {
        case ParsedArguments::ProgramMode::k_Convertation:
            break;
        case ParsedArguments::ProgramMode::k_Recognition:
            break;
        default:
            std::cout << "The program interrupted its execution: incorrect arguments.\n"
                    "Please follow the patterns provided on the help page. You can pass"
                    " -h argument to print it.\n";
    }

    return 0;
}
