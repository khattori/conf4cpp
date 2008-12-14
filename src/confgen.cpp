/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#include <limits.h>
#include "confgen.hpp"

using boost::lexical_cast;

static string get_typestr(const type_t& ty);

template <typename T> static T near_zero(T a, T b) {
    assert(a <= b);

    if (a > 0) return a;
    if (b < 0) return b;
    return T(0);
}
static string indent(unsigned int lv) {
    string ret;
    for (unsigned int i = 0; i < lv; i++) ret += "\t";
    return ret;
}
static string to_cchar(const string& name)
{
    string ret;

    for (string::const_iterator iter = name.begin(); iter != name.end(); ++iter) {
        if (isalnum(*iter)) ret += toupper(*iter);
        else ret += '_';
    }
    return ret;
}
static var_t def_value(const type_t& ty) {
    if (is_atomic_type(ty)) {
        ti_atomic_t ta = boost::get<ti_atomic_t>(ty);
        switch (ta) {
        case TI_BOOL:   return bool(false);
        case TI_INT:
            if (ta.c) {
                if (is_int(ta.c->first) && is_int(ta.c->second))
                    return near_zero(boost::get<int>(ta.c->first),boost::get<int>(ta.c->second));
                if (is_int(ta.c->first) && boost::get<int>(ta.c->first) > 0)
                    return boost::get<int>(ta.c->first);
                if (is_int(ta.c->second) && boost::get<int>(ta.c->second) < 0)
                    return boost::get<int>(ta.c->second);
            }
            return int(0);
        case TI_UINT:
            if (ta.c && is_uint(ta.c->first)) return boost::get<unsigned int>(ta.c->first);
            return (unsigned int)(0);
        case TI_DOUBLE:
            if (ta.c) {
                if (is_double(ta.c->first) && is_double(ta.c->second))
                    return near_zero(boost::get<double>(ta.c->first),boost::get<double>(ta.c->second));
                if (is_double(ta.c->first) && boost::get<double>(ta.c->first) > 0.0)
                    return boost::get<double>(ta.c->first);
                if (is_double(ta.c->second) && boost::get<double>(ta.c->second) < 0.0)
                    return boost::get<double>(ta.c->second);
            }
            return double(0.0);
        case TI_STRING: return string("");
        case TI_TIME:     { time_t t = 0; struct tm ret = *localtime(&t); return ret; }
        case TI_IPV4ADDR: { struct in_addr  ret = {0}; return ret; }
        case TI_IPV6ADDR: { struct in6_addr ret = {{{0}}}; return ret; }
        }
    } else if (is_enum_type(ty)) {
        return 0;
    }
    assert(false);
    return false; // return dummy value
}

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
    string operator() (ti_vect_t tp) const {
        return string("vector<") + apply_visitor(type_string(),tp.second) + " >";
    }
    string operator() (ti_tuple_t tv) const {
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
        return string("ti_enum_t(\"") + lexical_cast<string>(te.eid) + "\")";
    }
    string operator() (ti_vect_t tp) const {
        return string("make_pair(") + lexical_cast<string>(tp.first) + "," + apply_visitor(tset_string(lv+1),tp.second) + ")";
    }
    string operator() (ti_tuple_t tv) const {
        string ret("(\n" + indent(lv));
        ret += "tvv.push_back(ti_tuple_t()),\n" + indent(lv);
        for (unsigned int i = 0; i < tv.size()-1; i++) {
            ret += "tvv.back().push_back(" + apply_visitor(tset_string(lv+1),tv[i]) + "),\n" + indent(lv);
        }
        ret += "tvv.back().push_back(" + apply_visitor(tset_string(lv+1),tv.back()) + "),\n" + indent(lv);
        ret += "tv=tvv.back(),tvv.pop_back(),tv)";
        return ret;
    }
    unsigned int lv;
};

//
// value setter string
//
struct vset_string : public boost::static_visitor<string>
{
    vset_string(const enum_map_t& eem_, const string& lhs_, const string& rhs_, unsigned int lv_)
        : eem(eem_), lhs(lhs_), rhs(rhs_), lv(lv_) {}

