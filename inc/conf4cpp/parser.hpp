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
    template <typename iterator_T>
    struct line_info
    {
	line_info(iterator_T iter) : litr_(iter), lnum_(1) {}
	iterator_T litr_;
	size_t lnum_;
    };

    template <typename iterator_T>
    struct new_line
    {
	new_line(line_info<iterator_T>& info) : linf_(info) {}
	void operator()(iterator_T first, iterator_T last) const {
	    linf_.litr_ = last;
	    linf_.lnum_++;
	}
	line_info<iterator_T>& linf_;
    }; 

    struct tychk_visitor : public boost::static_visitor<>
    {
	var_t v_;
	tychk_visitor(const var_t& v) : v_(v) {}
	void operator() (ti_atomic_t ta) const {
	    switch (ta) {
	    case TI_BOOL:   assert(is_bool(v_));   break;
	    case TI_INT:    assert(is_int(v_));    break;
	    case TI_DOUBLE: assert(is_double(v_)); break;
	    case TI_STRING: assert(is_string(v_)); break;
	    default: assert(false);
	    }
	}
	void operator() (pair<int, type_t> tp) const {
	    assert(is_vector(v_));
	    vector<var_t> vec = boost::get<vector<var_t> >(v_);
	    if (tp.first > 0) assert(tp.first == vec.size());
	    for (unsigned int i = 0; i < vec.size(); i++)
		apply_visitor(tychk_visitor(vec[i]), tp.second);
	}
	void operator() (vector<type_t> tv) const {
	    assert(is_vector(v_));
	    vector<var_t> vec = boost::get<vector<var_t> >(v_);
	    assert(vec.size() == tv.size());
	    for (unsigned int i = 0; i < tv.size(); i++)
		apply_visitor(tychk_visitor(vec[i]), tv[i]);
	}
    };
 
    typedef map<int, type_t> tyinfo_map_t;
    typedef map<int, var_t> value_map_t;
    struct type_check {
	typedef void result_type;
	type_check(tyinfo_map_t tm) : tm_(tm) {}
	void operator()(int n, const var_t& v) { apply_visitor(tychk_visitor(v), tm_[n]); }
	tyinfo_map_t tm_;
    };
    struct store_value {
	typedef void result_type;
	store_value(value_map_t& vm) : vm_(vm) {}
	void operator()(int n, const var_t& v) const { vm_[n] = v; }
	value_map_t &vm_;
    };
    var_t add_value(var_t& v1, const var_t& v2) {
	assert(is_vector(v1));
	vector<var_t> v = var2vector(v1); v.push_back(v2); return v;
    }

    struct variant_val : closure<variant_val, var_t>  { member1 val; };
    struct string_val  : closure<string_val,  string> { member1 val; };
    struct int_val     : closure<int_val, int>        { member1 val; };

    template <typename derived_T>
    struct base_config_parser : public grammar<base_config_parser<derived_T> >
    {
	base_config_parser(value_map_t& vmap_) : vmap(vmap_) {}
	tyinfo_map_t timap;
	value_map_t& vmap;
	template <typename ScannerT> struct definition
	{
	    rule<ScannerT> config_r;
	    rule<ScannerT, variant_val::context_t> value_r, atomic_value_r, bool_r;
	    rule<ScannerT, string_val::context_t> string_r;
	    rule<ScannerT, int_val::context_t> item_r;

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
			= keywords_p[item_r.val = arg1] >> '=' >>
			value_r
//			[type_check(self.timap,item_r.val,arg1)]
			[phoenix::bind(type_check(self.timap))(item_r.val,arg1)]
			[phoenix::bind(store_value(self.vmap))(item_r.val,arg1)]
			>> ';';
		    value_r
			= atomic_value_r[value_r.val = arg1] >> *(',' >> atomic_value_r[value_r.val = phoenix::bind(&add_value)(value_r.val,arg1)]);
		    atomic_value_r
			= 
			strict_real_p	[atomic_value_r.val = arg1]
			| int_p		[atomic_value_r.val = arg1]
			| bool_r	[atomic_value_r.val = arg1]
			| string_r	[atomic_value_r.val = arg1]
			| '{' >> !value_r[atomic_value_r.val = arg1] >> '}'
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
