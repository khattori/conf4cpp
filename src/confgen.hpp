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
    //
    // format string for type variant
    // 
    struct type_string : public boost::static_visitor<string>
    {
        string operator() (ti_atomic_t ta) const {
            switch (ta) {
            case TI_BOOL:   return "bool"; 
            case TI_INT:    return "int";
            case TI_DOUBLE: return "double";
            case TI_STRING: return "string";
            default: assert(false);
            }
        }
        string operator() (ti_enum_t te) const {
            for (map<string,int>::const_iterator iter = enumid_map_.begin();
                 iter != enumid_map_.end();
                 ++iter) {
                if (iter->second == te.eid) return iter->first;
            }
            assert(false);
            return "???";
        }
        string operator() (pair<unsigned int, type_t> tp) const {
            return string("vector<") + apply_visitor(type_string(enumid_map_),tp.second) + " >";
        }
        string operator() (vector<type_t> tv) const {
            string ret("tuple<");
            for (unsigned int i = 0; i < tv.size()-1; i++) {
                ret += apply_visitor(type_string(enumid_map_),tv[i]) + ",";
            }
            ret += apply_visitor(type_string(enumid_map_),tv.back()) + " >";
            return ret;
        }
    };
    //
    // format string for type information 
    // 
    struct tset_string : public boost::static_visitor<string>
    {
        tset_string(unsigned int lv_) : lv(lv_) {}

        string operator() (ti_atomic_t ta) const {
            switch (ta) {
            case TI_BOOL:   return "TI_BOOL"; 
            case TI_INT:    return "TI_INT";
            case TI_DOUBLE: return "TI_DOUBLE";
            case TI_STRING: return "STRING";
            default: assert(false);
            }
        }
        string operator() (ti_enum_t te) const {
            for (map<string,int>::const_iterator iter = enumid_map_.begin();
                 iter != enumid_map_.end();
                 ++iter) {
                if (iter->second == te.eid) return iter->first + "(var2<pair<int,int> >(" + v + ").second)";
            }
            assert(false);
            return "???";
        }
        string operator() (pair<unsigned int, type_t> tp) const {
            return "make_vec<" + apply_visitor(type_string(),tp.second) + ">(") + v + ")");
        }
        string operator() (vector<type_t> tv) const {
            string ret("(\n" + indent(lv));
            ret += "tvv.push_back(vector<type_t>()),\n" + indent(lv);
            for (unsigned int i = 0; i < tv.size()-1; i++) {
                ret += "tvv.back().push_back(" + apply_visitor(tset_string(lv+1),tv[i]) + "),\n" + indent(lv);
            }
            ret += "tvv.back().push_back(" + apply_visitor(tset_string(lv+1),tv.back()) + "),\n" + indent(lv);
            ret += "tv=tvv.back(),tvv.pop_back(),tv)";
            return ret;
        }
        unsigned int lv;
    };

    struct vset_string : public boost::static_visitor<string>
    {
        vset_string(const string& v_, unsigned int lv_) : v(v_), lv(lv_) {}

        string operator() (ti_atomic_t ta) const {
            switch (ta) {
            case TI_BOOL:   return "var2<bool>(" + v + ")"; 
            case TI_INT:    return "var2<int>(" + v + ")";
            case TI_DOUBLE: return "var2<double>(" + v + ")";
            case TI_STRING: return "var2<string>(" + v + ")";
            default: assert(false);
            }
        }
        string operator() (ti_enum_t te) const {
            return string("ti_enum_t(") + boost::lexical_cast<string>(te.eid) + ")";
        }
        string operator() (pair<unsigned int, type_t> tp) const {
            return string("make_pair(") + boost::lexical_cast<string>(tp.first) + "," + apply_visitor(vset_string(lv+1),tp.second) + ")";
        }
        string operator() (vector<type_t> tv) const {
            string ret("(\n" + indent(lv));
            ret += "tvv.push_back(vector<type_t>()),\n" + indent(lv);
            for (unsigned int i = 0; i < tv.size()-1; i++) {
                ret += "tvv.back().push_back(" + apply_visitor(vset_string(lv+1),tv[i]) + "),\n" + indent(lv);
            }
            ret += "tvv.back().push_back(" + apply_visitor(vset_string(lv+1),tv.back()) + "),\n" + indent(lv);
            ret += "tv=tvv.back(),tvv.pop_back(),tv)";
            return ret;
        }
        unsigned int lv;
        strint v;
    };

    string get_typestr(const type_t& ty) { return apply_visitor(type_string(), ty); }
    string get_tsetstr(const type_t& ty, unsigned int lv) { return apply_visitor(tset_string(lv), ty); }
    string get_vsetstr(const type_t& ty, unsigned int lv) { return apply_visitor(vset_string("v", lv), ty); }

    string indent(unsigned int lv) const {
        string ret("\t");
        for (unsigned int i = 0; i < lv; i++) ret += "\t";
        return ret;
    }

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
