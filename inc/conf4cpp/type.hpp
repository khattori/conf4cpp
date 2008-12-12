/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#ifndef TYPE_HPP
#define TYPE_HPP

#include <vector>
#include <map>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/spirit.hpp>
#include <boost/lexical_cast.hpp>
#include <conf4cpp/var.hpp>

using namespace std;
using namespace boost::spirit;

namespace conf4cpp
{
    enum atomic_t {
        TI_BOOL,
        TI_INT,
        TI_UINT,
        TI_DOUBLE,
        TI_STRING,
        TI_TIME,
        TI_IPV4ADDR,
        TI_IPV6ADDR,
    };
    struct ti_atomic_t {
        ti_atomic_t() : t(TI_BOOL) {}
        ti_atomic_t(atomic_t t_) : t(t_) {}
        ti_atomic_t(atomic_t t_, pair<var_t,var_t> c_) : t(t_), c(c_) {}
        operator atomic_t() const { return t; }

        atomic_t t;
        boost::optional<pair<var_t,var_t> >c;
    };
    struct ti_enum_t {
	ti_enum_t(const string& eid_) : eid(eid_) {}
	string eid;
    };
    typedef boost::make_recursive_variant<
	ti_atomic_t,
	ti_enum_t,                              // enum type
	pair<int, boost::recursive_variant_>,   // vector type 
	vector<boost::recursive_variant_>       // tuple type
    >::type type_t;
    const int VAR_VECTOR = -1;
    enum { IS_ATOM_T, IS_ENUM_T, IS_VECT_T, IS_TUPLE_T };

    inline bool is_atomic_type(const type_t& typ) { return typ.which() == IS_ATOM_T; }
    inline bool is_enum_type(const type_t& typ)   { return typ.which() == IS_ENUM_T; }
    inline bool is_vector_type(const type_t& typ) { return typ.which() == IS_VECT_T; }
    inline bool is_tuple_type(const type_t& typ)  { return typ.which() == IS_TUPLE_T; }

    inline bool is_prim_type(const type_t& typ) { return typ.which() < 2; }
    inline bool is_comp_type(const type_t& typ) { return !is_prim_type(typ); }
    inline ti_atomic_t get_atom(const type_t& typ) { return boost::get<ti_atomic_t>(typ); }
    class type_checker : public boost::static_visitor<bool>
    {
    public:
	type_checker(const var_t& v) : v_(v) {}
	bool operator() (ti_atomic_t ta) const {
	    switch (ta) {
	    case TI_BOOL:     if (is_bool(v_))     return true; break;
	    case TI_INT:      if (is_int(v_)||(is_uint(v_) && boost::get<unsigned int>(v_) <= INT_MAX))
                                                   return true; break;
	    case TI_UINT:     if (is_uint(v_))     return true; break;
	    case TI_DOUBLE:   if (is_double(v_))   return true; break;
	    case TI_STRING:   if (is_string(v_))   return true; break;
            case TI_TIME:     if (is_time(v_))     return true; break; 
            case TI_IPV4ADDR: if (is_ipv4addr(v_)) return true; break;
            case TI_IPV6ADDR: if (is_ipv6addr(v_)) return true; break;
	    default: assert(false);
	    }
	    return false;
	}
	bool operator() (ti_enum_t te) const {
	    if (!is_pair(v_)) return false;
	    pair<string,int> p = boost::get<pair<string,int> >(v_);
	    if (p.first != te.eid) return false;
	    return true;
	}
	bool operator() (pair<unsigned int, type_t> tp) const {
	    if (!is_vector(v_)) return false;
	    vector<var_t> vec = boost::get<vector<var_t> >(v_);
	    if (tp.first > 0 && tp.first != vec.size()) return false;
	    for (unsigned int i = 0; i < vec.size(); i++) {
		if (!apply_visitor(type_checker(vec[i]), tp.second)) return false;
	    }
	    return true;
	}
	bool operator() (vector<type_t> tv) const {
	    if (!is_vector(v_)) return false;
	    vector<var_t> vec = boost::get<vector<var_t> >(v_);
	    if (vec.size() != tv.size()) return false;
	    for (unsigned int i = 0; i < tv.size(); i++) {
		if (!apply_visitor(type_checker(vec[i]), tv[i])) return false;
	    }
	    return true;
	}
    private:
	var_t v_;
    };
    class range_checker : public boost::static_visitor<bool>
    {
    public:
	range_checker(const var_t& v) : v_(v) {}
	bool operator() (ti_atomic_t ta) const {
            if (!ta.c) return true;
	    switch (ta) {
	    case TI_BOOL:     return true;
	    case TI_INT:      return (is_bool(ta.c->first) ||
                                      (is_uint(ta.c->first) && is_uint(v_) && boost::get<unsigned int>(ta.c->first) <= boost::get<unsigned int>(v_)) ||
                                      (is_int(v_) && boost::get<int>(ta.c->first) <= boost::get<int>(v_)) ||
                                      (is_uint(v_) && boost::get<int>(ta.c->first)))
                                  && (is_bool(ta.c->second) ||
                                      (is_int(ta.c->second) && is_int(v_) && boost::get<int>(ta.c->second) >= boost::get<int>(v_)) ||
                                      (is_uint(v_) && boost::get<unsigned int>(ta.c->second) >= boost::get<unsigned int>(v_)) ||
                                      (is_int(v_) && boost::get<unsigned int>(ta.c->second)));
	    case TI_UINT:     return (is_bool(ta.c->first)  || boost::get<unsigned int>(ta.c->first)  <= boost::get<unsigned int>(v_))
                                  && (is_bool(ta.c->second) || boost::get<unsigned int>(ta.c->second) >= boost::get<unsigned int>(v_));
	    case TI_DOUBLE:   return (is_bool(ta.c->first)  || boost::get<double>(ta.c->first)  <= boost::get<double>(v_))
                                  && (is_bool(ta.c->second) || boost::get<double>(ta.c->second) >= boost::get<double>(v_));
	    case TI_STRING:   return true;
            case TI_TIME:     return true;
            case TI_IPV4ADDR: return true;
            case TI_IPV6ADDR: return true;
	    default: assert(false);
	    }
	    return false;
	}
	bool operator() (ti_enum_t te) const { return true; }
	bool operator() (pair<unsigned int, type_t> tp) const {
	    vector<var_t> vec = boost::get<vector<var_t> >(v_);
	    if (tp.first > 0 && tp.first != vec.size()) return false;
	    for (unsigned int i = 0; i < vec.size(); i++) {
		if (!apply_visitor(range_checker(vec[i]), tp.second)) return false;
	    }
	    return true;
	}
	bool operator() (vector<type_t> tv) const {
	    vector<var_t> vec = boost::get<vector<var_t> >(v_);
	    for (unsigned int i = 0; i < tv.size(); i++) {
		if (!apply_visitor(range_checker(vec[i]), tv[i])) return false;
	    }
	    return true;
	}
    private:
	var_t v_;
    };
    inline bool range_check(const type_t& typ, const var_t& var) {
        return apply_visitor(range_checker(var), typ);
    }
}

#endif /* TYPE_HPP */
