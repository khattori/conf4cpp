/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#ifndef VAR_HPP
#define VAR_HPP

#include <time.h>
#include <netinet/in.h>
#include <string>
#include <vector>
#include <map>
#include <boost/variant.hpp>

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
}

#endif /* VAR_HPP */
