/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#ifndef PARSER_HPP
#define PARSER_HPP

#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <boost/spirit.hpp>
#include <boost/spirit/phoenix.hpp>
#include <conf4cpp/var.hpp>

using namespace std;
using namespace boost::spirit;

namespace conf4cpp
{
    typedef map<string, type_t> tyinfo_map_t;
    typedef map<string, var_t> value_map_t;

    class store_value
    {
    public:
        typedef error_t result_type;
        store_value(const tyinfo_map_t& tm, value_map_t& vm) : tm_(tm), vm_(vm) {}
        error_t operator()(const string& kw, const var_t& v) {
            vector<var_t> vec = var2<vector<var_t> >(v);	    
            type_t typ = tm_[kw];
            error_t err;
            if (is_prim_type(typ)) {
                if (vec.size() != 1) return type_mismatch;
                if ((err = apply_visitor(type_checker(vec[0]), typ)) != error_none)
                    return err;
                if (vm_.find(kw) != vm_.end()) return item_redefined;
                vm_[kw] = vec[0];
            } else {
                if (vec.size() == 1 && is_vector(vec[0])) {
                    if ((err = apply_visitor(type_checker(vec[0]), typ)) != error_none)
                        return err;
                    if (vm_.find(kw) != vm_.end()) return item_redefined;
                    vm_[kw] = vec[0];
                } else {
                    if ((err = apply_visitor(type_checker(v), typ)) != error_none)
                        return err;
                    if (vm_.find(kw) != vm_.end()) return item_redefined;
                    vm_[kw] = v;
                }
            }
            return error_none;
        }
    private:
        tyinfo_map_t tm_;
        value_map_t&  vm_;
    };
    struct check_value
    {
	typedef void result_type;
	template<typename IteratorT>
	void operator()(error_t err, const IteratorT& where) {
	    if (err != error_none) throw_(where, err);
	}
    };

    struct variant_val : closure<variant_val, var_t>  { member1 val; };
    struct string_val  : closure<string_val,  string> { member1 val; };
    struct item_val    : closure<item_val, string, error_t> { member1 kwid; member2 err;};

    template <typename derived_T>
    struct base_config_parser : public grammar<base_config_parser<derived_T> >
    {
        base_config_parser(value_map_t& vm) : vmap(vm) {}
        static var_t add_value(var_t& v1, const var_t& v2) { vector<var_t> v = var2<vector<var_t> >(v1); v.push_back(v2); return v; }

        template<typename InAddrT, int af, typename IteratorT>
        static InAddrT str2addr(const IteratorT& first, const IteratorT& last) {
            string s(first, last);
            InAddrT ret;
            if (inet_pton(af, s.c_str(), &ret) != 1) throw_(first, invalid_addr);
            return ret;
        }
        template<typename IteratorT>
        static struct tm str2time(const char *fmt, const IteratorT& first, const IteratorT& last) {
            string s(first, last);
            struct tm ret;
            if (strptime(s.c_str(), fmt, &ret) == NULL) throw_(first, invalid_time);
            return ret;
        }

        vector<string> reqs;
        tyinfo_map_t timap;
        value_map_t& vmap;

        template <typename ScannerT> struct definition
        {
            rule<ScannerT> config_r;
            rule<ScannerT, variant_val::context_t> value_r, atomic_value_r, bool_r, datetime_r;
            rule<ScannerT, string_val::context_t> string_r;
            rule<ScannerT, item_val::context_t> item_r;
            rule<ScannerT> date_r, time_r, ipv4addr_r, ipv6addr_r;

            typename derived_T::keywords keywords_p;
            typename derived_T::constvals constvals_p;

            definition(base_config_parser const& self) {
                using phoenix::arg1;
                using phoenix::arg2;
                using phoenix::construct_;
                config_r
                    = *item_r >> end_p;
                item_r
                    = keywords_p[item_r.kwid = arg1]
                        >> '='
                        >> value_r[item_r.err = phoenix::bind(store_value(self.timap,self.vmap))(item_r.kwid,arg1)]
                        >> eps_p[phoenix::bind(check_value())(item_r.err,arg1)]
                        >> ';';
                value_r
                    = atomic_value_r[value_r.val = construct_<vector<var_t> >(1,arg1)]
                        >> *(',' >> atomic_value_r[value_r.val = phoenix::bind(&add_value)(value_r.val,arg1)]);
                atomic_value_r
                    = 
                    strict_real_p[atomic_value_r.val = arg1]
                    | lexeme_d[(str_p("0x")|"0X") >> hex_p[atomic_value_r.val = arg1]]
                    | uint_p	[atomic_value_r.val = arg1]
                    | int_p	[atomic_value_r.val = arg1]
                    | bool_r	[atomic_value_r.val = arg1]
                    | string_r	[atomic_value_r.val = arg1]
                    | datetime_r[atomic_value_r.val = arg1]
                    | ipv4addr_r[atomic_value_r.val = phoenix::bind(&str2addr<struct in_addr, AF_INET, typename ScannerT::iterator_t>)(arg1,arg2)]
                    | ipv6addr_r[atomic_value_r.val = phoenix::bind(&str2addr<struct in6_addr, AF_INET6, typename ScannerT::iterator_t>)(arg1,arg2)]
                    | ch_p('{')[atomic_value_r.val = construct_<vector<var_t> >()]
                        >> !value_r[atomic_value_r.val = arg1] >> '}'
                    //	| '{' >> *item_r >> '}'
                    | constvals_p	[atomic_value_r.val = arg1];

                bool_r
                    = str_p("true") [bool_r.val = true ]
                    | str_p("false")[bool_r.val = false];
                string_r
                    = confix_p('"', (*c_escape_ch_p)[string_r.val = construct_<string>(arg1,arg2)], '"');
                datetime_r
                    = (date_r >> time_r)[datetime_r.val = phoenix::bind(&str2time<typename ScannerT::iterator_t>)("%Y/%M/%D %T",arg1,arg2)]
                    | date_r[datetime_r.val = phoenix::bind(&str2time<typename ScannerT::iterator_t>)("%Y/%M/%D",arg1,arg2)];
                date_r
                    = lexeme_d[repeat_p(4)[digit_p] >> "/" >> repeat_p(2)[digit_p] >> "/" >> repeat_p(2)[digit_p]];
                time_r
                    = lexeme_d[repeat_p(2)[digit_p] >> ":" >> repeat_p(2)[digit_p] >> ":" >> repeat_p(2)[digit_p]];
                ipv4addr_r
                    = lexeme_d[repeat_p(3)[repeat_p(1,3)[digit_p] >> "."] >> repeat_p(1,3)[digit_p]];
                ipv6addr_r
                    = lexeme_d[repeat_p(7)[repeat_p(1,4)[xdigit_p] >> ":"] >> repeat_p(1,4)[xdigit_p]]
                    | lexeme_d[repeat_p(0,6)[repeat_p(1,4)[xdigit_p] >> ":"] >> repeat_p(1,4)[xdigit_p] >> "::" >> 
                               repeat_p(0,6)[repeat_p(1,4)[xdigit_p] >> ":"] >> repeat_p(1,4)[xdigit_p]];
            }

            rule<ScannerT> const& start() const { return config_r; }
        };
    };

}

#endif /* PARSER_HPP */
