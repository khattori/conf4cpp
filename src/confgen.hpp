/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#ifndef CONFGEN_HPP
#define CONFGEN_HPP

#include <time.h>
#include <netinet/in.h>

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
        : conf_name_(g.conf_name[0]),
          namespace_(++g.conf_name.begin(), g.conf_name.end()),
          itemtyp_map_(g.itemtyp_map),
          itemdef_map_(g.itemdef_map),
          itemreq_map_(g.itemreq_map),
          itemcon_map_(g.itemcon_map),
          enumelem_map_(g.enumelem_map) { }

    void output_interface(ostream& os);
    void output_implementation(ostream& os);
    static void output_interface_header(ostream& os, const string& incfile);
    static void output_interface_footer(ostream& os);
    static void output_implementation_header(ostream& os, const string& incfile);
    static void output_implementation_footer(ostream& os);

private:
    static void output_file_header(ostream& os);

    void output_namespace_begin(ostream& os);
    void output_namespace_end(ostream& os);
    void output_interface_enumdefs(ostream& os);
    void output_interface_accessors(ostream& os);
    void output_interface_initializers(ostream& os);
    void output_interface_setters(ostream& os);
    void output_interface_rngchks(ostream& os);
    void output_interface_members(ostream& os);
    void output_implementation_keywords(ostream& os);
    void output_implementation_constvals(ostream& os);
    void output_implementation_parser_constructor(ostream& os);
    void output_implementation_config_constructor(ostream& os);
    void output_implementation_config_accessors(ostream& os);
    void output_implementation_config_initializers(ostream& os);
    void output_implementation_config_setters(ostream& os);
    void output_implementation_config_rngchks(ostream& os);
    void output_implementation_config_enum2str(ostream& os);
    void output_implementation_config_dump(ostream& os);

    const string conf_name_;
    const vector<string> namespace_;
    const tyinfo_map_t itemtyp_map_;
    const value_map_t itemdef_map_;
    const map<string, bool> itemreq_map_, itemcon_map_;
    const enum_map_t enumelem_map_;
};

#endif /* CONFGEN_HPP */
