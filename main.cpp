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


void parseArguments(ci::ParsedArguments& pargs, std::vector<std::string> &args) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i][0] != '-') {
            if (i + 1 == args.size()) {
                pargs.grammar_filename = args[i];
            } else if (args[i].size() < 2) {
                pargs.mode = ci::ParsedArguments::ProgramMode::k_Unknown;
            }
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

                    if (args.at(i + 1)[0] == '-') throw 1;

                    ++i;
                    pargs.text_filename = args[i];

                    break;
                }

                case 'n': {
                    pargs.is_already_converted = true;
                    break;
                }
    
                case 'C': {
                    pargs.mode = ci::ParsedArguments::ProgramMode::k_Convertation;

                    if (args.at(i + 1)[0] == '-') throw 1;

                    ++i;
                    pargs.convertation_end_phase = stoi(args[i]);

                    if (pargs.convertation_end_phase < 0) throw 1;

                    break;
                }
    
                case 's': {
                    ++i;
                    pargs.converted_grammar_filename = args.at(i);
                    break;
                }
    
                default: {
                    std::cout << "Entered default\n";
                    pargs.mode = ci::ParsedArguments::ProgramMode::k_Unknown;
                    return;
                }
            }
        }
        catch (...) {
            std::cout << "Entered catch\n";
            pargs.mode = ci::ParsedArguments::ProgramMode::k_Unknown;
            return;
        }
    }
}


int main(int argc, char* argv[]) {
    ci::Talker talker{.help_string = help_string};

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

    if (pargs.grammar_filename == std::string()) {
        talker.sendTerminationMessage("hasn't found a grammar filename.\n"
                                      "Note that it must be the last "
                                      "argument of the execution command.\n");
        return 1;
    }
   
    int ret_value = 0;

    switch (pargs.mode) {
        case ci::ParsedArguments::ProgramMode::k_Convertation:
            ret_value = details::execConvertation(pargs);
            break;
        case ci::ParsedArguments::ProgramMode::k_Recognition:
            ret_value = details::execRecognition(pargs);
            break;
        default:
            talker.sendTerminationMessage("incorrect arguments.\n"
                                          "Please, follow the patterns provided on the help page. ");
            ret_value = 1;
    }

    return ret_value;
}
