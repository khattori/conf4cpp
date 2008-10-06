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

    void output(ostream& os);

private:
    string conf_name_;
    map<string, type_t> itemtype_map_;
    map<string, vector<string> > enumelem_map_;
    map<string, int> enumid_map_;
};

#endif /* CONFGEN_HPP */
