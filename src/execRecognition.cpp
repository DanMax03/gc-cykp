#include "execRecognition.h"

#include <iostream>
#include <fstream>

#include "ParsedArguments.h"
#include "Talker.h"
#include "Grammar.h"
#include "GrammarAlgorithms.h"


namespace {

    void readText(std::ifstream& fin, std::string& text) {
        fin.seekg(0, std::ios::end);
        text.resize(fin.tellg());
        fin.seekg(0, std::ios::beg);
        fin.read(&text[0], text.size());
    }

}  // namespace

namespace details {

    [[nodiscard]] int execRecognition(const ci::ParsedArguments& pargs) {
        ci::Talker talker;

        Grammar g;
        std::string text;

        try {
            if (!pargs.text_filename) {
                throw std::invalid_argument("a text file is not provided.\n");
            }

            std::ifstream grammar_fin(pargs.grammar_filename);
            std::ifstream text_fin(pargs.text_filename.value());
            std::ofstream fout;

            if (!text_fin.good()) {
                throw std::invalid_argument("the text file path is incorrect.\n");
            }

            if (!grammar_fin.good()) {
                throw std::invalid_argument("the grammar file path is incorrect.\n");
            }

            grammar_fin >> g;
            readText(text_fin, text);

            if (pargs.converted_grammar_filename) {
                fout.open(pargs.converted_grammar_filename.value());

                if (!fout.good()) {
                    throw std::invalid_argument("the path for a converted grammar is incorrect.\n");
                }
            }

            if (pargs.is_already_converted) {
                if (!isInChomskyForm(g)) {
                    throw std::invalid_argument("the grammar is said to be in Chomsky form, but it is not.\n");
                }
            } else {
                convertToChomskyForm(g);
            }

            if (fout.is_open()) {
                fout << g;
            } else {
                std::cout << "The converted grammar:\n" << g << "\n";
            }

            bool recognition_res = cyk_details::isRecognized(text, g);

            std::cout << std::string(recognition_res ? "Yes" : "No") +
                         ", the text is" +
                         std::string(recognition_res ? " " : " not ") +
                         "recognized by the grammar." << std::endl;
        }
        catch(std::exception& e) {
            talker.sendTerminationMessage(e.what());
            return 1;
        }


        return 0;
    }

}  // namespace details

