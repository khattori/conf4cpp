/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#ifndef CONFDEF_HPP
#define CONFDEF_HPP

#include <time.h>
#include <netinet/in.h>
#include <boost/variant.hpp>
#include <boost/spirit.hpp>
#include <boost/spirit/actor.hpp>
#include <boost/spirit/error_handling/exceptions.hpp>

#include <conf4cpp.hpp>

using namespace std;
using namespace boost::spirit;
using namespace conf4cpp;

typedef map<string,vector<string> > enum_map_t;

struct confdef_g : public grammar<confdef_g>
{
    struct type_val    : closure<type_val,type_t>                 { member1 val; };
    struct typs_val    : closure<typs_val,vector<type_t> >        { member1 val; };
    struct typuint_val : closure<typuint_val,type_t,unsigned int> { member1 val; member2 len; };
    struct pairvar_val : closure<pairvar_val,pair<var_t,var_t> >  { member1 val; };
    struct int_val     : closure<int_val,unsigned int>            { member1 val; };
    struct string_val  : closure<string_val,string>               { member1 val; };
    struct strvec_val  : closure<strvec_val,vector<string> >      { member1 val; };
    enum symtype_t {
        SYM_TYPENAME, SYM_ENUM, SYM_ELEM, SYM_ITEM
    };

    struct sym_s : symbols<pair<symtype_t,type_t> >
    {
        sym_s() {
            add
                ("bool"             , make_pair(SYM_TYPENAME, TI_BOOL))
                ("real"             , make_pair(SYM_TYPENAME, TI_DOUBLE))
                ("int"              , make_pair(SYM_TYPENAME, TI_INT))
                ("uint"             , make_pair(SYM_TYPENAME, TI_UINT))
                ("string"           , make_pair(SYM_TYPENAME, TI_STRING))
                ("time"             , make_pair(SYM_TYPENAME, TI_TIME))
                ("ipv4addr"         , make_pair(SYM_TYPENAME, TI_IPV4ADDR))
                ("ipv6addr"         , make_pair(SYM_TYPENAME, TI_IPV6ADDR))
                ("list"             , make_pair(SYM_TYPENAME, TI_BOOL))
                ;
        }
    };

    template<symtype_t styp>
    struct add_sym
    {
        sym_s& sym;
        add_sym(sym_s &sym_) : sym(sym_) {}
        template<typename IteratorT>
        void operator() (const IteratorT& first, const IteratorT& last) const {
            string key(first, last);
            sym.add(key.c_str(), make_pair(styp, TI_BOOL));
        }
    };

    struct add_enumsym
    {
        sym_s& sym;
        add_enumsym(sym_s &sym_) : sym(sym_) {}
        template<typename IteratorT>
        void operator() (const IteratorT& first, const IteratorT& last) const {
            string key(first, last);
            sym.add(key.c_str(), make_pair(SYM_ENUM, ti_enum_t(key)));
        }
    };

    struct add_elemsym
    {
	typedef void result_type;
        sym_s& sym;
        enum_sym_t& elemsym;
        int& num;
        add_elemsym(sym_s &sym_, enum_sym_t& elemsym_, int &num_) : sym(sym_), elemsym(elemsym_), num(num_) {}
        template<typename IteratorT>
        result_type operator() (const string& en, const IteratorT& first, const IteratorT& last) const {
            string key(first, last);
            sym.add(key.c_str(), make_pair(SYM_ELEM, ti_enum_t(en)));
            elemsym.add(key.c_str(), make_pair(en, num++));
        }
    };

    static bool chk_range(const ti_atomic_t& atyp) {
        switch (atyp.t) {
        case TI_INT:
            if (is_int(atyp.c->first) && is_int(atyp.c->second))
                return boost::get<int>(atyp.c->first) <= boost::get<int>(atyp.c->second);
            break;
        case TI_UINT:
            if (is_uint(atyp.c->first) && is_uint(atyp.c->second))
                return boost::get<unsigned int>(atyp.c->first) <= boost::get<unsigned int>(atyp.c->second);
            break;
        case TI_DOUBLE:
            if (is_double(atyp.c->first) && is_double(atyp.c->second))
                return boost::get<double>(atyp.c->first) <= boost::get<double>(atyp.c->second);
	    break;
        default:
            assert(false);
        }
        return true;
    }

