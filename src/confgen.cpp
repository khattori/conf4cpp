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
    os << "using boost::tuple;" << endl;
    os << "struct " << conf_name_ << "_parser;" << endl;
    os << "class " << conf_name_ << " : public conf4cpp::base_config<" << conf_name_ << "_parser>" << endl;
    os << "{" << endl;
    os << "public:" << endl;
    os << "\t" << conf_name_ << "(const string& fname);" << endl;

    output_interface_enumdefs(os);
    output_interface_accessors(os);

    os << "\tvoid dump(ostream& os);" << endl;
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
    output_implementation_config_dump(os);
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
    os << "\t\tvector<vector<type_t> > tvv;" << endl
       << "\t\tvector<type_t> tv;" << endl;
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
    os << "\tusing boost::make_tuple;" << endl
       << "\tvar_t v;" << endl;
    for (map<string,type_t>::const_iterator iter = itemtype_map_.begin();
         iter != itemtype_map_.end();
         ++iter) {
        if (!itemreq_map_.find(iter->first)->second) {
            // オプション項目
            os << "\tif (vm_.find(\"" << iter->first << "\")!=vm_.end()) {" << endl;
            os << "\t\tv = vm_[\"" << iter->first << "\"];" << endl;
            os << "\t\t" << get_vsetstr(iter->second, iter->first+"_", "v", 2) << endl;
            os << "\t\thas_" << iter->first << "_ = true;" << endl
               << "\t}" << endl
               << "\telse {" << endl
               << "\t\thas_" << iter->first << "_ = false;"
               << "\t}" << endl;
        } else {
            // 必須項目
            os << "\tv = vm_[\"" << iter->first << "\"];" << endl;
            os << "\t" << get_vsetstr(iter->second, iter->first+"_", "v", 1) << endl;
        }
    }

    os << "}" << endl;
}

void
confgen::output_implementation_config_dump(ostream& os)
{
    os << "// definition config dump" << endl;
    os << "void " << conf_name_ << "::dump(ostream& os) {" << endl;
    os << "\tos << \"dump [" << conf_name_ << "] {\" << endl;" << endl;
    for (map<string,type_t>::const_iterator iter = itemtype_map_.begin();
         iter != itemtype_map_.end();
         ++iter) {
        if (!itemreq_map_.find(iter->first)->second) {
            //
            // os << "option: xxx = " << .... << ";" << endl;
            // os << "option: xxx = NONE" << .... << ";" << endl;
            os << "\tif (has_" << iter->first << "_) {" << endl;
            os << "\t\tos << \"\\toptional: " << iter->first << " = \" << " << get_dumpstr(iter->first,iter->second) << " << \";\" << endl;" << endl;
            os << "\t} else {" << endl;
            os << "\t\tos << \"\\toptional: " << iter->first << " = NONE;\" << endl;" << endl;
            os << "\t}" << endl;
        } else {
            os << "\tos << \"\\trequired: " << iter->first << " = \" << " << get_dumpstr(iter->first,iter->second) << " << \";\" << endl;" << endl;
        }
    }
    os << "\tos << \"}\" << endl;" << endl;
    os << "}" << endl;
}

void
confgen::output_file_header(ostream& os)
{
    os << "// This code has been automatically generated by conf4cpp." << endl;
}

