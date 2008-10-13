/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#ifndef CONFDEF_HPP
#define CONFDEF_HPP

#include <boost/spirit.hpp>
#include <boost/spirit/actor.hpp>
#include <conf4cpp.hpp>

using namespace std;
using namespace boost::spirit;

using namespace conf4cpp;

struct confdef_g : public grammar<confdef_g>
{
    struct type_val   : closure<type_val, type_t>            { member1 val; };
    struct typels_val : closure<typels_val, type_t, vector<type_t> > { member1 val; member2 tys; };
    struct uint_val   : closure<uint_val, unsigned int>      { member1 val; };
    struct tyuint_val : closure<tyuint_val, type_t, unsigned int> { member1 val; member2 len; };
    struct string_val : closure<string_val, string>          { member1 val; };
    struct strvec_val : closure<strvec_val, vector<string> > { member1 val; };
    enum symtype_t {
        SYM_RESERVED, SYM_TYPENAME, SYM_CONFIG, SYM_ENUM, SYM_ELEM, SYM_ITEM, 
    };
    struct sym_s : symbols<pair<symtype_t,int> >
    {
        sym_s() {
            add
                ("enum"     , make_pair(SYM_RESERVED, 0))
                ("config"   , make_pair(SYM_RESERVED, 0))
                ("required" , make_pair(SYM_RESERVED, 0))
                ("optional" , make_pair(SYM_RESERVED, 0))
                ("bool"     , make_pair(SYM_TYPENAME, TI_BOOL))
                ("real"     , make_pair(SYM_TYPENAME, TI_DOUBLE))
                ("int"      , make_pair(SYM_TYPENAME, TI_INT))
                ("string"   , make_pair(SYM_TYPENAME, TI_STRING))
                ("list"     , make_pair(SYM_TYPENAME, 0))
// not yet implemented ------------------------------------
//                ("date"     , make_pair(SYM_TYPENAME, 0))
//                ("time"     , make_pair(SYM_TYPENAME, 0))
//                ("in_addr"  , make_pair(SYM_TYPENAME, 0))
//                ("in6_addr" , make_pair(SYM_TYPENAME, 0))
                ;
        }
    };
    struct error
    {
        string msg;
        error(const string& msg_) : msg(msg_) {}
        template<typename IteratorT>
        void operator() (const IteratorT& first, const IteratorT& last) const {
            throw_(first, msg);
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
            sym.add(key.c_str(), make_pair(styp, 0));
        }
    };

    struct add_enumsym
    {
        sym_s& sym;
        int& enumid;
        map<string, int>& enumid_map;
        add_enumsym(sym_s &sym_, int& enumid_, map<string,int>& enumid_map_)
            : sym(sym_), enumid(enumid_), enumid_map(enumid_map_) {}

        template<typename IteratorT>
        void operator() (const IteratorT& first, const IteratorT& last) const {
            string key(first, last);
            enumid_map[key] = enumid;
            sym.add(key.c_str(), make_pair(SYM_ENUM, enumid++));
        }
    };

    struct strip_type
    {
	typedef type_t result_type;
	type_t operator()(const type_t& ty) {
            vector<type_t> tv = boost::get<vector<type_t> >(ty);
            if (tv.size() == 1) return tv.front();
            return ty;
	}
    };
    static void add_value(vector<type_t>& tv, const type_t& ty) { tv.push_back(ty); }