    static bool chk_rantype(ti_atomic_t& atyp) {
        switch (atyp.t) {
        case TI_INT:
            // normalize to int 
            if (is_uint(atyp.c->first) && boost::get<unsigned int>(atyp.c->first) <= INT_MAX)
                atyp.c->first = int(boost::get<unsigned int>(atyp.c->first));
            if (is_uint(atyp.c->second) && boost::get<unsigned int>(atyp.c->second) <= INT_MAX)
                atyp.c->second = int(boost::get<unsigned int>(atyp.c->second));
            return !(is_bool(atyp.c->first) && is_bool(atyp.c->second))
                && (is_bool(atyp.c->first)  || is_int(atyp.c->first))
                && (is_bool(atyp.c->second) || is_int(atyp.c->second));
        case TI_UINT:
            return !(is_bool(atyp.c->first) && is_bool(atyp.c->second))
                && (is_bool(atyp.c->first)  || is_uint(atyp.c->first))
                && (is_bool(atyp.c->second) || is_uint(atyp.c->second));
        case TI_DOUBLE:
            // normalize to double
            if (is_uint(atyp.c->first))
                atyp.c->first = double(boost::get<unsigned int>(atyp.c->first));
            else if (is_int(atyp.c->first))
                atyp.c->first = double(boost::get<int>(atyp.c->first));
            if (is_uint(atyp.c->second))
                atyp.c->second = double(boost::get<unsigned int>(atyp.c->second));
            else if (is_int(atyp.c->second))
                atyp.c->second = double(boost::get<int>(atyp.c->second));
            return !(is_bool(atyp.c->first) && is_bool(atyp.c->second))
                && (is_bool(atyp.c->first)  || is_double(atyp.c->first))
                && (is_bool(atyp.c->second) || is_double(atyp.c->second));
        default:
            return false;
        }
    }

    static type_t strip_type(const vector<type_t>& typs) { if (typs.size() == 1) return typs.front(); return typs; }
    static void add_type(vector<type_t>& typs, const type_t& ty) { typs.push_back(ty); }

