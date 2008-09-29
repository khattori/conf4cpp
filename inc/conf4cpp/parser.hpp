/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#ifndef PARSER_HPP
#define PARSER_HPP

#include <boost/spirit.hpp>
#include <boost/spirit/phoenix.hpp>
#include <conf4cpp/var.hpp>

using namespace std;
using namespace boost::spirit;

namespace conf4cpp
{

    typedef map<int, type_t> tyinfo_map_t;
    typedef map<int, var_t> value_map_t;

    struct tychk_visitor : public boost::static_visitor<error_t>
    {
	var_t v_;
	tychk_visitor(const var_t& v) : v_(v) {}
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
	error_t operator() (pair<int, type_t> tp) const {
	    if (!is_vector(v_)) return type_mismatch;
	    vector<var_t> vec = var2vector(v_);
	    cout << "operator(pair<int, type_t>)" << vec.size() << "," << tp.first << endl;
	    if (tp.first > 0 && tp.first != vec.size()) return invalid_length;
	    for (unsigned int i = 0; i < vec.size(); i++) {
		error_t err;
		if ((err = apply_visitor(tychk_visitor(vec[i]), tp.second)) != error_none)
		    return err;
	    }
	    return error_none;
	}
	error_t operator() (vector<type_t> tv) const {
	    if (!is_vector(v_)) return type_mismatch;
	    vector<var_t> vec = var2vector(v_);
	    cout << "operator(vector<type_t>)" << vec.size() << "," << tv.size() << endl;
	    if (vec.size() != tv.size()) return type_mismatch;
	    for (unsigned int i = 0; i < tv.size(); i++) {
		error_t err;
		if ((err = apply_visitor(tychk_visitor(vec[i]), tv[i])) != error_none)
		    return err;
	    }
	    return error_none;
	}
    };
 
    struct store_value {
	typedef error_t result_type;
	store_value(tyinfo_map_t tm, value_map_t& vm) : tm_(tm), vm_(vm) {}
	error_t operator()(int n, const var_t& v) {
	    vector<var_t> vec = var2vector(v);	    
	    type_t typ = tm_[n];
	    error_t err;
	    if (is_atomic_type(typ)) {
		if (vec.size() != 1) return type_mismatch;
		if ((err = apply_visitor(tychk_visitor(vec[0]), typ)) != error_none)
		    return err;
		vm_[n] = vec[0];
	    } else {
		if (vec.size() == 1 && is_vector(vec[0])) {
		    if ((err = apply_visitor(tychk_visitor(vec[0]), typ)) != error_none)
			return err;
		    vm_[n] = vec[0];
		} else {
		    if ((err = apply_visitor(tychk_visitor(v), typ)) != error_none)
			return err;
		    vm_[n] = v;
		}
	    }
	    return error_none;
	}
	value_map_t &vm_;
	tyinfo_map_t tm_;
    };
    struct check_value {
	typedef void result_type;
	template<typename IteratorT>
	void operator()(error_t err, const IteratorT& where) {
	    if (err != error_none) throw_(where, err);
	}
    };
    var_t add_value(var_t& v1, const var_t& v2) { vector<var_t> v = var2vector(v1); v.push_back(v2); return v; }

    struct variant_val : closure<variant_val, var_t>  { member1 val; };
    struct string_val  : closure<string_val,  string> { member1 val; };
    struct item_val    : closure<item_val, int, error_t> { member1 kwid; member2 err;};

    template <typename derived_T>
    struct base_config_parser : public grammar<base_config_parser<derived_T> >
    {
	base_config_parser(value_map_t& vm) : vmap(vm) {}

	tyinfo_map_t timap;
	value_map_t& vmap;

	template <typename ScannerT> struct definition
	{
	    rule<ScannerT> config_r;
	    rule<ScannerT, variant_val::context_t> value_r, atomic_value_r, bool_r;
	    rule<ScannerT, string_val::context_t> string_r;
	    rule<ScannerT, item_val::context_t> item_r;

	    typename derived_T::keywords keywords_p;
	    typename derived_T::constvals constvals_p;

	    definition(base_config_parser const& self)
		{
		    using phoenix::arg1;
		    using phoenix::arg2;
		    using phoenix::construct_;
		    config_r
			= *item_r;
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
			strict_real_p	[atomic_value_r.val = arg1]
			| int_p		[atomic_value_r.val = arg1]
			| bool_r	[atomic_value_r.val = arg1]
			| string_r	[atomic_value_r.val = arg1]
			| ch_p('{')[atomic_value_r.val = construct_<vector<var_t> >()]
			    >> !value_r[atomic_value_r.val = arg1] >> '}'
			//	| '{' >> *item_r >> '}'
			| constvals_p	[atomic_value_r.val = arg1];

		    bool_r
			= str_p("true") [bool_r.val = true]
			| str_p("false")[bool_r.val = false];
		    string_r
			= confix_p('"', (*c_escape_ch_p)[string_r.val = construct_<string>(arg1,arg2)], '"');
		}

	    rule<ScannerT> const& start() const { return config_r; }
	};
    };

}

#endif /* PARSER_HPP */
