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
	    p = new derived_config_parser_T();
	    parse_config(fname);
	}
	virtual ~base_config();

    protected:
        derived_config_parser_T* p;

    private:
	void parse_config(const string& fname) {
	    typedef file_iterator<char> file_iterator_t;

	    file_iterator_t first(fname);
	    if (!first) throw error("could not open file");
	    file_iterator_t last = first.make_end();

	    typedef position_iterator<file_iterator_t> position_iterator_t;
	    position_iterator_t begin(first, last, fname);
	    position_iterator_t end;
	    begin.set_tabchars(8);
	    try {
		parse_info<position_iterator_t> pinfo = parse(begin, end, *p, eol_p|space_p|comment_p("#"));
		if (!pinfo.full) throw error("parse error", pinfo.stop.get_position().line);
	    } catch (boost::spirit::parser_error<string,position_iterator_t>& e) {
		throw error(e.descriptor, e.where.get_position().line);
	    }
	    for (unsigned int i = 0; i < p->reqs.size(); i++) {
		if (p->vmap.find(p->reqs[i]) == p->vmap.end()) throw error("item undefined", p->reqs[i]);
	    }
	}
    };
}

#endif /* CONFIG_HPP */
