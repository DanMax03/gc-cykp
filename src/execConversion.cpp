#include "Application.h"

#include <iostream>
#include <fstream>
#include <memory>
#include <map>
#include <exception>

#include "Grammar.h"
#include "GrammarAlgorithms.h"

namespace logic {
    void Application::execConversion(const ui::ParsedArguments& pargs) {
        fl::Grammar g;

        try {
            std::ifstream fin(pargs.grammar_filename);
            std::ofstream fout;

            if (!fin.good()) {
                throw std::invalid_argument("the grammar path is incorrect.\n");
            }

            fin >> g;

            if (pargs.converted_grammar_filename) {
                fout.open(pargs.converted_grammar_filename.value());

                if (!fout.good()) {
                    throw std::invalid_argument("the path for a converted grammar is incorrect.\n");
                }
            }


            fl::algo::convertToChomskyForm(g, *pargs.convertation_end_phase);

            if (fout.is_open()) {
                fout << g;
            } else {
                std::cout << g;
            }
        }
        catch (std::exception& e) {
            m_exceptor.sendException(e.what());
        }
    }
}  // namespace logic

