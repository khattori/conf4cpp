/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#ifndef TYPE_HPP
#define TYPE_HPP

#include <vector>
#include <map>
#include <boost/variant.hpp>

using namespace std;

namespace conf4cpp
{
    typedef enum {
	TI_BOOL,
	TI_INT,
	TI_DOUBLE,
	TI_STRING
    } ti_atomic_t;

    typedef boost::make_recursive_variant<
	ti_atomic_t,
	pair<int, boost::recursive_variant_>,   // vector type 
	vector<boost::recursive_variant_>       // tuple type
    >::type type_t;
}

#endif /* TYPE_HPP */
