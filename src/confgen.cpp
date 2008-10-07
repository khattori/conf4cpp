/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#include "confgen.hpp"

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
//    // definitions of members
//    xxx_t xxx_;
//    bool has_xxx_t_;
// };
//
void
confgen::output_interface(ostream& os)
{
    os << "// conf4cpp : interface definition" << endl;
    output_file_header(os);
    os << "#include <conf4cpp.hpp>" << endl;
    os << "#include <boost/tuple/tuple.hpp>" << endl;
    os << "struct " << conf_name_ << "_parser;" << endl;
    os << "class " << conf_name_ << " : public conf4cpp::base_config<" << conf_name_ << "_parser>" << endl;
    os << "{" << endl;
    os << "public:" << endl;
    os << conf_name_ << "(const string& fname)" << endl;

    output_interface_enumdefs(os);
    output_interface_accessors(os);

    os << "private:" << endl;

    output_interface_members(os);

    os << "};" << endl;
}

void
confgen::output_interface_enumdefs(ostream& os)
{
    os << "\t// definitions of enum type" << endl;
    for (map<string,vector<string> >::const_iterator iter = enumelem_map_.begin();
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
    for (map<string,type_t>::const_iterator iter = itemtype_map_.begin();
         iter != itemtype_map_.end();
         ++iter) {
        os << "\tconst " << get_typestr(iter->second) << "& " << iter->first << "() const { return " << iter->first << "_; }" << endl;
        if (!itemreq_map_.find(iter->first)->second) os << "\tbool has_" << iter->first << "() const { return has_" << iter->first << "_; }" << endl;
    }
}

void
confgen::output_interface_members(ostream& os)
{
    os << "\t// definitions of members" << endl;
    for (map<string,type_t>::const_iterator iter = itemtype_map_.begin();
        iter != itemtype_map_.end();
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
    os << "// conf4cpp : implementation file" << endl;
    output_file_header(os);
    os << "#include \"" << conf_name_ << ".out.hpp\"" << endl;
    os << "using namespace conf4cpp;" << endl;
    os << "// definition of config parser" << endl;
    os << "struct " << conf_name_ << "_parser : public base_config_parser<" << conf_name_ << "_parser>" << endl;
    os << "{" << endl;
    output_implementation_keywords(os);
    output_implementation_constvals(os);
    output_implementation_parser_constructor(os);
    os << "};" << endl;
    output_implementation_config_constructor(os);
}
void
confgen::output_implementation_keywords(ostream& os)
{    
    os << "\tstruct keywords : symbols<string>" << endl;
    os << "\t{" << endl;
    os << "\t\tkeywords() {" << endl;
    os << "\t\t\tadd" << endl;
    for (map<string,type_t>::const_iterator iter = itemtype_map_.begin();
         iter != itemtype_map_.end();
         ++iter) {
        os << "\t\t\t(\"" << iter->first << "\", \"" << iter->first << "\")" << endl;
    }
    os << "\t\t\t;" << endl;
    os << "\t\t}" << endl;
    os << "\t};" << endl;
}
void
confgen::output_implementation_constvals(ostream& os)
{
    os << "\tstruct constvals : symbols<pair<int,int> >" << endl;
    os << "\t{" << endl;
    os << "\t\tconstvals() {" << endl;
    os << "\t\t\tadd" << endl;
    for (map<string,int>::const_iterator iter = enumid_map_.begin();
         iter != enumid_map_.end();
         ++iter) {
        for (unsigned int i = 0; i < enumelem_map_.find(iter->first)->second.size(); i++) {
            os << "\t\t\t(\"" << enumelem_map_.find(iter->first)->second[i] << "\", make_pair("
               << iter->second << ", " << conf_name_ << "::" << enumelem_map_.find(iter->first)->second[i] << "))" << endl;
        }
    }
    os << "\t\t\t;" << endl;
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
    os << "\t" << conf_name_ << "_parser(value_map_t& vm) : base_config_parser<" << conf_name_ << "_parser>(vm)" << endl;
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
    os << "\t\tvector<vector<type_t> > tvv;" << endl;
    os << "\t\tvector<type_t> tv;" << endl;
    for (map<string,type_t>::const_iterator iter = itemtype_map_.begin();
         iter != itemtype_map_.end();
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
    os << "// definition config constructor" << endl;
    os << conf_name_ << "::" << conf_name_ << "(const string& fname) : base_config<" << conf_name_ << "_parser>(fname)" << endl;
    os << "{" << endl;
    for (map<string,type_t>::const_iterator iter = itemtype_map_.begin();
         iter != itemtype_map_.end();
         ++iter) {
    }

    os << "}" << endl;
}

string
confgen::get_typestr(const type_t& ty)
{
    return apply_visitor(type_string(enumid_map_), ty);
}

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
        return string("ti_enum_t(") + boost::lexical_cast<string>(te.eid) + ")";
    }
    string operator() (pair<unsigned int, type_t> tp) const {
        return string("make_pair(") + boost::lexical_cast<string>(tp.first) + "," + apply_visitor(tset_string(lv+1),tp.second) + ")";
    }
    string operator() (vector<type_t> tv) const {
        string ret("(\n" + indent());
        ret += "tvv.push_back(vector<type_t>()),\n" + indent();
        for (unsigned int i = 0; i < tv.size()-1; i++) {
            ret += "tvv.back().push_back(" + apply_visitor(tset_string(lv+1),tv[i]) + "),\n" + indent();
        }
        ret += "tvv.back().push_back(" + apply_visitor(tset_string(lv+1),tv.back()) + "),\n" + indent();
        ret += "tv=tvv.back(),tvv.pop_back(),tv)";
        return ret;
    }
    string indent() const {
        string ret("\t");
        for (unsigned int i = 0; i < lv; i++) ret += "\t";
        return ret;
    }
    unsigned int lv;
};

string
confgen::get_tsetstr(const type_t& ty, unsigned int lv)
{
    return apply_visitor(tset_string(lv), ty);
}

void
confgen::output_file_header(ostream& os)
{
    os << "// This code has been automatically generated by conf4cpp." << endl;
    os << "// DO NOT EDIT" << endl;
}

