/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#ifndef VAR_HPP
#define VAR_HPP

#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <map>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/spirit.hpp>
#include <boost/spirit/phoenix.hpp>

using namespace std;
using boost::spirit::nil_t;

namespace conf4cpp
{
    typedef boost::make_recursive_variant<
        nil_t,
	bool,
	int,
        unsigned int,
	double,
	string,
        struct tm,
        struct in_addr,
        struct in6_addr,
	pair<string,int>,                 // enum type
	vector<boost::recursive_variant_> // list or vector type
    >::type var_t;

    enum type_id_t {
        IS_NIL,
	IS_BOOL,
	IS_INT,
        IS_UINT,
	IS_DOUBLE,
	IS_STRING,
        IS_TIME,
        IS_IPV4ADDR,
        IS_IPV6ADDR,
	IS_PAIR,
	IS_VECTOR,
    };

    //　バリアント型のトレイツ定義（型と型IDの対応を定義）
    template <typename T> struct vtype_traits { };
    template <> struct vtype_traits<nil_t>             { typedef nil_t            variant_type; static type_id_t type_id() { return IS_NIL;      } };
    template <> struct vtype_traits<bool>              { typedef bool             variant_type; static type_id_t type_id() { return IS_BOOL;     } };
    template <> struct vtype_traits<int>               { typedef int              variant_type; static type_id_t type_id() { return IS_INT;      } };
    template <> struct vtype_traits<unsigned int>      { typedef unsigned int     variant_type; static type_id_t type_id() { return IS_UINT;     } };
    template <> struct vtype_traits<double>            { typedef double           variant_type; static type_id_t type_id() { return IS_DOUBLE;   } };
    template <> struct vtype_traits<string>            { typedef string           variant_type; static type_id_t type_id() { return IS_STRING;   } };
    template <> struct vtype_traits<struct tm>         { typedef struct tm        variant_type; static type_id_t type_id() { return IS_TIME;     } };
    template <> struct vtype_traits<struct in_addr>    { typedef struct in_addr   variant_type; static type_id_t type_id() { return IS_IPV4ADDR; } };
    template <> struct vtype_traits<struct in6_addr>   { typedef struct in6_addr  variant_type; static type_id_t type_id() { return IS_IPV6ADDR; } };
    template <> struct vtype_traits<pair<string,int> > { typedef pair<string,int> variant_type; static type_id_t type_id() { return IS_PAIR;     } };
    template <> struct vtype_traits<vector<var_t> >    { typedef vector<var_t>    variant_type; static type_id_t type_id() { return IS_VECTOR;   } };

    // バリアント型の判定関数定義
    template <typename T> bool is_(const var_t& v) { return v.which() == vtype_traits<T>::type_id(); }
    inline bool is_nil      (const var_t& v) { return is_<nil_t>(v);             }
    inline bool is_bool     (const var_t& v) { return is_<bool>(v);              }
    inline bool is_int      (const var_t& v) { return is_<int>(v);               }
    inline bool is_uint     (const var_t& v) { return is_<unsigned int>(v);      }
    inline bool is_double   (const var_t& v) { return is_<double>(v);            }
    inline bool is_string   (const var_t& v) { return is_<string>(v);            }
    inline bool is_time     (const var_t& v) { return is_<struct tm>(v);         }
    inline bool is_ipv4addr (const var_t& v) { return is_<struct in_addr>(v);    }
    inline bool is_ipv6addr (const var_t& v) { return is_<struct in6_addr>(v);   }
    inline bool is_pair     (const var_t& v) { return is_<pair<string,int> >(v); }
    inline bool is_vector   (const var_t& v) { return is_<vector<var_t> >(v);    }

    inline string to_str(const var_t& v) {
        switch(v.which()) {
	case IS_BOOL: return boost::get<bool>(v) ? "true" : "false";
	case IS_INT: return "(int)" + boost::lexical_cast<string>(boost::get<int>(v));
        case IS_UINT: return "(unsigned int)" + boost::lexical_cast<string>(boost::get<unsigned int>(v));
	case IS_DOUBLE: return "(double)" + boost::lexical_cast<string>(boost::get<double>(v));
	case IS_STRING: return "\"" + boost::get<string>(v) +  "\"";
        case IS_TIME:
        case IS_IPV4ADDR:
        case IS_IPV6ADDR:
	case IS_PAIR:
	case IS_VECTOR:
        case IS_NIL:
	assert(false);
        }
	return "???";
    }

    inline struct in_addr to_in_addr(in_addr_t n) { struct in_addr addr = {n}; return addr; }
    inline struct in6_addr to_in6_addr(uint8_t n1, uint8_t n2, uint8_t n3, uint8_t n4,
                                      uint8_t n5, uint8_t n6, uint8_t n7, uint8_t n8,
                                      uint8_t n9, uint8_t n10,uint8_t n11,uint8_t n12,
	                              uint8_t n13,uint8_t n14,uint8_t n15,uint8_t n16) {
	struct in6_addr addr = {{{n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16}}};
	return addr;
    }