    string operator() (ti_atomic_t ta) const {
        switch (ta) {
        case TI_BOOL:     return lhs + " = boost::get<bool>(" + rhs + ");"; 
        case TI_INT:      return lhs + " = boost::get<int>(" + rhs + ");";
        case TI_UINT:     return lhs + " = boost::get<unsigned int>(" + rhs + ");";
        case TI_DOUBLE:   return lhs + " = boost::get<double>(" + rhs + ");";
        case TI_STRING:   return lhs + " = boost::get<string>(" + rhs + ");";
        case TI_TIME:     return lhs + " = boost::get<struct tm>(" + rhs + ");";
        case TI_IPV4ADDR: return lhs + " = boost::get<struct in_addr>(" + rhs + ");";
        case TI_IPV6ADDR: return lhs + " = boost::get<struct in6_addr>(" + rhs + ");";
        }
        assert(false);
        return "???";
    }
    string operator() (ti_enum_t te) const {
        if (eem.find(te.eid) == eem.end()) {
            assert(false);
        }
        return lhs + " = " + te.eid + "(boost::get<pair<string,int> >(" + rhs + ").second);";
    }
    string operator() (ti_vect_t tp) const {
        string istr = "i"+lexical_cast<string>(lv);
        string ret("vector<var_t> " + lhs + "v = boost::get<vector<var_t> >(" + rhs + ");\n" + indent(lv-1));
        ret += "for (unsigned int "+istr+" = 0; "+istr+" < " + lhs + "v.size(); "+istr+"++) {\n" + indent(lv);
        ret += get_typestr(tp.second) + " " + lhs + "iv;\n" + indent(lv);
        ret += apply_visitor(vset_string(eem,lhs+"iv",lhs+"v["+istr+"]",lv+1),tp.second) + "\n" + indent(lv);
        ret += lhs + ".push_back(" + lhs + "iv);\n" + indent(lv-1);
        ret += "}";
        return ret;
    }
    string operator() (ti_tuple_t tv) const {
        string ret;
        for (unsigned int i = 0; i < tv.size(); i++) {
            string tystr = get_typestr(tv[i]);
            string istr = lexical_cast<string>(i);
            ret +=  tystr + " " + lhs + istr + ";\n" + indent(lv-1);
            ret += apply_visitor(vset_string(eem,lhs+istr,"boost::get<vector<var_t> >("+rhs+")["+istr+"]",lv),tv[i]) + "\n" + indent(lv-1);
        }
        ret += lhs + " = make_tuple(";
        for (unsigned int i = 0; i < tv.size()-1; i++) {
            ret += lhs + lexical_cast<string>(i) + ",";
        }
        ret += lhs + lexical_cast<string>(tv.size()-1) + ");";
        return ret;
    }
    const enum_map_t& eem;
    const string& lhs;
    const string& rhs;
    unsigned int lv;
};

//
// format string for range check
// 
template<typename T> static string range_check(const string& v, const var_t& a, const var_t& b) {
    string ret;
    if (is_<T>(a) && is_<T>(b))
        ret = "if (" + v + " < " + lexical_cast<string>(boost::get<T>(a))
            + " || " + v + " > " + lexical_cast<string>(boost::get<T>(b)) + ") return false;";
    else if (is_<T>(a))
        ret = "if (" + v + " < " + lexical_cast<string>(boost::get<T>(a)) + ") return false;";
    else if (is_<T>(b))
        ret = "if (" + v + " > " + lexical_cast<string>(boost::get<T>(b)) + ") return false;";
    return ret;
}
struct rchk_string : public boost::static_visitor<string>
{
    rchk_string(const string& rhs_, unsigned int lv_) : rhs(rhs_), lv(lv_) {}

    const string& rhs;
    unsigned int lv;

