#include "Application.h"

#include <iostream>
#include <fstream>

#include "Grammar.h"
#include "GrammarAlgorithms.h"


namespace {
    void readText(std::ifstream& fin, std::string& text) {
        fin.seekg(0, std::ios::end);
        auto text_size = fin.tellg();
        text.resize(text_size);
        fin.seekg(0, std::ios::beg);
        fin.read(&text[0], text_size);
    }
}  // namespace

namespace logic {
    void Application::execRecognition(const ui::ParsedArguments& pargs) {
        fl::Grammar g;
        std::string text;

        if (!pargs.text_filename) {
            m_exceptor.sendException("a text file is not provided.\n");
        }

        std::ifstream grammar_fin(pargs.grammar_filename);
        std::ifstream text_fin(*pargs.text_filename);
        std::ofstream fout;

        if (!text_fin.good()) {
            m_exceptor.sendException("failed to open the text file.\n");
        }

        if (!grammar_fin.good()) {
            m_exceptor.sendException("failed to open the grammar file.\n");
        }

        grammar_fin >> g;
        readText(text_fin, text);

        if (pargs.converted_grammar_filename) {
            fout.open(pargs.converted_grammar_filename.value());

            if (!fout.good()) {
                m_exceptor.sendException("failed to open the file for a converted grammar.\n");
            }
        }

        if (pargs.is_already_converted) {
            if (!isInChomskyForm(g)) {
                m_exceptor.sendException("the grammar is said to be in Chomsky form, but it is not.\n");
            }
        } else {
            convertToChomskyForm(g);
        }

        if (fout.is_open()) {
            fout << g;
        } else {
            // todo: toString method for Grammar class, change to m_talker.sendMessage()
            std::cout << "The converted grammar:\n" << g << "\n";
        }

        bool recognition_res = fl::cyk::isRecognized(text, g);

        std::cout << std::string(recognition_res ? "Yes" : "No") +
                     ", the text is" +
                     std::string(recognition_res ? " " : " not ") +
                     "recognized by the grammar." << std::endl;
    }
}  // namespace logic

