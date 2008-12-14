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
    enum { IS_ATOM_T, IS_ENUM_T, IS_VECT_T, IS_TUPLE_T };
    const int VAR_VECTOR = -1;
    typedef pair<int,type_t> ti_vect_t;
    typedef vector<type_t> ti_tuple_t;

    inline bool is_atomic_type(const type_t& typ) { return typ.which() == IS_ATOM_T;  }
    inline bool is_enum_type(const type_t& typ)   { return typ.which() == IS_ENUM_T;  }
    inline bool is_vector_type(const type_t& typ) { return typ.which() == IS_VECT_T;  }
    inline bool is_tuple_type(const type_t& typ)  { return typ.which() == IS_TUPLE_T; }

    inline bool is_prim_type(const type_t& typ) { return typ.which() < 2; }
    inline bool is_comp_type(const type_t& typ) { return !is_prim_type(typ); }

    inline ti_atomic_t get_atom(const type_t& typ) { return boost::get<ti_atomic_t>(typ); }

    //
    // 必要に応じて，uint型の値をint型に正規化する
    //
    class var_normalizer : public boost::static_visitor<var_t>
    {
    public:
	var_normalizer(const var_t& v) : v_(v) {}
	var_t operator() (ti_atomic_t ta) const {
	    if (ta == TI_INT && is_uint(v_) && boost::get<unsigned int>(v_) <= INT_MAX) {
                return int(boost::get<unsigned int>(v_));
            }
            return v_;
	}
	var_t operator() (ti_enum_t te) const { return v_; }
	var_t operator() (ti_vect_t tp) const {
	    if (!is_vector(v_)) return v_;
	    vector<var_t> vec = boost::get<vector<var_t> >(v_);
            vector<var_t> result;
	    for (unsigned int i = 0; i < vec.size(); i++) {
		result.push_back(apply_visitor(var_normalizer(vec[i]), tp.second));
	    }
	    return result;
	}
	var_t operator() (ti_tuple_t tv) const {
	    if (!is_vector(v_)) return v_;
	    vector<var_t> vec = boost::get<vector<var_t> >(v_);
	    if (vec.size() != tv.size()) return v_;
            vector<var_t> result;
	    for (unsigned int i = 0; i < tv.size(); i++) {
		result.push_back(apply_visitor(var_normalizer(vec[i]), tv[i]));
	    }
	    return result;
	}
    private:
	var_t v_;
    };

    class type_checker : public boost::static_visitor<bool>
    {
    public:
	type_checker(const var_t& v) : v_(v) {}
	bool operator() (ti_atomic_t ta) const {
	    switch (ta) {
	    case TI_BOOL:     if (is_bool(v_))     return true; break;
	    case TI_INT:      if (is_int(v_))      return true; break;
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
	bool operator() (ti_vect_t tp) const {
	    if (!is_vector(v_)) return false;
	    vector<var_t> vec = boost::get<vector<var_t> >(v_);
	    if (tp.first >= 0 && tp.first != static_cast<int>(vec.size())) return false;
	    for (unsigned int i = 0; i < vec.size(); i++) {
		if (!apply_visitor(type_checker(vec[i]), tp.second)) return false;
	    }
	    return true;
	}
	bool operator() (ti_tuple_t tv) const {
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
	    case TI_INT:
                assert(is_int(v_));
                return (is_bool(ta.c->first)  || boost::get<int>(ta.c->first)  <= boost::get<int>(v_))
                    && (is_bool(ta.c->second) || boost::get<int>(ta.c->second) >= boost::get<int>(v_));
	    case TI_UINT:
                assert(is_uint(v_));
                return (is_bool(ta.c->first)  || boost::get<unsigned int>(ta.c->first)  <= boost::get<unsigned int>(v_))
                    && (is_bool(ta.c->second) || boost::get<unsigned int>(ta.c->second) >= boost::get<unsigned int>(v_));
	    case TI_DOUBLE:
                assert(is_double(v_));
                return (is_bool(ta.c->first)  || boost::get<double>(ta.c->first)  <= boost::get<double>(v_))
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
	bool operator() (ti_vect_t tp) const {
	    vector<var_t> vec = boost::get<vector<var_t> >(v_);
	    if (tp.first >= 0 && tp.first != static_cast<int>(vec.size())) return false;
	    for (unsigned int i = 0; i < vec.size(); i++) {
		if (!apply_visitor(range_checker(vec[i]), tp.second)) return false;
	    }
	    return true;
	}
	bool operator() (ti_tuple_t tv) const {
	    vector<var_t> vec = boost::get<vector<var_t> >(v_);
	    for (unsigned int i = 0; i < tv.size(); i++) {
		if (!apply_visitor(range_checker(vec[i]), tv[i])) return false;
	    }
	    return true;
	}
    private:
	var_t v_;
    };

    typedef map<string, type_t> tyinfo_map_t;
    typedef map<string, var_t> value_map_t;

    static var_t normalize(tyinfo_map_t& tm, const string& key, const var_t& v) {
        vector<var_t> vec = boost::get<vector<var_t> >(v);
        type_t typ = tm[key];
        // {{a,b,c}}や{{a}}, {{}}のケースか，primitive型で{a}などのケース
        if (vec.size() == 1 && (is_prim_type(typ) || is_vector(vec[0]))) {
            return apply_visitor(var_normalizer(vec[0]), typ);
        }
        // {a}, {a,b,c}, {{a},{b}}のケース
        return apply_visitor(var_normalizer(v), typ);
    }

    static bool type_mismatch(tyinfo_map_t& tm, value_map_t& vm, const string& key) {
        return !apply_visitor(type_checker(vm[key]), tm[key]);
    }

    static bool out_of_range(tyinfo_map_t& tm, value_map_t& vm, const string& key) {
        return !apply_visitor(range_checker(vm[key]), tm[key]);
    }
}

#endif /* TYPE_HPP */