    string operator() (ti_atomic_t ta) const {
        if (!ta.c) return "";
        switch (ta) {
        case TI_BOOL:     break;
        case TI_INT:      return range_check<int>         (rhs, ta.c->first, ta.c->second);
        case TI_UINT:     return range_check<unsigned int>(rhs, ta.c->first, ta.c->second);
        case TI_DOUBLE:   return range_check<double>      (rhs, ta.c->first, ta.c->second);
        case TI_STRING:   break;
        case TI_TIME:     break;
        case TI_IPV4ADDR: break;
        case TI_IPV6ADDR: break;
        }
        assert(false);
        return "???";
    }
    string operator() (ti_enum_t te) const {
        return "";
    }
    string operator() (ti_vect_t tp) const {
        string ret;
        if (tp.first >= 0) {
            ret += "if (" + rhs + ".size() != " + lexical_cast<string>(tp.first) + ") return false;\n" + indent(lv);
        }
        string istr = "i"+lexical_cast<string>(lv);
        string rchk = apply_visitor(rchk_string(istr+"v",lv+1),tp.second);
 	if (rchk == "") return ret;
        ret += "for (unsigned int "+istr+" = 0; "+istr+" < " + rhs + ".size(); "+istr+"++) {\n" + indent(lv+1);
        ret += get_typestr(tp.second) + " " + istr + "v = " + rhs + "[" + istr + "];\n" + indent(lv+1);
        ret += rchk + "\n" + indent(lv);
        ret += "}";
        return ret;
    }
    string operator() (ti_tuple_t tv) const {
        string ret;
        for (unsigned i = 0; i < tv.size(); i++) {
	    string rchk = apply_visitor(rchk_string("boost::get<"+lexical_cast<string>(i)+">("+rhs+")",lv+1),tv[i]);
	    if (rchk == "") continue;
            ret += rchk + "\n" + indent(lv);
        }
        return ret;
    }
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
    string operator() (ti_vect_t tp) const {
        // os << "{ ";
        // unsigned int i#lv;
        // for (i#lv = 0; i#lv < #kw.size()-1; i++) {
        //     apply_visitor(dump_string(#kw[i#lv], lv+1), tp.second);
        //     os << ",";
        // }
        // apply_visitor(dump_string(#kw[i#lv]}, lv+1), tp.second);
        // os << "}";
        string ret;
        string istr = "i" + lexical_cast<string>(lv);
        ret += indent(lv) + "os << \"{\";\n";
        ret += indent(lv) + "for (unsigned int " + istr + " = 0; " + istr + " < " + kw + ".size(); " + istr + "++) {\n";
        ret += apply_visitor(dump_string(kw+"["+istr+"]",lv+1),tp.second);
        ret += indent(lv+1) + "if (" + istr + " < " + kw + ".size()-1) os << \",\";\n";
        ret += indent(lv) + "}\n";
        ret += indent(lv) + "os << \"}\";\n";
        return ret;
    }
    string operator() (ti_tuple_t tv) const {
        string ret;
        ret += indent(lv) + "os << \"{\";\n";
        unsigned int i;
        for (i = 0; i < tv.size()-1; i++) {
            ret += apply_visitor(dump_string("boost::get<"+lexical_cast<string>(i)+">("+kw+")",lv),tv[i]);
            ret += indent(lv) + "os << \",\";\n";
        }
        ret += apply_visitor(dump_string("boost::get<"+lexical_cast<string>(i)+">("+kw+")",lv),tv[i]);
        ret += indent(lv) + "os << \"}\";\n";
        return ret;
    }
    const string& kw;
    unsigned int lv;
};

//
// format string for default value setting
//
struct defv_string : public boost::static_visitor<string>
{

    defv_string(const string& lhs_, const var_t& dv_, unsigned int lv_) : lhs(lhs_), dv(dv_), lv(lv_)  {}

    string operator() (ti_atomic_t ta) const {
        if (is_nil(dv)) {
            dv = def_value(ta);
        }
        switch (ta) {
        case TI_BOOL:     return lhs + " = " + (boost::get<bool>(dv) ? "true" : "false") + ";";
        case TI_INT:      return lhs + " = " + lexical_cast<string>(boost::get<int>(dv)) + ";";
        case TI_UINT:     return lhs + " = " + lexical_cast<string>(boost::get<unsigned int>(dv)) + "U;";
        case TI_DOUBLE:   return lhs + " = " + lexical_cast<string>(boost::get<double>(dv)) + ";";
        case TI_STRING:   return lhs + " = \"" + boost::get<string>(dv) + "\";";
        case TI_TIME: {
            struct tm t = boost::get<struct tm>(dv);
            string ret("*localtime((t_=");
            ret += lexical_cast<string>(mktime(&t)) + ",&t_))";
            return lhs + " = " + ret + ";";
        }
        case TI_IPV4ADDR: return lhs + " = to_in_addr(" + lexical_cast<string>(boost::get<struct in_addr>(dv).s_addr) + ");";
        case TI_IPV6ADDR: {
            struct in6_addr addr = boost::get<struct in6_addr>(dv);
            unsigned int i;
            string ret("to_in6_addr(");
            for (i = 0; i < sizeof(addr.s6_addr)-1; i++) {
                ret += lexical_cast<string>(int(addr.s6_addr[i])) + ",";
            }
            ret += lexical_cast<string>(int(addr.s6_addr[i])) + ")";
            return lhs + " = " + ret + ";";
        }
        }
        assert(false);
        return "???";
    }

