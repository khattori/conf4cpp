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
          enumid_map_(g.enumid_map),
          defval_map_(g.defval_map) {}

    void output_interface(ostream& os);
    void output_implementation(ostream& os);
    static void output_interface_header(ostream& os);
    static void output_implementation_header(ostream& os, const string& incfile);

private:
    //
    // format string for type variant
    // 
    struct type_string : public boost::static_visitor<string>
    {
	type_string(const map<string,int>& eidm_) : eidm(eidm_) {}
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
            for (map<string,int>::const_iterator iter = eidm.begin();
                 iter != eidm.end();
                 ++iter) {
                if (iter->second == te.eid) return iter->first;
            }
            assert(false);
            return "???";
        }
        string operator() (pair<unsigned int, type_t> tp) const {
            return string("vector<") + apply_visitor(type_string(eidm),tp.second) + " >";
        }
        string operator() (vector<type_t> tv) const {
            string ret("tuple<");
            for (unsigned int i = 0; i < tv.size()-1; i++) {
                ret += apply_visitor(type_string(eidm),tv[i]) + ",";
            }
            ret += apply_visitor(type_string(eidm),tv.back()) + " >";
            return ret;
        }
	const map<string,int>& eidm;
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
            case TI_STRING: return "TI_STRING";
            default: assert(false);
            }
        }
        string operator() (ti_enum_t te) const {
            return string("ti_enum_t(") + boost::lexical_cast<string>(te.eid) + ")";
        }
        string operator() (pair<unsigned int, type_t> tp) const {
            return string("make_pair(") + boost::lexical_cast<string>(tp.first) + "," + apply_visitor(tset_string(lv+1),tp.second) + ")";
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
        vset_string(const map<string,int>& eidm_, const string& lhs_, const string& rhs_, unsigned int lv_)
            : eidm(eidm_), lhs(lhs_), rhs(rhs_), lv(lv_) {}

        string operator() (ti_atomic_t ta) const {
            switch (ta) {
            case TI_BOOL:   return lhs + " = var2<bool>(" + rhs + ");"; 
            case TI_INT:    return lhs + " = var2<int>(" + rhs + ");";
            case TI_DOUBLE: return lhs + " = var2<double>(" + rhs + ");";
            case TI_STRING: return lhs + " = var2<string>(" + rhs + ");";
            default: assert(false);
            }
        }
        string operator() (ti_enum_t te) const {
            for (map<string,int>::const_iterator iter = eidm.begin();
                 iter != eidm.end();
                 ++iter) {
                if (iter->second == te.eid) {
                    return lhs + " = " + iter->first + "(var2<pair<int,int> >(" + rhs + ").second);";
                }
            }
            assert(false);
            return "???";
        }
        string operator() (pair<unsigned int, type_t> tp) const {
	    string istr = "i"+boost::lexical_cast<string>(lv);
            string ret("vector<var_t> " + lhs + "v = var2<vector<var_t> >(" + rhs + ");\n" + indent(lv-1));
            ret += "for (unsigned int "+istr+" = 0; "+istr+" < " + lhs + "v.size(); "+istr+"++) {\n" + indent(lv);
            ret += apply_visitor(type_string(eidm),tp.second) + " " + lhs + "iv;\n" + indent(lv);
            ret += apply_visitor(vset_string(eidm,lhs+"iv",lhs+"v["+istr+"]",lv+1),tp.second) + "\n" + indent(lv);
            ret += lhs + ".push_back(" + lhs + "iv);\n" + indent(lv-1);
            ret += "}";
            return ret;
        }
        string operator() (vector<type_t> tv) const {
            string ret;
            for (unsigned int i = 0; i < tv.size(); i++) {
                string tystr = apply_visitor(type_string(eidm), tv[i]);
                string istr = boost::lexical_cast<string>(i);
                ret += tystr + " " + lhs + istr + ";\n" + indent(lv-1);
                ret += apply_visitor(vset_string(eidm,lhs+istr,"var2<vector<var_t> >("+rhs+")["+istr+"]",lv),tv[i]) + "\n" + indent(lv-1);
            }
            ret += lhs + " = make_tuple(";
            for (unsigned int i = 0; i < tv.size()-1; i++) {
                ret += lhs + boost::lexical_cast<string>(i) + ",";
            }
            ret += lhs + boost::lexical_cast<string>(tv.size()-1) + ");";
            return ret;
        }
        const map<string,int>& eidm;
        const string& lhs;
        const string& rhs;
        unsigned int lv;
    };
    //
    // format string for value dump 
    // 
    struct dump_string : public boost::static_visitor<string>
    {
        dump_string(const string& kw_, unsigned int lv_) : kw(kw_), lv(lv_) {}

        string operator() (ti_atomic_t ta) const {
            switch (ta) {
            case TI_BOOL:   return indent(lv) + "os << (" + kw + " ? \"true\" : \"false\");\n";
            case TI_INT:    return indent(lv) + "os << " + kw + ";\n";
            case TI_DOUBLE: return indent(lv) + "os << " + kw + ";\n";
            case TI_STRING: return indent(lv) + "os << " + kw + ";\n";
            default: assert(false);
            }
            return "???";
        }
        string operator() (ti_enum_t te) const {
            return indent(lv) + "os << enum2str(" + kw + ");\n";
        }
        string operator() (pair<unsigned int, type_t> tp) const {
            // os << "{ ";
            // unsigned int i#lv;
            // for (i#lv = 0; i#lv < #kw.size()-1; i++) {
            //     apply_visitor(dump_string(#kw[i#lv], lv+1), tp.second);
            //     os << ",";
            // }
            // apply_visitor(dump_string(#kw[i#lv]}, lv+1), tp.second);
            // os << "}";
            string ret;
            string istr = "i" + boost::lexical_cast<string>(lv);
            ret += indent(lv) + "os << \"{\";\n";
            ret += indent(lv) + "for (unsigned int " + istr + " = 0; " + istr + " < " + kw + ".size(); " + istr + "++) {\n";
            ret += apply_visitor(dump_string(kw+"["+istr+"]",lv+1),tp.second);
            ret += indent(lv+1) + "if (" + istr + " < " + kw + ".size()-1) os << \",\";\n";
            ret += indent(lv) + "}\n";
            ret += indent(lv) + "os << \"}\";\n";
            return ret;
        }
        string operator() (vector<type_t> tv) const {
            string ret;
            ret += indent(lv) + "os << \"{\";\n";
            unsigned int i;
            for (i = 0; i < tv.size()-1; i++) {
                ret += apply_visitor(dump_string("boost::get<"+boost::lexical_cast<string>(i)+">("+kw+")",lv),tv[i]);
                ret += indent(lv) + "os << \",\";\n";
            }
            ret += apply_visitor(dump_string("boost::get<"+boost::lexical_cast<string>(i)+">("+kw+")",lv),tv[i]);
            ret += indent(lv) + "os << \"}\";\n";
            return ret;
        }
        const string& kw;
        unsigned int lv;
    };

    string get_typestr(const type_t& ty) { return apply_visitor(type_string(enumid_map_), ty); }
    string get_tsetstr(const type_t& ty, unsigned int lv) { return apply_visitor(tset_string(lv), ty); }
    string get_vsetstr(const type_t& ty, const string& lhs, const string& rhs, unsigned int lv)
        { return apply_visitor(vset_string(enumid_map_,lhs,rhs,lv), ty); }
    string get_dumpstr(const string& kw, const type_t& ty, unsigned int lv) { return apply_visitor(dump_string(kw+"_", lv), ty); }

    static string indent(unsigned int lv) {
        string ret;
        for (unsigned int i = 0; i < lv; i++) ret += "\t";
        return ret;
    }
    static void output_file_header(ostream& os);

    void output_interface_enumdefs(ostream& os);
    void output_interface_accessors(ostream& os);
    void output_interface_members(ostream& os);
    void output_implementation_keywords(ostream& os);
    void output_implementation_constvals(ostream& os);
    void output_implementation_parser_constructor(ostream& os);
    void output_implementation_config_constructor(ostream& os);
    void output_implementation_config_enum2str(ostream& os);
    void output_implementation_config_dump(ostream& os);

    const string conf_name_;
    const map<string, type_t> itemtype_map_;
    const map<string, bool> itemreq_map_;
    const map<string, vector<string> > enumelem_map_;
    const map<string, int> enumid_map_;
    const map<string, var_t> defval_map_;
};

#endif /* CONFGEN_HPP */
