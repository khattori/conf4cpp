/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <conf4cpp/parser.hpp>

namespace conf4cpp
{
    template <class derived_config_parser_T>
    class base_config {
    public:
	base_config(const string& fname) {
	    typedef file_iterator<char> iterator_t;
	    iterator_t begin(fname);

	    if (!begin) throw file_error(fname, "can not open");
	    iterator_t first = begin;
	    iterator_t last = first.make_end();
	    line_info<iterator_t> linfo(begin);
	    new_line<iterator_t> newln(linfo);
	    derived_config_parser_T g(vm_);

	    parse_info<iterator_t> pinfo = parse(first, last, g, (eol_p[newln])|space_p|comment_p("#"));
	    if (!pinfo.full) parse_error(linfo.lnum_);
	}
    protected:
	value_map_t vm_;
    };
}

#endif /* CONFIG_HPP */