    string operator() (ti_enum_t te) const {
        if (is_nil(dv)) {
            return lhs + " = " + te.eid + "(0);";
        } 
        return lhs + " = " + boost::get<pair<string,int> >(dv).first + "(" + lexical_cast<string>(boost::get<pair<string,int> >(dv).second) + ");"; 
    }

    string operator() (ti_vect_t tp) const {
        if (tp.first < 0 && is_nil(dv)) {
            return lhs + " = vector<" + get_typestr(tp.second) + " >();";
        }
        string ret(get_typestr(tp.second) + " " + lhs + "iv;\n");
        if (is_nil(dv)) {
            for (int i = 0; i < tp.first; i++) {
                ret += indent(lv-1) + apply_visitor(defv_string(lhs+"iv",dv,lv+1),tp.second) + "\n" + indent(lv-1); 
                ret += indent(lv-1) + lhs + ".push_back(" + lhs + "iv);\n" + indent(lv-1);
            }
        } else {
            vector<var_t> dvv = boost::get<vector<var_t> >(dv);
            for (unsigned int i = 0; i < dvv.size(); i++) {
                ret += indent(lv-1) + apply_visitor(defv_string(lhs+"iv",dvv[i],lv+1),tp.second) + "\n";
                ret += indent(lv-1) + lhs + ".push_back(" + lhs + "iv);\n";
            } 
        }
        return ret;
    }

    string operator() (ti_tuple_t tv) const {
        string ret;
        ret += "{\n" + indent(lv-1);
        for (unsigned int i = 0; i < tv.size(); i++) {
            string tystr = get_typestr(tv[i]);
            string istr = lexical_cast<string>(i);
            ret += tystr + " " + lhs + istr + ";\n" + indent(lv-1);
            if (is_nil(dv)) {
                ret += apply_visitor(defv_string(lhs+istr,dv,lv),tv[i]) + "\n" + indent(lv-1);
            } else {
                ret += apply_visitor(defv_string(lhs+istr,boost::get<vector<var_t> >(dv)[i],lv),tv[i]) + "\n" + indent(lv-1);
            }
        }
        ret += lhs + " = make_tuple(";
        for (unsigned int i = 0; i < tv.size()-1; i++) {
            ret += lhs + lexical_cast<string>(i) + ",";
        }
        ret += lhs + lexical_cast<string>(tv.size()-1) + ");";
        ret += "}\n" + indent(lv-1);
        return ret;
    }

    const string& lhs;
    mutable var_t dv;
    unsigned int lv;        
};

static inline string get_typestr(const type_t& ty)
{ return apply_visitor(type_string(), ty); }
static inline string get_tsetstr(const type_t& ty, unsigned int lv)
{ return apply_visitor(tset_string(lv), ty); }
static inline string get_vsetstr(const type_t& ty, const string& lhs, const string& rhs, unsigned int lv, const enum_map_t& enumelem_map)
{ return apply_visitor(vset_string(enumelem_map,lhs,rhs,lv), ty); }
static inline string get_defvstr(const type_t& ty, const string& lhs, const var_t& dv, unsigned int lv)
{ return apply_visitor(defv_string(lhs,dv,lv), ty); }
static inline string get_rchkstr(const type_t& ty, const string& rhs, unsigned int lv) 
{ return apply_visitor(rchk_string(rhs,lv), ty); }
static inline string get_dumpstr(const string& kw, const type_t& ty, unsigned int lv)
{ return apply_visitor(dump_string(kw+"_", lv), ty); }