    template <typename ScannerT> struct definition
    {
	rule<ScannerT> config_r, spec_r, itemdef_r, enumdef_r;
	rule<ScannerT,type_val::context_t> texp_r, atomic_texp_r;
        rule<ScannerT,tyuint_val::context_t> postfix_texp_r ;
	rule<ScannerT,typels_val::context_t> compound_texp_r;
        rule<ScannerT,uint_val::context_t> list_type_r;
	rule<ScannerT> mandatory_r, constraints_r;
	rule<ScannerT,strvec_val::context_t> elemseq_r;
        rule<ScannerT> new_sym, id_r;
        rule<ScannerT,string_val::context_t> newconf_sym, newitem_sym, newenum_sym, newelem_sym;
	sym_s sym_p;

        definition(confdef_g const& self) {
            using phoenix::arg1;
	    using phoenix::arg2;
	    using phoenix::var;
	    using phoenix::construct_;
	    using phoenix::static_cast_;
            self.enumid = 0;
            //
            // <config>    ::= config { <spec>* }
            // <spec>      ::= <enumdef> | <itemdef>
            // <itemdef>   ::= <mandatory>? <id> : <compound_type> ;
            // <enumdef>   ::= enum <id> { <id> (, <id>)* }
            // <mandatory> ::= required | optional
            //
            config_r
                = lexeme_d[str_p("config") >> blank_p] >> newconf_sym[var(self.conf_name)=arg1] >> '{' >> *spec_r >> '}' >> end_p;
	    spec_r
		= (enumdef_r | itemdef_r);

            itemdef_r
		= eps_p[var(self.cur_req)=true]
                    >> !mandatory_r >> newitem_sym[var(self.cur_sym)=arg1][insert_key_a(self.itemreq_map,self.cur_req)] >> ':'
                    >> texp_r[insert_at_a(self.itemtype_map,self.cur_sym)] >> ';';
	    enumdef_r
		= lexeme_d[str_p("enum") >> blank_p]
                                         >> newenum_sym[var(self.cur_sym)=arg1]
                                         >> '{' >> elemseq_r[insert_at_a(self.enumelem_map,self.cur_sym)] >> '}';
	    mandatory_r
		= lexeme_d[str_p("required") >> blank_p] | lexeme_d[str_p("optional") >> blank_p][var(self.cur_req)=false];
            //
            // <compound_type> ::= <postfix_type> (, <postfix_type>)*
            // <postfix_type>  ::= list ([ <uint> ])? < <compound_type> >
            //                   | <atomic_type> (< <constraints> >)?
            // <atomic_type>   ::= <tid> | bool | int | real | string
            //
            texp_r
                = compound_texp_r[texp_r.val=phoenix::bind(strip_type())(arg1)];
	    compound_texp_r
		= postfix_texp_r[compound_texp_r.tys=construct_<vector<type_t> >(1,arg1)]
                    >> *(',' >> postfix_texp_r[phoenix::bind(&add_value)(compound_texp_r.tys,arg1)])
                    >> eps_p[compound_texp_r.val=compound_texp_r.tys];
	    postfix_texp_r
		= list_type_r[postfix_texp_r.len=arg1]
                    >> '<' >> texp_r[postfix_texp_r.val=construct_<pair<int,type_t> >(postfix_texp_r.len,arg1)] >> '>'
	        | atomic_texp_r[postfix_texp_r.val=arg1] >> !('[' >> constraints_r >> ']');
	    list_type_r
		= str_p("list")[list_type_r.val=0] >> !('[' >> uint_p[list_type_r.val=arg1] >> ']');
	    atomic_texp_r
		= sym_p[var(self.cur_type)=arg1]
                    >> eps_p(var(self.cur_type.first)==SYM_TYPENAME)[atomic_texp_r.val=static_cast_<ti_atomic_t>(var(self.cur_type.second))]
		| sym_p[var(self.cur_type)=arg1]
                    >> eps_p(var(self.cur_type.first)==SYM_ENUM)[atomic_texp_r.val=construct_<ti_enum_t>(var(self.cur_type.second))]
		| '(' >> compound_texp_r[atomic_texp_r.val=arg1] >> ')';

            // not yet implemented
	    constraints_r
                = eps_p;

	    id_r
		= lexeme_d[ (alpha_p|'_') >> +(alnum_p|'_') ];
            new_sym
                = (id_r - sym_p) | (sym_p >> eps_p)[error("symbol redefined")] >> nothing_p;
            newconf_sym
                = new_sym[add_sym<SYM_CONFIG>(sym_p)][newconf_sym.val=construct_<string>(arg1,arg2)];
            newitem_sym
                = new_sym[add_sym<SYM_ITEM>(sym_p)][newitem_sym.val=construct_<string>(arg1,arg2)];
            newenum_sym
                = new_sym[add_enumsym(sym_p,self.enumid,self.enumid_map)][newenum_sym.val=construct_<string>(arg1,arg2)];
            newelem_sym
                = new_sym[add_sym<SYM_ELEM>(sym_p)][newelem_sym.val=construct_<string>(arg1,arg2)];
	    elemseq_r
		= newelem_sym[var(self.elem_list)=construct_<vector<string> >(1,arg1)]
                    >> *(',' >> newelem_sym[push_back_a(self.elem_list)])
                    >> eps_p[elemseq_r.val=var(self.elem_list)];
        }

        rule<ScannerT> const& start() const { return config_r; }
    };
    mutable int enumid;
    mutable string cur_sym;
    mutable bool cur_req;
    mutable pair<symtype_t,int> cur_type;
    mutable vector<string> elem_list;
    mutable map<string, type_t> itemtype_map;
    mutable map<string, vector<string> > enumelem_map;
    mutable map<string, int> enumid_map;
    mutable map<string, bool> itemreq_map;
    mutable string conf_name;
};

#endif /* CONFDEF_HPP */
