#include "CYK_Algorithm.h"


namespace cyk_details {

    namespace extra {

        size_t initMetaVector(MetaVector& mvec, const details::Grammar& g) {
            mvec.resize(g.tntable.table.size());

            size_t terminal_count = 0;

            for (auto& record : g.tntable.table) {
                if ((record.second.type & details::TableValue::k_Nonterminal) == 0) continue;

                mvec[record.first] = terminal_count;
                ++terminal_count;
            }

            return terminal_count;
        }

        void initTerminalMultirulesVector(details::MultirulesMap& mm, const details::Grammar& g) {
            for (auto& [nt_index, rrs_vec] : g.multirules) {
                for (auto& rrs : rrs_vec) {
                    if (!rrs.nt_indexes.empty()) continue;

                    mm[nt_index].push_back(rrs);
                }
            }
        }

        bool isTerminalRecognized(const std::string_view sv,
                                  const details::RuleRightSide& rrs,
                                  const details::TokenTable::Table& table) {
            std::string concat_s = "";

            for (auto& t_code : rrs.sequence) {
                concat_s += table.at(t_code).token;
            }

            return sv == concat_s;
        }

        void initRecognitionVector(std::vector<SubstringVector>& v,
                                   const std::string& text,
                                   MetaVector& mvec,
                                   const details::Grammar& g) {
            details::MultirulesMap terminal_multirules;
            std::string_view text_sv;

            initTerminalMultirulesVector(terminal_multirules, g);

            for (size_t len = 0; len <= text.size(); ++len) {
                for (size_t pos = 0; pos + len <= text.size(); ++pos) {
                    for (auto& [nt_code, rrs_vec] : terminal_multirules) {
                        for (auto& rrs : rrs_vec) {
                            v[mvec[nt_code]][len][pos] = isTerminalRecognized(text_sv.substr(pos, len),
                                                                            rrs,
                                                                            g.tntable.table);
                        }
                    }
                }
            }
        }
        
    }  // namespace extra
    
    bool isRecognized(const std::string& text, const details::Grammar& g) {
        using namespace cyk_details::extra;

        MetaVector meta_vector;

        size_t terminal_count = initMetaVector(meta_vector, g);

        std::vector<SubstringVector> is_recognized(terminal_count, SubstringVector(text.size() + 1));

        // is_recognized[meta_table[nt_code]][length][position] == true,
        // if there is an output for the grammar to text[position:position + length]
        initRecognitionVector(is_recognized, text, meta_vector, g);

        size_t nt_meta_code;
        size_t a_meta_code;
        size_t b_meta_code;

        for (size_t len = 2; len <= text.size(); ++len) {
            for (size_t pos = 0; pos + len <= text.size(); ++pos) {
                for (auto& [nt_code, rrs_vec] : g.multirules) {
                    nt_meta_code = meta_vector[nt_code];

                    for (auto& rrs : rrs_vec) {
                        if (rrs.nt_indexes.empty()) continue;

                        a_meta_code = meta_vector[rrs.sequence[0]];
                        b_meta_code = meta_vector[rrs.sequence[1]];

                        for (size_t k = 0; !is_recognized[nt_meta_code][len][pos] &&
                                           k <= len; ++k) {
                            is_recognized[nt_meta_code][len][pos] =
                                    is_recognized[a_meta_code][k][pos] &&
                                    is_recognized[b_meta_code][len - k][pos + k];
                        }
                    }
                }
            }
        }

        return is_recognized[meta_vector[g.start]][text.size()][0];
    }

}  // namespace cyk_details
