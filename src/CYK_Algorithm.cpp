#include "CYK_Algorithm.h"

#include "NonterminalCompression.h"

namespace {
    using namespace fl;
    using namespace fl::algo;

    using SubstringVector = std::vector<std::vector<bool>>;

    void initTerminalMultirulesVector(MultirulesMap& mm, const Grammar& g) {
        for (const auto& [nt_index, multirrs] : g.multirules) {
            for (const auto& rrs : multirrs) {
                if (!rrs.nt_indexes.empty()) {
                    continue;
                }

                mm[nt_index].push_back(rrs);
            }
        }
    }

    void initRecognitionVector(std::vector<SubstringVector>& v,
                               const std::string& text,
                               NonterminalTokenKeyTable& nt_table,
                               const Grammar& g) {
        MultirulesMap terminal_multirules;
        std::string_view text_sv;
        initTerminalMultirulesVector(terminal_multirules, g);

        // There may be multiple terminals in one RuleRightSide
        auto isTerminalRecognized = [&](std::string_view sv, const RuleRightSide& rrs) {
            ssize_t pos = 0;

            for (auto t_key : rrs.sequence) {
                auto& token = g.tntable.table.at(t_key).token;

                if (sv.substr(pos, token.size()) != token) {
                    return false;
                }

                pos += static_cast<ssize_t>(token.size());
            }

            return true;
        };

        for (size_t len = 0; len <= text.size(); ++len) {
            for (size_t pos = 0; pos + len <= text.size(); ++pos) {
                for (const auto& [nt_code, multirrs] : terminal_multirules) {
                    for (const auto& rrs : multirrs) {
                        v[len][pos][nt_table[nt_code]] = isTerminalRecognized(text_sv.substr(pos, len), rrs);

                        if (v[len][pos][nt_table[nt_code]]) {
                            break;
                        }
                    }
                }
            }
        }
    }
}  // namespace

namespace fl::algo::cyk {
    // g must be in CNF
    bool isRecognized(const std::string& text, const fl::Grammar& g) {
        NonterminalTokenKeyTable nt_table;
        initNonterminalTokenKeyTable(nt_table, g);
        // is_recognized[length][position][nt_table[nt_code]] == true,
        // if there is an output for the grammar to text[position:position + length]
        // that starts from nt_code
        std::vector<SubstringVector> is_recognized(text.size() + 1,
                                                   SubstringVector(text.size(),
                                                                   std::vector<bool>(g.tntable.nt_count, false)));
        initRecognitionVector(is_recognized, text, nt_table, g);

        size_t nt_code;
        size_t a_nt_code;
        size_t b_nt_code;

        for (size_t len = 2; len <= text.size(); ++len) {
            for (size_t pos = 0; pos + len <= text.size(); ++pos) {
                for (const auto& [nt_key, multirrs] : g.multirules) {
                    nt_code = nt_table[nt_key];

                    for (const auto& rrs : multirrs) {
                        if (rrs.nt_indexes.empty()) {
                            continue;
                        }

                        // Here we depend on CNF. If there are nonterminals
                        // in the rrs, then it's possible only and only when
                        // the rule looks like A -> BC
                        a_nt_code = nt_table[rrs.sequence[0]];
                        b_nt_code = nt_table[rrs.sequence[1]];
                        is_recognized[len][pos][nt_code] = is_recognized[len][pos][a_nt_code] &&
                                is_recognized[0][pos][b_nt_code];

                        for (size_t k = 0; !is_recognized[nt_code][len][pos] && k < len; ++k) {
                            is_recognized[len][pos][nt_code] =
                                    is_recognized[k][pos][a_nt_code] &&
                                    is_recognized[len - k][pos + k][b_nt_code];
                        }
                    }
                }
            }
        }

        return is_recognized[text.size()][0][nt_table[g.start]];
    }
}  // namespace fl::algo::cyk
