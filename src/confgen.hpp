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

using namespace std;
using namespace conf4cpp;

class confgen
{
public:
    confgen(string& cn, map<string,type_t>& itypm, map<string,vector<string> >& eelmm, map<string,int> eidm)
        : conf_name_(cn), itemtype_map_(itypm), enumelem_map_(eelmm), enumid_map_(eidm) {}

    void output_interface(ostream& os);
    void output_implementation(ostream& os);

private:
    string get_typestr(const type_t& ty);

    void output_file_header(ostream& os);
    void output_interface_enumdefs(ostream& os);
    void output_interface_accessors(ostream& os);
    void output_interface_members(ostream& os);

    const string conf_name_;
    const map<string, type_t> itemtype_map_;
    const map<string, vector<string> > enumelem_map_;
    const map<string, int> enumid_map_;
};

#endif /* CONFGEN_HPP */