void
confgen::output_interface_header(ostream& os, const string& incfile)
{
    os << "// conf4cpp : interface definition" << endl;
    output_file_header(os);
    os << "#ifndef " << to_cchar(incfile) << endl
       << "#define " << to_cchar(incfile) << endl << endl;
    os << "#include <conf4cpp.hpp>" << endl
       << "#include <boost/tuple/tuple.hpp>" << endl
       << "using boost::tuple;" << endl;
}

void
confgen::output_interface_footer(ostream& os)
{
    os << "#endif" << endl;
}

void
confgen::output_implementation_header(ostream& os, const string& incfile)
{
    os << "// conf4cpp : implementation file" << endl;
    output_file_header(os);
    os << "#include <time.h>" << endl
       << "#include <netinet/in.h>" << endl
       << "#include <arpa/inet.h>" << endl;
    os << "#include \"" << incfile << "\"" << endl
       << "using namespace conf4cpp;" << endl;
}

void
confgen::output_implementation_footer(ostream& os)
{
}

void
confgen::output_file_header(ostream& os)
{
    os << "// This code has been automatically generated by conf4cpp." << endl;
}

// 
// output_interface:
//    output interface part of config file class
//
// --- 
// class XXX_parser;
// class XXX : public base_config<XXX_parser> {
// public:
//    // definitions of enumeration type
//    enum xxx { ... };
//    enum yyy { ... };
//    XXX(const string& fname);
//
//    // definitions of accessor methods
//    const xxx_t& xxx() const { return xxx_; }
//    bool has_xxx() const { return has_xxx_; }
//
// private:
//    // definitions of private setter
//    // definitions of members
//    xxx_t xxx_;
//    bool has_xxx_t_;
// };
//
void
confgen::output_interface(ostream& os)
{
    os << "//=============================================================================" << endl
       << "// [" << conf_name_ << "]" << endl
       << "//" << endl;
    os << "struct " << conf_name_ << "_parser;" << endl;
    os << "class " << conf_name_ << " : public conf4cpp::base_config<" << conf_name_ << "_parser>" << endl;
    os << "{" << endl;
    os << "public:" << endl;
    os << "\t" << conf_name_ << "(const string& fname);" << endl;
    
    output_interface_enumdefs(os);
    output_interface_accessors(os);

    os << "\tvoid dump(ostream& os);" << endl;
    for (enum_map_t::const_iterator iter = enumelem_map_.begin();
         iter != enumelem_map_.end();
         ++iter) {
        os << "\tstatic const char* enum2str(" << iter->first << " e);" << endl;
    }

    os << "private:" << endl;
    output_interface_initializers(os);
    output_interface_setters(os);
    output_interface_rngchks(os);
    output_interface_members(os);

    os << "};" << endl;
}

void
confgen::output_interface_enumdefs(ostream& os)
{
    os << "\t// definitions of enum type" << endl;
    for (enum_map_t::const_iterator iter = enumelem_map_.begin();
         iter != enumelem_map_.end();
         ++iter) {
        os << "\tenum " << iter->first << " { ";
        for (unsigned int i = 0; i < iter->second.size(); i++) {
            os << iter->second[i] << ", ";
        }
        os << "};" << endl;
    }
}

void
confgen::output_interface_accessors(ostream& os)
{
    os << "\t// definitions of accessors" << endl;
    os << "\tbool set(const string& itemdef);" << endl;
    for (tyinfo_map_t::const_iterator iter = itemtyp_map_.begin();
         iter != itemtyp_map_.end();
         ++iter) {
        os << "\tconst " << get_typestr(iter->second) << "& " << iter->first << "() const { return " << iter->first << "_; }" << endl;
        if (!itemreq_map_.find(iter->first)->second) os << "\tbool has_" << iter->first << "() const { return has_" << iter->first << "_; }" << endl;
        if (!itemcon_map_.find(iter->first)->second) os << "\tbool set_" << iter->first << "(const " << get_typestr(iter->second) << "& v);" << endl;
    }
}

void
confgen::output_interface_initializers(ostream& os)
{
    os << "\t// definitions of private initializers" << endl;
    for (tyinfo_map_t::const_iterator iter = itemtyp_map_.begin();
        iter != itemtyp_map_.end();
        ++iter) {
        if (!itemreq_map_.find(iter->first)->second) os << "\tvoid init_" << iter->first << "_();" << endl;
    }
}

