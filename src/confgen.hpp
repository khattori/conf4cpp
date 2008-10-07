/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#ifndef CONFGEN_HPP
#define CONFGEN_HPP

#include <ostream>
#include <map>
#include <string>

#include <conf4cpp/type.hpp>

#include "confdef_g.hpp"

using namespace std;
using namespace conf4cpp;

class confgen
{
public:
    confgen(confdef_g& g)
        : conf_name_(g.conf_name),
          itemtype_map_(g.itemtype_map),
          itemreq_map_(g.itemreq_map),
          enumelem_map_(g.enumelem_map),
          enumid_map_(g.enumid_map) {}

    void output_interface(ostream& os);
    void output_implementation(ostream& os);

private:
    string get_typestr(const type_t& ty);
    string get_tsetstr(const type_t& ty, unsigned int lv);

    void output_file_header(ostream& os);
    void output_interface_enumdefs(ostream& os);
    void output_interface_accessors(ostream& os);
    void output_interface_members(ostream& os);
    void output_implementation_keywords(ostream& os);
    void output_implementation_constvals(ostream& os);
    void output_implementation_parser_constructor(ostream& os);
    void output_implementation_config_constructor(ostream& os);

    const string conf_name_;
    const map<string, type_t> itemtype_map_;
    const map<string, bool> itemreq_map_;
    const map<string, vector<string> > enumelem_map_;
    const map<string, int> enumid_map_;
};

#endif /* CONFGEN_HPP */
