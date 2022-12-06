#include "execConvertation.h"

#include <iostream>
#include <fstream>
#include <memory>
#include <map>
#include <exception>

#include "ParsedArguments.h"
#include "Talker.h"
#include "Grammar.h"
#include "GrammarAlgorithms.h"

namespace details {

    [[nodiscard]] int execConvertation(const ci::ParsedArguments& pargs) {
        ci::Talker talker;

        Grammar g;

        try {
            std::ifstream fin(pargs.grammar_filename);
            std::ofstream fout;

            if (!fin.good()) {
                throw std::invalid_argument("the grammar path is incorrect.\n");
            }

            if (pargs.converted_grammar_filename) {
                fout.open(pargs.converted_grammar_filename.value());

                if (!fout.good()) {
                    throw std::invalid_argument("the path for a converted grammar is incorrect.\n");
                }
            }
            
            fin >> g;

            convertToChomskyForm(g);

            if (fout.is_open()) {
                fout << g;
            } else {
                std::cout << g;
            }
        }
        catch (std::exception& e) {
            talker.sendTerminationMessage(e.what());
            return 1;
        }

        return 0;
    }

}  // namespace details

