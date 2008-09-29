/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <conf4cpp/error.hpp>
#include <conf4cpp/parser.hpp>

namespace conf4cpp
{
    template <class derived_config_parser_T>
    class base_config {
    public:
	base_config(const string& fname) {
	    derived_config_parser_T g(vm_);
	    parse_config(fname, g);
	}
    protected:
	value_map_t vm_;
    private:
	void parse_config(const string& fname, const derived_config_parser_T& g) {
	    typedef file_iterator<char> file_iterator_t;

	    file_iterator_t first(fname);
	    if (!first) throw file_error(fname, "cannot open file");
	    file_iterator_t last = first.make_end();

	    typedef position_iterator<file_iterator_t> position_iterator_t;
	    position_iterator_t begin(first, last, fname);
	    position_iterator_t end;
	    begin.set_tabchars(8);
	    try {
		parse_info<position_iterator_t> pinfo = parse(begin, end, g, eol_p|space_p|comment_p("#"));
		if (!pinfo.full) throw parse_error(pinfo.stop.get_position().line);
	    } catch(boost::spirit::parser_error<error_t,position_iterator_t>& e) {
		throw type_error(e.where.get_position().line);
	    }
	}

    };
}

#endif /* CONFIG_HPP */