    using namespace boost::spirit;
    using phoenix::arg1;
    using phoenix::arg2;
    using phoenix::bind;
    using phoenix::var;
    using phoenix::construct_;
    //
    // 時刻パーサ定義
    //
    template<typename IteratorT>
    inline struct tm str2time(const char *fmt, const IteratorT& first, const IteratorT& last) {
        string s(first, last);
        struct tm ret = {0};
        char *p = strptime(s.c_str(), fmt, &ret);
        if (p == NULL || *p != '\0') throw_(first, string("invalid time format"));
        return ret;
    }
    struct datetime_parser : public grammar<datetime_parser>
    {
        template <typename ScannerT>
        struct definition
        {
            typedef rule<ScannerT> rule_t;
            rule_t top, date_r, time_r;
            definition(datetime_parser const& self) {
                top = (date_r >> time_r)[var(self.val)=bind(&str2time<typename ScannerT::iterator_t>)("%Y/%m/%d %T",arg1,arg2)]
                    | date_r[var(self.val)=bind(&str2time<typename ScannerT::iterator_t>)("%Y/%m/%d",arg1,arg2)];
                date_r
                    = lexeme_d[repeat_p(1,4)[digit_p] >> "/" >> repeat_p(1,2)[digit_p] >> "/" >> repeat_p(1,2)[digit_p]];
                time_r
                    = lexeme_d[repeat_p(1,2)[digit_p] >> ":" >> repeat_p(1,2)[digit_p] >> ":" >> repeat_p(1,2)[digit_p]];
            }
            rule_t const& start() const { return top; }
        };
        mutable struct tm val;
    };

    //
    // IPアドレスパーサ定義
    //
    template<typename InAddrT, int af, typename IteratorT>
    inline InAddrT str2addr(const IteratorT& first, const IteratorT& last) {
        string s(first, last);
        InAddrT ret;
        if (inet_pton(af, s.c_str(), &ret) != 1) throw_(first, string("invalid IP address format"));
        return ret;
    }

    struct ipv4addr_parser : public grammar<ipv4addr_parser>
    {
        template <typename ScannerT>
        struct definition
        {
            typedef rule<ScannerT> rule_t;
            rule_t top;
            definition(ipv4addr_parser const& self) {
                top = lexeme_d[(repeat_p(3)[repeat_p(1,3)[digit_p] >> "."] >> repeat_p(1,3)[digit_p])]
                    [var(self.val)=bind(&str2addr<struct in_addr, AF_INET, typename ScannerT::iterator_t>)(arg1,arg2)];
            }
            rule_t const& start() const { return top; }
        };
        mutable struct in_addr val;
    };

    struct ipv6addr_parser : public grammar<ipv6addr_parser>
    {
        template <typename ScannerT>
        struct definition
        {
            typedef rule<ScannerT> rule_t;
            rule_t top;
            definition(ipv6addr_parser const& self) {
                top = ( lexeme_d[repeat_p(7)[repeat_p(1,4)[xdigit_p] >> ":"] >> repeat_p(1,4)[xdigit_p]]
                        | lexeme_d[(repeat_p(1,7)[repeat_p(1,4)[xdigit_p] >> ":"] | ":") >> (repeat_p(1,7)[":" >> repeat_p(1,4)[xdigit_p]] | ":")])
                    [var(self.val)=bind(&str2addr<struct in6_addr, AF_INET6, typename ScannerT::iterator_t>)(arg1,arg2)];
            }
            rule_t const& start() const { return top; }
        };
        mutable struct in6_addr val;
    };

    typedef symbols<pair<string,int> > enum_sym_t;
    struct variant_val : closure<variant_val, var_t>  { member1 val; };
    struct value_parser : public grammar<value_parser>
    {
        static var_t add_value(var_t& v1, const var_t& v2) {
            vector<var_t> v = boost::get<vector<var_t> >(v1);
            v.push_back(v2);
            return v;
        }
        value_parser(const enum_sym_t& constvals) : constvals_p(constvals) {}
        value_parser() {}
        template <typename ScannerT>
        struct definition
        {
            typedef rule<ScannerT> rule_t;
            rule_t top;
            rule<ScannerT, variant_val::context_t> values_r, atomic_value_r;
            datetime_parser datetime_r;
            ipv4addr_parser ipv4addr_r;
            ipv6addr_parser ipv6addr_r;

            definition(value_parser const& self) {
                top
                    = values_r[var(self.val)=arg1];
                values_r
                    = atomic_value_r[values_r.val=construct_<vector<var_t> >(1,arg1)]
                    >> *(',' >> atomic_value_r[values_r.val=bind(&add_value)(values_r.val,arg1)]);
                atomic_value_r
                    = lexeme_d[((str_p("0x")|"0X") >> hex_p[atomic_value_r.val=arg1])]
                    | datetime_r[atomic_value_r.val=var(datetime_r.val)]
                    | ipv4addr_r[atomic_value_r.val=var(ipv4addr_r.val)]
                    | ipv6addr_r[atomic_value_r.val=var(ipv6addr_r.val)]
                    | strict_real_p[atomic_value_r.val=arg1]
                    | uint_p[atomic_value_r.val=arg1]
                    | int_p[atomic_value_r.val=arg1]
                    | str_p("true")[atomic_value_r.val=true] | str_p("false")[atomic_value_r.val=false]
                    | confix_p('"', (*c_escape_ch_p)[atomic_value_r.val=construct_<string>(arg1,arg2)], '"')
                    | ch_p('{')[atomic_value_r.val=construct_<vector<var_t> >()] >> !values_r[atomic_value_r.val=arg1] >> '}'
                    | self.constvals_p[atomic_value_r.val=arg1];
            }
            rule_t const& start() const { return top; }
        };
        mutable var_t val;
        mutable enum_sym_t constvals_p;
    };
}

#endif /* VAR_HPP */
