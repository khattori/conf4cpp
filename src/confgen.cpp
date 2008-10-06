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

// class XXX_parser;
// class XXX : public base_config<XXX_parser> {
// public:
//    enum xxx { ... };
//    enum yyy { ... };
//    XXX(const string& fname) : base_config<XXX_parser>(fname) {
//
//    }
//
//    // define accessor methods
//    const xxx_t& xxx() const { return xxx_; }
//    bool has_xxx() const { return has_xxx_; }
//
// private:
//    // define config properties
//    // xxx_t xxx_;
//    // bool has_xxx_t_;
// };


