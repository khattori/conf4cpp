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
#include <boost/spirit.hpp>
#include <boost/lexical_cast.hpp>
#include <conf4cpp/var.hpp>
#include <conf4cpp/error.hpp>

using namespace std;
using namespace boost::spirit;

namespace conf4cpp
{
    enum ti_atomic_t { TI_BOOL, TI_INT, TI_DOUBLE, TI_STRING };
    struct ti_enum_t {
	ti_enum_t(int eid_) : eid(eid_) {}
	int eid;
    };
    typedef boost::make_recursive_variant<
	ti_atomic_t,
	ti_enum_t,                              // enum type
	pair<int, boost::recursive_variant_>,   // vector type 
	vector<boost::recursive_variant_>       // tuple type
    >::type type_t;

    inline bool is_prim_type(const type_t& typ) { return typ.which() < 2; }
    inline bool is_comp_type(const type_t& typ) { return !is_prim_type(typ); }

    class type_string : public boost::static_visitor<string>
    {
    public:
        type_string(map<string,int> eidm) : eidm_(eidm) {}
        string operator() (ti_atomic_t ta) const {
            switch (ta) {
            case TI_BOOL:   return "bool"; 
            case TI_INT:    return "int";
            case TI_DOUBLE: return "double";
            case TI_STRING: return "string";
            default: assert(false);
            }
        }
        string operator() (ti_enum_t te) const {
            for (map<string,int>::const_iterator iter = eidm_.begin();
                 iter != eidm_.end();
                 ++iter) {
                if (iter->second == te.eid) return iter->first;
            }
            return "???: " + boost::lexical_cast<string>(te.eid);
        }
        string operator() (pair<unsigned int, type_t> tp) const {
            return string("vector<") + apply_visitor(type_string(eidm_),tp.second) + " >";
        }
        string operator() (vector<type_t> tv) const {
            string ret("tuple<");
            for (unsigned int i = 0; i < tv.size()-1; i++) {
                ret += apply_visitor(type_string(eidm_),tv[i]) + ",";
            }
            ret += apply_visitor(type_string(eidm_),tv.back()) + " >";
            return ret;
        }
    private:
        map<string,int> eidm_;
    };

    class type_checker : public boost::static_visitor<error_t>
    {
    public:
	type_checker(const var_t& v) : v_(v) {}
	error_t operator() (ti_atomic_t ta) const {
	    switch (ta) {
	    case TI_BOOL:   if (is_bool(v_))   return error_none; break;
	    case TI_INT:    if (is_int(v_))    return error_none; break;
	    case TI_DOUBLE: if (is_double(v_)) return error_none; break;
	    case TI_STRING: if (is_string(v_)) return error_none; break;
	    default: assert(false);
	    }
	    return type_mismatch;
	}
	error_t operator() (ti_enum_t te) const {
	    if (!is_pair(v_)) return type_mismatch;
	    pair<int,int> p = var2<pair<int,int> >(v_);
	    if (p.first != te.eid) return type_mismatch;
	    return error_none;
	}
	error_t operator() (pair<unsigned int, type_t> tp) const {
	    if (!is_vector(v_)) return type_mismatch;
	    vector<var_t> vec = var2<vector<var_t> >(v_);
	    if (tp.first > 0 && tp.first != vec.size()) return invalid_length;
	    for (unsigned int i = 0; i < vec.size(); i++) {
		error_t err;
		if ((err = apply_visitor(type_checker(vec[i]), tp.second)) != error_none)
		    return err;
	    }
	    return error_none;
	}
	error_t operator() (vector<type_t> tv) const {
	    if (!is_vector(v_)) return type_mismatch;
	    vector<var_t> vec = var2<vector<var_t> >(v_);
	    if (vec.size() != tv.size()) return type_mismatch;
	    for (unsigned int i = 0; i < tv.size(); i++) {
		error_t err;
		if ((err = apply_visitor(type_checker(vec[i]), tv[i])) != error_none)
		    return err;
	    }
	    return error_none;
	}
    private:
	var_t v_;
    };

}

#endif /* TYPE_HPP */
