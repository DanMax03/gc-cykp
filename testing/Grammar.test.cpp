#include "Grammar.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>

#include <gtest/gtest.h>


using details::TableValue;
using details::Grammar;
using details::rule_violation;


namespace {
    void testInputWithExpectedMessagePart(std::ifstream& in, Grammar& g, const char* const message_part) {
        try {
            in >> g;
            FAIL();
        }
        catch (rule_violation& v) {
            ASSERT_TRUE(std::strstr(v.what(), message_part) != nullptr);
        }
        catch (std::exception& e) {
            FAIL() << e.what() << std::endl;
        }
        catch (...) {
            FAIL();
        }
    }
}


TEST(GrammarIOSuite, EmptyFileTest) {
    std::ifstream fin("assets/empty_grammar.txt");
    Grammar g;

    ASSERT_NO_THROW(fin >> g);
    ASSERT_TRUE(g.multirules.empty());
}

TEST(GrammarIOSuite, MultipleRulesWithSimilarLeftSideTest) {
    std::ifstream fin("assets/multiple_right_sides_grammar.txt");

    ASSERT_TRUE(fin.good());

    Grammar g;
    Grammar xptd_g;

    std::vector<std::string> words = {"input", "", "abc", "line", "test", "magic", "why", "how"};
    std::vector<TableValue::TokenType> words_type(words.size());
    std::vector<size_t> words_i(words.size());

    words_type[0] = words_type[3] = TableValue::k_Terminal;

    for (size_t j = 0; j < words_i.size(); ++j) {
        words_i[j] = xptd_g.tdtable.insert(std::move(words[j]), words_type[j]);
    }

    size_t input_i = words_i[0];
    size_t line_i = words_i[3];

    xptd_g.multirules[input_i].push_back({{words_i[1]}, {}});
    xptd_g.multirules[input_i].push_back({{words_i[2]}, {}});
    xptd_g.multirules[line_i].push_back({{words_i[4]}, {}});
    xptd_g.multirules[line_i].push_back({{words_i[5]}, {}});
    xptd_g.multirules[input_i].push_back({{words_i[6]}, {}});
    xptd_g.multirules[input_i].push_back({{words_i[7]}, {}});

    ASSERT_NO_THROW(fin >> g);

    ASSERT_EQ(g, xptd_g);
}

TEST(GrammarIOSuite, NotClosedQuoteTest) {
    std::ifstream fin("assets/not_closed_quote_grammar.txt");

    ASSERT_TRUE(fin.good());

    Grammar g;

    testInputWithExpectedMessagePart(fin, g, "quotes must be closed");
}

TEST(GrammarIOSuite, EscapedQuoteTest) {
    std::ifstream fin("assets/escaped_quote_grammar.txt");

    ASSERT_TRUE(fin.good());

    Grammar g;
    Grammar xptd_g;

    std::vector<std::string> words = {"input", "line", "", "\"", "abc"};
    std::vector<TableValue::TokenType> words_type(words.size());
    std::vector<size_t> words_i(words.size());

    words_type[0] = words_type[1] = TableValue::k_Nonterminal;

    for (size_t j = 0; j < words_i.size(); ++j) {
        words_i[j] = xptd_g.tdtable.insert(std::move(words[j]), words_type[j]);
    }

    size_t input_i = words_i[0];
    size_t line_i = words_i[1];

    auto& input_rules = xptd_g.multirules[input_i];
    auto& line_rules = xptd_g.multirules[line_i];

    input_rules.insert(input_rules.begin(), {{{words_i[2]}, {}},
                                             {{line_i}, {0}},
                                             {{words_i[3], line_i, words_i[3]}, {1}}});
    line_rules.insert(line_rules.begin(), {{{words_i[4]}, {}}});
    
    ASSERT_NO_THROW(fin >> g);
    ASSERT_EQ(g, xptd_g);
}

TEST(GrammarIOSuite, MultilineTerminalTest) {
    std::ifstream fin("assets/multiline_terminal_grammar.txt");

    ASSERT_TRUE(fin.good());

    Grammar g;
    Grammar xptd_g;

    std::vector<std::string> words = {"input",
                                      "line",
                                      "",
                                      "will you work correctly?",
                                      "(",
                                      ")"};
    std::vector<TableValue::TokenType> words_type(words.size());
    std::vector<size_t> words_i(words.size());

    words_type[0] = words_type[1] = TableValue::k_Nonterminal;

    for (size_t j = 0; j < words_i.size(); ++j) {
        words_i[j] = xptd_g.tdtable.insert(std::move(words[j]), words_type[j]);
    }

    size_t input_i = words_i[0];
    size_t line_i = words_i[1];

    auto& input_rules = xptd_g.multirules[input_i];
    auto& line_rules = xptd_g.multirules[line_i];

    input_rules.insert(input_rules.begin(), {{{words_i[2]}, {}},
                                            {{line_i}, {0}}});

    line_rules.insert(line_rules.begin(), {{{words_i[3]}, {}},
                                           {{words_i[4], line_i, words_i[5]}, {1}}});

    ASSERT_NO_THROW(fin >> g);
    ASSERT_EQ(g, xptd_g);
}

TEST(GrammarIOSuite, IllegalNonterminalTest) {
    std::ifstream fin("assets/illegal_nonterminal_grammar.txt");

    ASSERT_TRUE(fin.good());

    Grammar g;

    testInputWithExpectedMessagePart(fin, g, "invalid nonterminal");
}

TEST(GrammarIOSuite, RuleWithTerminalInLeftSideTest) {
    std::ifstream fin;

    Grammar g;

    for (size_t i = 1; i <= 2; ++i) {
        fin.open("assets/terminal_in_left_side_grammar" + std::to_string(i) + ".txt");
    
        ASSERT_TRUE(fin.good());

        testInputWithExpectedMessagePart(fin, g, "but met a terminal");
    
        fin.close();
    }
}

TEST(GrammarIOSuite, RuleWithWrongVerticalBar) {
    std::ifstream fin("assets/vertical_bar_with_wrong_position_grammar.txt");

    ASSERT_TRUE(fin.good());

    Grammar g;

    testInputWithExpectedMessagePart(fin, g, "cannot be used before ':'");
}

TEST(GrammarIOSuite, UnfinishedLastRuleTest) {
    std::ifstream fin;

    Grammar g;

    for (size_t i = 1; i <= 3; ++i) {
        fin.open("assets/unfinished_grammar" + std::to_string(i) + ".txt");

        ASSERT_TRUE(fin.good());

        testInputWithExpectedMessagePart(fin, g, "the last rule is not finished");

        fin.close();
    }
}

TEST(GrammarIOSuite, OutputTest) {
    std::ifstream fin;
    std::stringstream sstream;

    Grammar g;
    std::string s1, s2;

    for (size_t i = 1; i <= 2; ++i) {
        fin.open("assets/correct_grammar" + std::to_string(i) + ".txt");

        sstream.clear();
        sstream.str("");

        ASSERT_TRUE(fin.good());

        ASSERT_NO_THROW(fin >> g);

        sstream << g;

        fin.clear();
        fin.seekg(0, std::ios::beg);

        sstream.clear();
        sstream.seekg(0, std::ios::beg);

        while (true) {
            std::getline(fin, s1);
            std::getline(sstream, s2);

            if (fin.eof() || sstream.eof()) break;

            ASSERT_EQ(s1, s2);
        }
        
        ASSERT_TRUE(fin.eof() && sstream.eof());

        fin.close();
    }
}