void
confgen::output_interface_setters(ostream& os)
{
    os << "\t// definitions of private setters" << endl;
    for (tyinfo_map_t::const_iterator iter = itemtyp_map_.begin();
        iter != itemtyp_map_.end();
        ++iter) {
        os << "\tvoid set_" << iter->first << "_(const conf4cpp::var_t& v_);" << endl;
    }
}

void
confgen::output_interface_rngchks(ostream& os)
{
    os << "\t// definitions of private range checkers" << endl;
    for (tyinfo_map_t::const_iterator iter = itemtyp_map_.begin();
         iter != itemtyp_map_.end();
         ++iter) {
        if (!itemcon_map_.find(iter->first)->second) {
            os << "\tbool rngchk_" << iter->first << "_(const " << get_typestr(iter->second) << "& v_);" << endl;
        }
    }
}

void
confgen::output_interface_members(ostream& os)
{
    os << "\t// definitions of members" << endl;
    for (tyinfo_map_t::const_iterator iter = itemtyp_map_.begin();
        iter != itemtyp_map_.end();
        ++iter) {
        os << "\t" << get_typestr(iter->second) << " " << iter->first << "_;" << endl;
        if (!itemreq_map_.find(iter->first)->second) os << "\tbool has_" << iter->first << "_;" << endl;
    }

}

//
// output_implementation:
//    output implementation part of config file class
//
// --- 
// #include "XXX.out.hpp"
// struct XXX_parser : public base_config_parser<XXX_parser> {
// };
//
void
confgen::output_implementation(ostream& os)
{
    os << "//=============================================================================" << endl
       << "// [" << conf_name_ << "_parser]" << endl
       << "//" << endl;
    os << "struct " << conf_name_ << "_parser : public base_config_parser<" << conf_name_ << "_parser>" << endl;
    os << "{" << endl;
    output_implementation_keywords(os);
    output_implementation_constvals(os);
    output_implementation_parser_constructor(os);
    os << "};" << endl;
    os << "namespace conf4cpp {" << endl
       << "\ttemplate<> base_config<" << conf_name_ << "_parser>::~base_config() { delete p; }" << endl
       << "\ttemplate<> bool base_config<" << conf_name_ << "_parser>::set(const string& itemdef) { return parse_itemdef(itemdef.c_str()); }" << endl
       << "}" << endl;
    output_implementation_config_constructor(os);
    output_implementation_config_accessors(os);
    output_implementation_config_initializers(os);
    output_implementation_config_setters(os);
    output_implementation_config_rngchks(os);
    output_implementation_config_enum2str(os);
    output_implementation_config_dump(os);
}
void
confgen::output_implementation_keywords(ostream& os)
{    
    os << "\tstruct keywords : symbols<string>" << endl;
    os << "\t{" << endl;
    os << "\t\tkeywords() {" << endl;
    if (itemtyp_map_.begin() != itemtyp_map_.end()) {
    	os << "\t\t\tadd" << endl;
    	for (tyinfo_map_t::const_iterator iter = itemtyp_map_.begin();
             iter != itemtyp_map_.end();
             ++iter) {
            os << "\t\t\t(\"" << iter->first << "\", \"" << iter->first << "\")" << endl;
    	}
        os << "\t\t\t;" << endl;
    }
    os << "\t\t}" << endl;
    os << "\t};" << endl;
}
void
confgen::output_implementation_constvals(ostream& os)
{
    os << "\tstruct constvals : symbols<pair<string,int> >" << endl;
    os << "\t{" << endl;
    os << "\t\tconstvals() {" << endl;
    if (enumelem_map_.begin() != enumelem_map_.end()) {
        os << "\t\t\tadd" << endl;
        for (enum_map_t::const_iterator iter = enumelem_map_.begin();
             iter != enumelem_map_.end();
             ++iter) {
            for (unsigned int i = 0; i < enumelem_map_.find(iter->first)->second.size(); i++) {
                os << "\t\t\t(\"" << enumelem_map_.find(iter->first)->second[i] << "\", make_pair(string(\""
                   << iter->first << "\"), " << conf_name_ << "::" << enumelem_map_.find(iter->first)->second[i] << "))" << endl;
            }
        }
        os << "\t\t\t;" << endl;
    }
    os << "\t\t}" << endl;
    os << "\t};" << endl;
}

