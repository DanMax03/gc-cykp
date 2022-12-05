#include <iostream>
#include <vector>
#include <string>

#include "ParsedArguments.h"
#include "Talker.h"
#include "execConvertation.h"
#include "execRecognition.h"


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

constexpr const char* const term_string = "The program has interrupted its execution: ";

constexpr const char* const final_string = "For more information, execute the program with \"-h\" flag.\n";


void parseArguments(ci::ParsedArguments& pargs, std::vector<std::string> &args) {
    size_t i;

    for (i = 0; i + 1 < args.size(); ++i) {
        if (args[i][0] != '-' || args[i].size() < 2) {
            pargs.mode = ci::ParsedArguments::ProgramMode::k_Unknown;
            return;
        }

        try {
            switch (args[i][1]) {
                case 'h': {
                    pargs.need_help = true;
                    break;
                }
    
                case 'R': {
                    pargs.mode = ci::ParsedArguments::ProgramMode::k_Recognition; 
                    ++i;
                    pargs.text_filename = args.at(i);
                    break;
                }

                case 'n': {
                    pargs.is_already_converted = true;
                    break;
                }
    
                case 'C': {
                    pargs.mode = ci::ParsedArguments::ProgramMode::k_Convertation;
                    ++i;
                    pargs.convertation_end_phase = stoi(args.at(i));
                    if (pargs.convertation_end_phase < 0) throw;
                    break;
                }
    
                case 's': {
                    ++i;
                    pargs.converted_grammar_filename = args.at(i);
                    break;
                }
    
                default: {
                    pargs.mode = ci::ParsedArguments::ProgramMode::k_Unknown;
                    return;
                }
            }
        }
        catch (...) {
            pargs.mode = ci::ParsedArguments::ProgramMode::k_Unknown;
            return;
        }
    }

    if (i == args.size()) {
        pargs.mode = ci::ParsedArguments::ProgramMode::k_Unknown;
        return;
    }

    pargs.grammar_filename = args.back();
}


int main(int argc, char* argv[]) {
    ci::Talker talker{.help_string = help_string,
                      .termination_string = term_string,
                      .final_string = final_string};

    if (argc == 1) {
        talker.sendTerminationMessage("no arguments provided.\n");
        return 1;
    }

    std::vector<std::string> args;
    args.reserve(argc);

    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    ci::ParsedArguments pargs;

    parseArguments(pargs, args);

    if (pargs.need_help) {
        talker.sendHelpMessage();
        return 0;
    }

    switch (pargs.mode) {
        case ci::ParsedArguments::ProgramMode::k_Convertation:
            details::execConvertation(pargs);
            break;
        case ci::ParsedArguments::ProgramMode::k_Recognition:
            details::execRecognition(pargs);
            break;
        default:
            talker.sendTerminationMessage("incorrect arguments.\n"
                                          "Please, follow the patterns provided on the help page. ");
            return 1;
    }

    return 0;
}