    template <typename ScannerT> struct definition
    {
	rule<ScannerT> config_r, confname_r, spec_r, itemdef_r, enumdef_r;
	rule<ScannerT,type_val::context_t> texp_r, atomic_texp_r;
        rule<ScannerT,typs_val::context_t> compound_texp_r;
        rule<ScannerT,typuint_val::context_t> postfix_texp_r;
        rule<ScannerT,int_val::context_t> list_type_r;
        rule<ScannerT,pairvar_val::context_t> constraints_r;
	rule<ScannerT> mandatory_r, qualifier_r;
        rule<ScannerT> new_sym, id_r;
	rule<ScannerT,strvec_val::context_t> elemseq_r;
        rule<ScannerT,string_val::context_t> conf_sym, newitem_sym, newenum_sym, newelem_sym;
        value_parser value_p;
        symbols<> rsvwd_s;
	sym_s sym_p;

        definition(confdef_g const& self) {
            assertion<string> expect_elem_e("no enum element");
            assertion<string> rsvwd_redef_e("reserved word redefined");
            assertion<string> typnm_redef_e("type name redefined");
            assertion<string> symbol_redef_e("symbol redefined");
            assertion<string> invalid_range_e("invalid range");
            assertion<string> invalid_rantype_e("invalid range type");
            assertion<string> defval_typemismatch_e("default value is type mismatch");
            assertion<string> defval_outofrange_e("default value is out of range");
            assertion<string> parse_failed_e("parser error");

            using phoenix::arg1;
	    using phoenix::arg2;
	    using phoenix::var;
            using phoenix::bind;
	    using phoenix::construct_;
	    using phoenix::static_cast_;

            rsvwd_s = "config", "enum", "required", "optional", "const", "mutable", "true", "false",
                "bool", "real", "int",  "uint", "string", "time", "ipv4addr", "ipv6addr", "list",
                // C++ reserved words
                "asm", /*bool*/ "break", "case", "catch", "char", "class", /*const*/ "const_cast",
                "continue", "default", "delete", "do", /*double*/ "dynamic_cast", "else", /*enum*/
                "explicit", "export", "extern", /*false*/ "float", "for", "friend", "goto", "if",
                "inline", /*int*/ "long", /*mutable*/ "namespace", "new", "operator", "private",
                "protected", "public", "register", "reinterpret_cast", "return", "short", "signed",
                "sizeof", "static", "static_cast", "struct", "switch", "template", "this", "throw",
                /*true*/ "try", "typedef", "typeid", "typename", "union", "unsigned", "using",
                "virtual", "void", "volatile", "wchar_t", "while"
                ;

            //
            // <config>    ::= { config <id> ::{ <spec>* }
            // <spec>      ::= <enumdef> | <itemdef>
            // <itemdef>   ::= <mandatory>? <qualifire>? <id> <compound_type> ;
            // <enumdef>   ::= enum <id> { <id> (, <id>)* }
            // <mandatory> ::= required | optional
            //
            config_r
                = lexeme_d[str_p("config") >> blank_p] >> confname_r >> '{' >> *spec_r >> '}' >> end_p;
            confname_r
                = conf_sym[push_front_a(self.conf_name)] >> *( lexeme_d[str_p("::")] >> conf_sym[push_front_a(self.conf_name)] );
	    spec_r
		= (enumdef_r | itemdef_r);

            itemdef_r
		= eps_p[var(self.cur_req)=true][var(self.cur_con)=true]
                    >> !((mandatory_r >> !qualifier_r) | (qualifier_r >> !mandatory_r))
                    >> newitem_sym[var(self.cur_sym)=arg1][insert_key_a(self.itemreq_map,self.cur_req)][insert_key_a(self.itemcon_map,self.cur_con)]
                    >> ':'
                    >> texp_r[insert_at_a(self.itemtyp_map,self.cur_sym)]
                    >> !('='
                         >> value_p[var(value_p.val)=bind(&normalize)(var(self.itemtyp_map),var(self.cur_sym),var(value_p.val))]
                                   [insert_at_a(self.itemdef_map,self.cur_sym,value_p.val)]
                         >> defval_typemismatch_e(eps_p(bind(&type_mismatch)(var(self.itemtyp_map),var(self.itemdef_map),var(self.cur_sym))==false))
                         >> defval_outofrange_e(eps_p(bind(&out_of_range)(var(self.itemtyp_map),var(self.itemdef_map),var(self.cur_sym))==false))
                        )
                    >> ';';
	    enumdef_r
		= lexeme_d[str_p("enum") >> blank_p] 
                                         >> (( newenum_sym[var(self.cur_sym)=arg1]
                                              >> '{' >> expect_elem_e(elemseq_r[insert_at_a(self.enumelem_map,self.cur_sym)]) >> '}' )
                                             | parse_failed_e(nothing_p));
	    mandatory_r
		= lexeme_d[str_p("required") >> blank_p] | lexeme_d[str_p("optional") >> blank_p][var(self.cur_req)=false];
            qualifier_r
                = lexeme_d[str_p("const") >> blank_p] | lexeme_d[str_p("mutable") >> blank_p][var(self.cur_con)=false];
            //
            // <compound_type> ::= <postfix_type> (, <postfix_type>)*
            // <postfix_type>  ::= list ([ <uint> ])? < <compound_type> >
            //                   | <atomic_type> [< <constraints> >]?
            // <atomic_type>   ::= <tid> | bool | int | real | string
            //
            texp_r
                = compound_texp_r[texp_r.val=bind(&strip_type)(arg1)];
	    compound_texp_r
		= postfix_texp_r[compound_texp_r.val=construct_<vector<type_t> >(1,arg1)]
                    >> *(',' >> postfix_texp_r[bind(&add_type)(compound_texp_r.val,arg1)] );
	    postfix_texp_r
		= list_type_r[postfix_texp_r.len=arg1]
                    >> '<' >> texp_r[postfix_texp_r.val=bind(&make_pair<int,type_t>)(postfix_texp_r.len,arg1)] >> '>'
//                    >> !( '(' >> uint_p >> ')' )
	        | atomic_texp_r[postfix_texp_r.val=arg1];
	    list_type_r
		= str_p("list")[list_type_r.val=VAR_VECTOR] >> !('[' >> uint_p[list_type_r.val=static_cast_<int>(arg1)] >> ']');
	    atomic_texp_r
		= sym_p[var(self.cur_type)=arg1]
                    >> eps_p(var(self.cur_type.first)==SYM_TYPENAME)[var(self.cur_atyp)=bind(&get_atom)(var(self.cur_type.second))]
                    >> !('[' >> constraints_r[var(self.cur_atyp)=construct_<ti_atomic_t>(var(self.cur_atyp.t),arg1)]
                         >> (eps_p(bind(&chk_rantype)(var(self.cur_atyp))) | invalid_rantype_e(nothing_p)) >> ']'
                         >> (eps_p(bind(&chk_range)(var(self.cur_atyp))) | invalid_range_e(nothing_p))
                        )
                    >> eps_p[atomic_texp_r.val=var(self.cur_atyp)]
		| sym_p[var(self.cur_type)=arg1]
                    >> eps_p(var(self.cur_type.first)==SYM_ENUM)[atomic_texp_r.val=var(self.cur_type.second)]
		| '(' >> compound_texp_r[atomic_texp_r.val=arg1] >> ')';

	    constraints_r
                = eps_p[var(self.cur_range)=construct_<pair<var_t,var_t> >(false,false)] >>
                !( strict_real_p[var(self.cur_range.first)=arg1] | uint_p[var(self.cur_range.first)=arg1] | int_p[var(self.cur_range.first)=arg1] ) >>
                '~' >> 
                !( strict_real_p[var(self.cur_range.second)=arg1] | uint_p[var(self.cur_range.second)=arg1] | int_p[var(self.cur_range.second)=arg1] ) >>
                eps_p[constraints_r.val=var(self.cur_range)];

	    id_r
		= ( lexeme_d[(alpha_p|'_') >> +(alnum_p|'_')] - rsvwd_s ) | ( rsvwd_s >> rsvwd_redef_e(nothing_p) );
            new_sym
                = (id_r - sym_p) | ( sym_p[var(self.cur_type)=arg1]
                                     >> ( ( eps_p(var(self.cur_type.first)==SYM_TYPENAME) >> typnm_redef_e(nothing_p) ) |
                                          symbol_redef_e(nothing_p) ) );
            conf_sym
                = id_r[conf_sym.val=construct_<string>(arg1,arg2)];
            newitem_sym
                = new_sym[add_sym<SYM_ITEM>(sym_p)][newitem_sym.val=construct_<string>(arg1,arg2)];
            newenum_sym
                = new_sym[add_enumsym(sym_p)][newenum_sym.val=construct_<string>(arg1,arg2)];
            newelem_sym
                = new_sym[bind(add_elemsym(sym_p,value_p.constvals_p,self.cur_num))(var(self.cur_sym),arg1,arg2)]
                [newelem_sym.val=construct_<string>(arg1,arg2)]
                ;
	    elemseq_r
		= eps_p[var(self.cur_num)=0]
                    >> newelem_sym[var(self.elem_list)=construct_<vector<string> >(1,arg1)]
                    >> *(',' >> newelem_sym[push_back_a(self.elem_list)])
                    >> eps_p[elemseq_r.val=var(self.elem_list)];
        }

        rule<ScannerT> const& start() const { return config_r; }
    };
    mutable string cur_sym;
    mutable int cur_num;
    mutable bool cur_req, cur_con;
    mutable pair<symtype_t,type_t> cur_type;
    mutable ti_atomic_t cur_atyp;
    mutable pair<var_t,var_t> cur_range;
    mutable vector<string> elem_list;
    mutable tyinfo_map_t itemtyp_map;
    mutable value_map_t itemdef_map;
    mutable enum_map_t enumelem_map;
    mutable map<string,bool> itemreq_map, itemcon_map;
    mutable deque<string> conf_name;
};

#endif /* CONFDEF_HPP */