//
// XXX_parser(value_map_t& vmap_) : base_config_parser<XXX_parser>(vmap_)
// {
// }        
//
void
confgen::output_implementation_parser_constructor(ostream& os)
{
    os << "\t" << conf_name_ << "_parser()" << endl;
    os << "\t{" << endl;
    // set required items
    os << "\t\t// set required items" << endl;
    for (map<string,bool>::const_iterator iter = itemreq_map_.begin();
         iter != itemreq_map_.end();
         ++iter) {
        if (iter->second) os << "\t\treqs.push_back(\"" << iter->first << "\");" << endl;
    }
    // set item type
    os << "\t\t// set item type" << endl;
    os << "\t\tvector<ti_tuple_t> tvv;" << endl
       << "\t\tti_tuple_t tv;" << endl;
    for (tyinfo_map_t::const_iterator iter = itemtyp_map_.begin();
         iter != itemtyp_map_.end();
         ++iter) {
        os << "\t\ttimap[\"" << iter->first << "\"] = " << get_tsetstr(iter->second, 2) << ";" << endl;
    }
    os << "\t}" << endl;
}

//
// XXX::XXX(const string& fname) : base_config<XXX_parser>(fname)
// {
//      xxx_ = var2int(vm_["xxx"]);
//      has_xxx_ = true;
//      yyy_ = var2bool(vm_["yyy"]);
//      yas_zzz_ = false;
//      ...
// }
//
void
confgen::output_implementation_config_constructor(ostream& os)
{
    os << "using boost::make_tuple;" << endl;
    os << "// definition config constructor" << endl;
    os << conf_name_ << "::" << conf_name_ << "(const string& fname) : base_config<" << conf_name_ << "_parser>(fname)" << endl;
    os << "{" << endl;
    os << "\ttime_t t_ __attribute__((unused));" << endl;
    for (tyinfo_map_t::const_iterator iter = itemtyp_map_.begin();
         iter != itemtyp_map_.end();
         ++iter) {
        if (!itemreq_map_.find(iter->first)->second) {
            // オプション項目
            os << "\tif (p->vmap.find(\"" << iter->first << "\")!=p->vmap.end()) {" << endl;
            os << "\t\tset_" << iter->first << "_(p->vmap[\"" << iter->first << "\"]);" << endl
               << "\t} else {" << endl
               << "\t\thas_" << iter->first << "_ = false;" << endl
               << "\t\tinit_" << iter->first << "_();" << endl
               << "\t}" << endl;
        } else {
            // 必須項目
            os << "\tset_" << iter->first << "_(p->vmap[\"" << iter->first << "\"]);" << endl;
        }
    }

    os << "}" << endl;
}

void
confgen::output_implementation_config_accessors(ostream& os)
{
    os << "// definitions of accessors" << endl;
    for (tyinfo_map_t::const_iterator iter = itemtyp_map_.begin();
         iter != itemtyp_map_.end();
         ++iter) {
        if (!itemcon_map_.find(iter->first)->second) {
	    type_t t = iter->second;
            os << "bool " << conf_name_ << "::set_" << iter->first << "(const " << get_typestr(t) << "& v) { ";
            os << "if (rngchk_" << iter->first << "_(v)) { "
               << iter->first << "_ = v; return true; } else return false; }" << endl;
	}
    }
    os << "bool " << conf_name_ << "::set(const string& itemdef) {" << endl;
    os << "\tif (!base_config<" << conf_name_ << "_parser>::set(itemdef)) return false;" << endl;
    for (tyinfo_map_t::const_iterator iter = itemtyp_map_.begin();
         iter != itemtyp_map_.end();
         ++iter) {
        string key = iter->first;
        if (!itemcon_map_.find(key)->second) {
            os << "\tif (p->vmap.find(\"" << key << "\")!=p->vmap.end()) set_" << key << "_(p->vmap[\"" << key << "\"]);" << endl;
        } else {
            os << "\tif (p->vmap.find(\"" << key << "\")!= p->vmap.end()) return false;" << endl;
        }
    }
    os << "\treturn true;" << endl;
    os << "}" << endl;
}

