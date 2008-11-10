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
#include <boost/spirit.hpp>
#include <boost/spirit/phoenix.hpp>
#include <conf4cpp/error.hpp>

using namespace std;

namespace conf4cpp
{
    typedef boost::make_recursive_variant<
	bool,
	int,
        unsigned int,
	double,
	string,
        struct tm,
        struct in_addr,
        struct in6_addr,
	pair<string,int>,
	vector<boost::recursive_variant_>
    >::type var_t;

    enum {
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

    inline bool is_bool     (const var_t& v) { return v.which() == IS_BOOL;     }
    inline bool is_int      (const var_t& v) { return v.which() == IS_INT;      }
    inline bool is_uint     (const var_t& v) { return v.which() == IS_UINT;     }
    inline bool is_double   (const var_t& v) { return v.which() == IS_DOUBLE;   }
    inline bool is_string   (const var_t& v) { return v.which() == IS_STRING;   }
    inline bool is_time     (const var_t& v) { return v.which() == IS_TIME;     }
    inline bool is_ipv4addr (const var_t& v) { return v.which() == IS_IPV4ADDR; }
    inline bool is_ipv6addr (const var_t& v) { return v.which() == IS_IPV6ADDR; }
    inline bool is_pair     (const var_t& v) { return v.which() == IS_PAIR;     }
    inline bool is_vector   (const var_t& v) { return v.which() == IS_VECTOR;   }

    template<typename T> const T& var2(const var_t &v) { return boost::get<T>(v); }

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

    //
    // IPアドレスパーサ定義
    //
    template<typename IteratorT>
    inline struct tm str2time(const char *fmt, const IteratorT& first, const IteratorT& last) {
        string s(first, last);
        struct tm ret = {0};
        char *p = strptime(s.c_str(), fmt, &ret);
        if (p == NULL || *p != '\0') throw_(first, invalid_time);
        return ret;
    }

    //
    // IPアドレスパーサ定義
    //
    template<typename InAddrT, int af, typename IteratorT>
    inline InAddrT str2addr(const IteratorT& first, const IteratorT& last) {
        string s(first, last);
        InAddrT ret;
        if (inet_pton(af, s.c_str(), &ret) != 1) throw_(first, invalid_addr);
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
}

#endif /* VAR_HPP */
