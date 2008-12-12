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
    typedef map<string, type_t> tyinfo_map_t;
    typedef map<string, var_t> value_map_t;

    static bool type_mismatch(tyinfo_map_t& tm, value_map_t& vm, const string& key) {
        vector<var_t> vec = boost::get<vector<var_t> >(vm[key]);
        type_t typ = tm[key];

        if (is_prim_type(typ)) {
            if (vec.size() != 1) return true;
            if (!apply_visitor(type_checker(vec[0]), typ)) return true;
            vm[key] = vec[0];
        } else {
            if (vec.size() == 1 && is_vector(vec[0])) {
                if (!apply_visitor(type_checker(vec[0]), typ)) return true;
                vm[key] = vec[0];
            } else {
                if (!apply_visitor(type_checker(vm[key]), typ)) return true;
            }
        }
        return false;
    }

    static bool out_of_range(tyinfo_map_t& tm, value_map_t& vm, const string& key) {
        return !apply_visitor(range_checker(vm[key]), tm[key]);
    }

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
                    >> value_p[insert_at_a(self.vmap,self.current_keywd,value_p.val)]
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
