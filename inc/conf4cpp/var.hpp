/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#ifndef VAR_HPP
#define VAR_HPP
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
	double,
	string,
	pair<int,int>,
	vector<boost::recursive_variant_>
    >::type var_t;

    enum {
	IS_BOOL,
	IS_INT,
	IS_DOUBLE,
	IS_STRING,
	IS_PAIR,
	IS_VECTOR,
    };

    inline bool is_bool   (const var_t& v) { return v.which() == IS_BOOL;   }
    inline bool is_int    (const var_t& v) { return v.which() == IS_INT;    }
    inline bool is_double (const var_t& v) { return v.which() == IS_DOUBLE; }
    inline bool is_string (const var_t& v) { return v.which() == IS_STRING; }
    inline bool is_pair   (const var_t& v) { return v.which() == IS_PAIR;   }
    inline bool is_vector (const var_t& v) { return v.which() == IS_VECTOR; }

    template<typename T> const T& var2(const var_t &v);
    template<> inline const bool&          var2(const var_t &v) { assert(is_bool(v));   return boost::get<bool>(v);           }
    template<> inline const int&           var2(const var_t &v) { assert(is_int(v));    return boost::get<int>(v);            }
    template<> inline const double&        var2(const var_t &v) { assert(is_double(v)); return boost::get<double>(v);         }
    template<> inline const string&        var2(const var_t &v) { assert(is_string(v)); return boost::get<string>(v);         }
    template<> inline const pair<int,int>& var2(const var_t &v) { assert(is_pair(v));   return boost::get<pair<int,int> >(v); }
    template<> inline const vector<var_t>& var2(const var_t &v) { assert(is_vector(v)); return boost::get<vector<var_t> >(v); }
}

#endif /* VAR_HPP */
