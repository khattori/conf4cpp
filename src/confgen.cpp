/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#include "confgen.hpp"

//    mutable map<string, type_t> itemtype_map;
//    mutable map<string, vector<string> > enumelem_map;
//    mutable map<string, int> enumid_map;
//    mutable string conf_name;

void
confgen::output(ostream& os) {
    os << "conf name:" << conf_name_ << endl;
    os << "element list:" << endl;
    for (map<string, vector<string> >::iterator iter = enumelem_map_.begin();
         iter != enumelem_map_.end();
         ++iter) {
        
        os << iter->first << ":" << enumid_map_[iter->first] << "={";
        for (unsigned int i = 0; i < iter->second.size(); i++) {
            os << iter->second[i] << ' ';
        }
        os << '}' << endl;
    }
    os << "item list:" << endl;
    for (map<string, type_t>::iterator iter = itemtype_map_.begin();
         iter != itemtype_map_.end();
         ++iter) {
        os << iter->first << endl;
    }
}
