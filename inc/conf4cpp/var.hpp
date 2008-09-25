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
	vector<boost::recursive_variant_>
    >::type var_t;

    inline int           var2int   (const var_t &v) { return boost::get<int>(v);            }
    inline double        var2double(const var_t &v) { return boost::get<double>(v);         }
    inline string        var2string(const var_t &v) { return boost::get<string>(v);         }
    inline bool          var2bool  (const var_t &v) { return boost::get<bool>(v);           }
    inline vector<var_t> var2vector(const var_t &v) { return boost::get<vector<var_t> >(v); }

    enum {
	IS_BOOL,
	IS_INT,
	IS_DOUBLE,
	IS_STRING,
	IS_VECTOR,
    };

    inline bool is_bool   (const var_t& v) { return v.which() == IS_BOOL;   }
    inline bool is_int    (const var_t& v) { return v.which() == IS_INT;    }
    inline bool is_double (const var_t& v) { return v.which() == IS_DOUBLE; }
    inline bool is_string (const var_t& v) { return v.which() == IS_STRING; }
    inline bool is_vector (const var_t& v) { return v.which() == IS_VECTOR; }
}

#endif /* VAR_HPP */
