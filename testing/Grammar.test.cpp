#include "Grammar.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>

#include <gtest/gtest.h>


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

TEST(GrammarIOSuite, MultipleRulesWithSimilarRightSideTest) {
    std::ifstream fin("assets/multiple_right_sides_grammar.txt");

    ASSERT_TRUE(fin.good());

    Grammar g;
    Grammar xptd_g;

    xptd_g.multirules["input"].push_back({{""}, {}});
    xptd_g.multirules["input"].push_back({{"abc"}, {}});
    xptd_g.multirules["line"].push_back({{"test"}, {}});
    xptd_g.multirules["line"].push_back({{"magic"}, {}});
    xptd_g.multirules["input"].push_back({{"why"}, {}});
    xptd_g.multirules["input"].push_back({{"how"}, {}});

    ASSERT_NO_THROW(fin >> g);
    ASSERT_EQ(g.multirules, xptd_g.multirules);
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

    auto& input_rules = xptd_g.multirules["input"];
    auto& line_rules = xptd_g.multirules["line"];

    input_rules.insert(input_rules.begin(), {{{""}, {}},
                                             {{"line"}, {0}},
                                             {{"\"", "line", "\""}, {1}}});
    line_rules.insert(line_rules.begin(), {{{"abc"}, {}}});
    
    ASSERT_NO_THROW(fin >> g);
    ASSERT_EQ(g.multirules, xptd_g.multirules);
}

TEST(GrammarIOSuite, MultilineTerminalTest) {
    std::ifstream fin("assets/multiline_terminal_grammar.txt");

    ASSERT_TRUE(fin.good());

    Grammar g;
    Grammar xptd_g;

    auto& input_rules = xptd_g.multirules["input"];
    auto& line_rules = xptd_g.multirules["line"];

    input_rules.insert(input_rules.begin(), {{{""}, {}},
                                            {{"line"}, {0}}});

    line_rules.insert(line_rules.begin(), {{{"will you work correctly?"}, {}},
                                           {{"(", "line", ")"}, {1}}});

    ASSERT_NO_THROW(fin >> g);
    ASSERT_EQ(g.multirules, xptd_g.multirules);
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

        testInputWithExpectedMessagePart(fin, g, "must start from a nonterminal");
    
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

