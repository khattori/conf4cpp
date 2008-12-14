/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#ifndef PARSER_HPP
#define PARSER_HPP

#include <set>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <boost/spirit.hpp>
#include <boost/spirit/actor.hpp>
#include <boost/spirit/phoenix.hpp>
#include <conf4cpp/var.hpp>

using namespace std;
using namespace boost::spirit;

namespace conf4cpp
{
    template <typename derived_T>
    struct base_config_parser : public grammar<base_config_parser<derived_T> >
    {
        static bool item_redefined(set<string>& ss, const string& key) {
            if (ss.find(key) != ss.end()) return true;
            ss.insert(key);
            return false;
        }

        template <typename ScannerT> struct definition
        {
            rule<ScannerT> config_r, item_r;
            typename derived_T::keywords keywords_p;
            typename derived_T::constvals constvals_p;
            value_parser value_p;

            definition(base_config_parser const& self) : value_p(constvals_p) {
                assertion<string> item_redefined_e("item redefined");
                assertion<string> type_mismatch_e("type mismatch");
                assertion<string> out_of_range_e("out of range");
                using phoenix::arg1;
                using phoenix::construct_;
                using phoenix::var;
                using phoenix::bind;

                config_r
                    = !item_r >> *( ';' >> !item_r) >> end_p;
                item_r
                    = keywords_p[var(self.current_keywd)=arg1]
                    >> item_redefined_e(eps_p(bind(&item_redefined)(var(self.defined_symbols),var(self.current_keywd))==false))
                    >> '='
                    >> value_p[var(value_p.val)=bind(&normalize)(var(self.timap),var(self.current_keywd),var(value_p.val))]
                              [insert_at_a(self.vmap,self.current_keywd,value_p.val)]
                    >> type_mismatch_e(eps_p(bind(&type_mismatch)(var(self.timap),var(self.vmap),var(self.current_keywd))==false))
                    >> out_of_range_e(eps_p(bind(&out_of_range)(var(self.timap),var(self.vmap),var(self.current_keywd))==false));
            }

            rule<ScannerT> const& start() const { return config_r; }
        };

        mutable string current_keywd;
        mutable set<string> defined_symbols;
        mutable tyinfo_map_t timap;
        mutable value_map_t vmap;
        vector<string> reqs;

	void init() { vmap.clear(); defined_symbols.clear(); }
    };

}

#endif /* PARSER_HPP */