void
confgen::output_implementation_config_initializers(ostream& os)
{
    os << "// definitions of private initializers" << endl;
    for (tyinfo_map_t::const_iterator iter = itemtyp_map_.begin();
         iter != itemtyp_map_.end();
         ++iter) {
        if (!itemreq_map_.find(iter->first)->second) {
            value_map_t::const_iterator di = itemdef_map_.find(iter->first);
            os << "void " << conf_name_ << "::init_" << iter->first << "_() {" << endl;
            os << "\ttime_t t_ __attribute__((unused));" << endl;
            os << "\t" << get_defvstr(iter->second, iter->first+"_", di == itemdef_map_.end() ? boost::spirit::nil_t() : di->second, 2) << endl;
            os << "}" << endl;
        }
    }
}

void
confgen::output_implementation_config_setters(ostream& os)
{
    os << "// definitions of private setters" << endl;
    for (tyinfo_map_t::const_iterator iter = itemtyp_map_.begin();
         iter != itemtyp_map_.end();
         ++iter) {
        os << "void " << conf_name_ << "::set_" << iter->first << "_(const var_t& v_) {" << endl;
        if (!itemreq_map_.find(iter->first)->second) {
            os << "\thas_" << iter->first << "_ = true;" << endl;
        }
        os << "\t" << get_vsetstr(iter->second, iter->first+"_", "v_", 2, enumelem_map_) << endl;
        os << "}" << endl;
    }
}


void
confgen::output_implementation_config_rngchks(ostream& os)
{
    os << "// definitions of private range checkers" << endl;
    for (tyinfo_map_t::const_iterator iter = itemtyp_map_.begin();
         iter != itemtyp_map_.end();
         ++iter) {
        if (!itemcon_map_.find(iter->first)->second) {
            os << "bool " << conf_name_ << "::rngchk_" << iter->first << "_(const " << get_typestr(iter->second) << "& v_) {" << endl;
            os << "\t" << get_rchkstr(iter->second, "v_", 1) << endl;
            os << "\treturn true;" << endl;
            os << "}" << endl;
        }
    }
}

void
confgen::output_implementation_config_enum2str(ostream& os)
{
    os << "// definition config enum2str" << endl;
    for (enum_map_t::const_iterator iter = enumelem_map_.begin();
         iter != enumelem_map_.end();
         ++iter) {
        os << "const char* " << conf_name_ << "::enum2str(" << iter->first << " e) {" << endl;
        os << "\tstatic const char* enumelem_tbl[] = {";
        for (unsigned int i = 0; i < iter->second.size(); i++) {
            os << "\"" << iter->second[i] << "\", ";
        }
        os << "};" << endl;
        os << "\treturn enumelem_tbl[e];" << endl;
        os << "}" << endl;
    }
}

void
confgen::output_implementation_config_dump(ostream& os)
{
    os << "// definition config dump" << endl;
    os << "void " << conf_name_ << "::dump(ostream& os) {" << endl;
    os << "\tchar buf[BUFSIZ] __attribute__((unused));" << endl;
    os << "\tos << \"[" << conf_name_ << "]\\n{\" << endl;" << endl;
    for (tyinfo_map_t::const_iterator iter = itemtyp_map_.begin();
         iter != itemtyp_map_.end();
         ++iter) {
        if (!itemreq_map_.find(iter->first)->second) {
            //
            // os << "option: xxx = " << .... << ";" << endl;
            // os << "option: xxx = NONE" << .... << ";" << endl;
            os << "\tif (has_" << iter->first << "_) {" << endl;
            os << "\t\tos << \"\\toptional: " << iter->first << " = \";" << endl
               << get_dumpstr(iter->first, iter->second, 2)
               << "\t\tos << \";\" << endl;" << endl;
            os << "\t} else {" << endl;
            os << "\t\tos << \"\\toptional: " << iter->first << " = [DEFAULT]\";" << endl
               << get_dumpstr(iter->first, iter->second, 2)
               << "\t\tos << \";\" << endl;" << endl;
            os << "\t}" << endl;
        } else {
            os << "\tos << \"\\trequired: " << iter->first << " = \";" << endl
               << get_dumpstr(iter->first, iter->second, 1)
               << "\tos << \";\" << endl;" << endl;
        }
    }
    os << "\tos << \"}\" << endl;" << endl;
    os << "}" << endl;
}
