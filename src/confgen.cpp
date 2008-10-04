#include "confgen.hpp"

void
confgen::output(ostream& os) {
    os << "element list:" << endl;
    for (map<string, vector<string> >::iterator iter = enumelem_map_.begin();
         iter != enumelem_map_.end();
         ++iter) {
        os << iter->first << "={";
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
