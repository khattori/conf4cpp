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
        : conf_name_(g.conf_name),
          itemtyp_map_(g.itemtyp_map),
          itemdef_map_(g.itemdef_map),
          itemreq_map_(g.itemreq_map),
          itemcon_map_(g.itemcon_map),
          enumelem_map_(g.enumelem_map) {}

    void output_interface(ostream& os);
    void output_implementation(ostream& os);
    static void output_interface_header(ostream& os, const string& incfile);
    static void output_interface_footer(ostream& os);
    static void output_implementation_header(ostream& os, const string& incfile);
    static void output_implementation_footer(ostream& os);

private:
    static string to_cchar(const string& name);
    //
    // format string for type variant
    // 
    struct type_string : public boost::static_visitor<string>
    {
        string operator() (ti_atomic_t ta) const {
            switch (ta) {
            case TI_BOOL:     return "bool"; 
            case TI_INT:      return "int";
            case TI_UINT:     return "unsigned int";
            case TI_DOUBLE:   return "double";
            case TI_STRING:   return "string";
            case TI_TIME:     return "struct tm";
            case TI_IPV4ADDR: return "struct in_addr";
            case TI_IPV6ADDR: return "struct in6_addr";
            }
            assert(false);
            return "???";
        }
        string operator() (ti_enum_t te) const { return te.eid; }
        string operator() (pair<unsigned int, type_t> tp) const {
            return string("vector<") + apply_visitor(type_string(),tp.second) + " >";
        }
        string operator() (vector<type_t> tv) const {
            string ret("tuple<");
            for (unsigned int i = 0; i < tv.size()-1; i++) {
                ret += apply_visitor(type_string(),tv[i]) + ",";
            }
            ret += apply_visitor(type_string(),tv.back()) + " >";
            return ret;
        }
    };
    //
    // format string for type information 
    // 
    struct tset_string : public boost::static_visitor<string>
    {
        tset_string(unsigned int lv_) : lv(lv_) {}
        string range_string(const string& ta_name, ti_atomic_t ta) const {
	    if (!ta.c) return ta_name;
            return  "ti_atomic_t(" + ta_name + ",make_pair(" + to_str(ta.c->first) + "," + to_str(ta.c->second) + "))";
        }
        string operator() (ti_atomic_t ta) const {
            switch (ta) {
            case TI_BOOL:     return "TI_BOOL"; 
            case TI_INT:      return range_string("TI_INT",ta);
            case TI_UINT:     return range_string("TI_UINT",ta);
            case TI_DOUBLE:   return range_string("TI_DOUBLE",ta);
            case TI_STRING:   return "TI_STRING";
            case TI_TIME:     return "TI_TIME";
            case TI_IPV4ADDR: return "TI_IPV4ADDR";
            case TI_IPV6ADDR: return "TI_IPV6ADDR";
            }
            assert(false);
            return "???";
        }
        string operator() (ti_enum_t te) const {
            return string("ti_enum_t(\"") + boost::lexical_cast<string>(te.eid) + "\")";
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
        vset_string(const map<string,vector<string> >& eem_, const string& lhs_, const string& rhs_, unsigned int lv_)
            : eem(eem_), lhs(lhs_), rhs(rhs_), lv(lv_) {}

        string operator() (ti_atomic_t ta) const {
            switch (ta) {
            case TI_BOOL:     return lhs + " = var2<bool>(" + rhs + ");"; 
            case TI_INT:      return lhs + " = is_int(" + rhs + ") ? var2<int>(" + rhs + ") : var2<unsigned int>(" + rhs + ");";
            case TI_UINT:     return lhs + " = var2<unsigned int>(" + rhs + ");";
            case TI_DOUBLE:   return lhs + " = var2<double>(" + rhs + ");";
            case TI_STRING:   return lhs + " = var2<string>(" + rhs + ");";
            case TI_TIME:     return lhs + " = var2<struct tm>(" + rhs + ");";
            case TI_IPV4ADDR: return lhs + " = var2<struct in_addr>(" + rhs + ");";
            case TI_IPV6ADDR: return lhs + " = var2<struct in6_addr>(" + rhs + ");";
            }
            assert(false);
            return "???";
        }
        string operator() (ti_enum_t te) const {
            if (eem.find(te.eid) == eem.end()) {
                assert(false);
            }
            return lhs + " = " + te.eid + "(var2<pair<string,int> >(" + rhs + ").second);";
        }
        string operator() (pair<unsigned int, type_t> tp) const {
	    string istr = "i"+boost::lexical_cast<string>(lv);
            string ret("vector<var_t> " + lhs + "v = var2<vector<var_t> >(" + rhs + ");\n" + indent(lv-1));
            ret += "for (unsigned int "+istr+" = 0; "+istr+" < " + lhs + "v.size(); "+istr+"++) {\n" + indent(lv);
            ret += apply_visitor(type_string(),tp.second) + " " + lhs + "iv;\n" + indent(lv);
            ret += apply_visitor(vset_string(eem,lhs+"iv",lhs+"v["+istr+"]",lv+1),tp.second) + "\n" + indent(lv);
            ret += lhs + ".push_back(" + lhs + "iv);\n" + indent(lv-1);
            ret += "}";
            return ret;
        }
        string operator() (vector<type_t> tv) const {
            string ret;
            for (unsigned int i = 0; i < tv.size(); i++) {
                string tystr = apply_visitor(type_string(), tv[i]);
                string istr = boost::lexical_cast<string>(i);
                ret += tystr + " " + lhs + istr + ";\n" + indent(lv-1);
                ret += apply_visitor(vset_string(eem,lhs+istr,"var2<vector<var_t> >("+rhs+")["+istr+"]",lv),tv[i]) + "\n" + indent(lv-1);
            }
            ret += lhs + " = make_tuple(";
            for (unsigned int i = 0; i < tv.size()-1; i++) {
                ret += lhs + boost::lexical_cast<string>(i) + ",";
            }
            ret += lhs + boost::lexical_cast<string>(tv.size()-1) + ");";
            return ret;
        }
        const map<string, vector<string> >& eem;
        const string& lhs;
        const string& rhs;
        unsigned int lv;
    };

    //
    // format string for default value setting
    //
    struct defv_string : public boost::static_visitor<string>
    {
        defv_string(const var_t& dv_) : dv(dv_) {}

        string operator() (ti_atomic_t ta) const {
            switch (ta) {
            case TI_BOOL:     return boost::get<bool>(dv) ? "true" : "false";
            case TI_INT:      if (is_uint(dv)) return boost::lexical_cast<string>(boost::get<uint>(dv)); else return boost::lexical_cast<string>(boost::get<int>(dv));
            case TI_UINT:     return boost::lexical_cast<string>(boost::get<unsigned int>(dv)) + "U";
            case TI_DOUBLE:   return boost::lexical_cast<string>(boost::get<double>(dv));
            case TI_STRING:   return "\"" + boost::get<string>(dv) + "\"";
            case TI_TIME: {
                struct tm t = boost::get<struct tm>(dv);
                string ret("*localtime((t_=");
                ret += boost::lexical_cast<string>(mktime(&t)) + ",&t_))";
                return ret;
            }
            case TI_IPV4ADDR: return "to_in_addr("+ boost::lexical_cast<string>(boost::get<struct in_addr>(dv).s_addr) + ")";
            case TI_IPV6ADDR: {
                struct in6_addr addr = boost::get<struct in6_addr>(dv);
                unsigned int i;
                string ret("to_in6_addr(");
                for (i = 0; i < sizeof(addr.s6_addr)-1; i++) {
                    ret += boost::lexical_cast<string>(int(addr.s6_addr[i])) + ",";
                }
                ret += boost::lexical_cast<string>(int(addr.s6_addr[i])) + ")";

                return ret;
            }
            }
            assert(false);
            return "???";
        }
        string operator() (ti_enum_t te) const {
		if (is_int(dv)) {
                    return te.eid + "(0)";
                } 
                return boost::get<string>(dv); 
        }
        string operator() (pair<unsigned int, type_t> tp) const {
            if (tp.first == 0) {
                return string("vector<") + apply_visitor(type_string(),tp.second) + " >()";
            } else {
                return string("vector<") + apply_visitor(type_string(),tp.second) + " >(" + boost::lexical_cast<string>(tp.first) + "," + apply_visitor(defv_string(dv),tp.second) + ")";
            }
        }
        string operator() (vector<type_t> tv) const {
            string ret("make_tuple(");
            vector<var_t> vec = boost::get<vector<var_t> >(dv);
            for (unsigned int i = 0; i < tv.size()-1; i++) {
                ret += apply_visitor(defv_string(vec[i]), tv[i]) + ",";
            }
            ret += apply_visitor(defv_string(vec.back()), tv.back()) + ")";
            return ret;
        }
        const var_t& dv;
    };

    //
    // format string for value dump 
    // 
    struct dump_string : public boost::static_visitor<string>
    {
        dump_string(const string& kw_, unsigned int lv_) : kw(kw_), lv(lv_) {}

        string operator() (ti_atomic_t ta) const {
            switch (ta) {
            case TI_BOOL:     return indent(lv) + "os << (" + kw + " ? \"true\" : \"false\");\n";
            case TI_INT:
	    case TI_UINT: 
	    case TI_DOUBLE:   return indent(lv) + "os << " + kw + ";\n";
            case TI_STRING:   return indent(lv) + "os << \"\\\"\" << " + kw + " << \"\\\"\";\n";
            case TI_TIME:     return indent(lv) + "os << " + "(strftime(buf,sizeof(buf),\"\%Y/\%m/\%d \%T\",&" + kw + "),buf);\n";
            case TI_IPV4ADDR: return indent(lv) + "os << " + "inet_ntop(AF_INET, &" + kw + ", buf, INET_ADDRSTRLEN);\n";
            case TI_IPV6ADDR: return indent(lv) + "os << " + "inet_ntop(AF_INET6, &" + kw + ", buf, INET6_ADDRSTRLEN);\n";
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

    string get_typestr(const type_t& ty) { return apply_visitor(type_string(), ty); }
    string get_tsetstr(const type_t& ty, unsigned int lv) { return apply_visitor(tset_string(lv), ty); }
    string get_vsetstr(const type_t& ty, const string& lhs, const string& rhs, unsigned int lv)
        { return apply_visitor(vset_string(enumelem_map_,lhs,rhs,lv), ty); }
    string get_defvstr(const type_t& ty, const var_t& dv) { return apply_visitor(defv_string(dv), ty); }
    string get_dumpstr(const string& kw, const type_t& ty, unsigned int lv) { return apply_visitor(dump_string(kw+"_", lv), ty); }

    static string indent(unsigned int lv) {
        string ret;
        for (unsigned int i = 0; i < lv; i++) ret += "\t";
        return ret;
    }
    static void output_file_header(ostream& os);

    void output_interface_enumdefs(ostream& os);
    void output_interface_accessors(ostream& os);
    void output_interface_setters(ostream& os);
    void output_interface_members(ostream& os);
    void output_implementation_keywords(ostream& os);
    void output_implementation_constvals(ostream& os);
    void output_implementation_parser_constructor(ostream& os);
    void output_implementation_config_constructor(ostream& os);
    void output_implementation_config_accessors(ostream& os);
    void output_implementation_config_setters(ostream& os);
    void output_implementation_config_enum2str(ostream& os);
    void output_implementation_config_dump(ostream& os);

    const string conf_name_;
    const map<string, type_t> itemtyp_map_;
    const map<string, var_t> itemdef_map_;
    const map<string, bool> itemreq_map_, itemcon_map_;
    const map<string, vector<string> > enumelem_map_;
};

#endif /* CONFGEN_HPP */
