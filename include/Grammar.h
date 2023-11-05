#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <exception>
#include <functional>

// I widely use the following short forms:
// t = terminal
// nt = nonterminal
namespace fl {
    enum class TokenType : int {
        kNothing = 0b00,
        kTerminal = 0b01,
        kNonterminal = 0b10
    };

    TokenType operator^(TokenType a, TokenType b);
    TokenType operator|(TokenType a, TokenType b);

    void assignXor(fl::TokenType& ref, fl::TokenType type);
    void assignOr(fl::TokenType& ref, fl::TokenType type);


    struct TableEntry {
        std::string token;
        TokenType type = TokenType::kNothing;
    };

    /**
     * TokenKey is a one-to-one mapping with tokens, which are strings
     */
    using TokenKey = size_t;

    /**
     * TokenTable is a structure to effectively handle all
     * kind of tokens that appear in a grammar
     */
    struct TokenTable {
        // todo: remove token duplication
        using Table = std::map<TokenKey, TableEntry>;
        using ReversedTable = std::map<std::string, TokenKey>;

        Table table;
        ReversedTable rtable;

        TokenKey insert(std::string&& s, TokenType type);
        void erase(TokenKey key, TokenType type);
        void clear() noexcept;
    };

    /**
     * sequence holds all the TokenKeys from the right side of a rule
     * nt_indexes - indexes of TokenKeys in the sequence which TokenType is kNonTerminal
     */
    struct RuleRightSide {
        std::vector<TokenKey> sequence;
        std::vector<size_t> nt_indexes;

        void pushTerminal(TokenKey key);
        void pushNonterminal(TokenKey key);
    };

    bool isRuleRightSidesEqual(const RuleRightSide& a,
                               const RuleRightSide& b,
                               const TokenTable::Table& a_table,
                               const TokenTable::Table& b_table);
    
    void outputRuleRightSide(std::ostream& out,
                             const RuleRightSide& rrs,
                             const TokenTable& tntable);

    using MultiruleRightSide = std::vector<RuleRightSide>;
    using MultirulesMap = std::map<TokenKey, MultiruleRightSide>;

    struct Grammar {
        TokenTable tntable;
        MultirulesMap multirules;
        TokenKey start;

        void clear() noexcept;
    };

    bool operator==(const Grammar& a, const Grammar& b);
    std::istream& operator>>(std::istream& in, Grammar& g);
    std::ostream& operator<<(std::ostream& out, const Grammar& g);

    class GrammarBuilder {
    public:
        GrammarBuilder();
        explicit GrammarBuilder(Grammar& g);

        void addRule(std::string&& nonterminal);
        void addRuleRightSide();
        void pushToken(std::string&& token, TokenType type);

        Grammar&& getGrammar() &&;

    private:
        using GrammarRefWrapper = std::reference_wrapper<Grammar>;

        std::optional<Grammar> m_owned_g;
        GrammarRefWrapper m_g;
        TokenKey m_cur_nt_key{0};
    };

    class GrammarInputException : public std::exception {
    public:
        using std::exception::exception;

    public:
        explicit GrammarInputException(const char* message);

        [[nodiscard]] const char* what() const noexcept override;

    private:
        const char* m_message{nullptr};
    };
}  // namespace fl
