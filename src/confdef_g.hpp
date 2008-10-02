/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#include <boost/spirit.hpp>
#include <conf4cpp.hpp>

using namespace std;
using namespace boost::spirit;

using namespace conf4cpp;

struct confdef_g : public grammar<confdef_g>
{
    struct type_val : closure<type_val, type_t>       { member1 val; };
    struct uint_val : closure<uint_val, unsigned int> { member1 val; };
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
    struct add_sym
    {
        sym_s& sym;
        symtype_t styp;
        int& symid;
        add_sym(sym_s &sym_, symtype_t styp_, int& symid_)
            : sym(sym_), styp(styp_), symid(symid_) {}

        template<typename IteratorT>
        void operator() (const IteratorT& first, const IteratorT& last) const {
            string key(first, last);
            cout << key << " is defined" << endl;
            sym.add(key.c_str(), make_pair(styp, symid++));
        }
    };

    template <typename ScannerT> struct definition
    {
	rule<ScannerT> config_r, spec_r, itemdef_r, enumdef_r;
	rule<ScannerT,type_val::context_t> type_r, compound_type_r, postfix_type_r, atomic_type_r;
        rule<ScannerT,uint_val::context_t> list_type_r;
	rule<ScannerT> mandatory_r, constraints_r;
	rule<ScannerT> elemseq_r, id_r;
        rule<ScannerT> new_sym, newconf_sym, newitem_sym, newenum_sym, newelem_sym;
	sym_s sym_p;

        definition(confdef_g const& self) {
            using phoenix::arg1;
	    using phoenix::arg2;
	    using phoenix::var;
	    using phoenix::construct_;

            config_r
                = lexeme_d[str_p("config") >> blank_p] >> newconf_sym >> '{' >> *spec_r >> '}';
	    spec_r
		= (itemdef_r | enumdef_r);
            itemdef_r
		= mandatory_r >> newitem_sym >> ':' >> type_r >> ';';
	    enumdef_r
		= lexeme_d[str_p("enum") >> blank_p] >> newenum_sym >> '{' >> elemseq_r >> '}';
	    mandatory_r
		= lexeme_d[str_p("required") >> blank_p] | lexeme_d[str_p("optional") >> blank_p];
	    constraints_r
		= eps_p;

	    type_r
		= compound_type_r;
	    compound_type_r
		= postfix_type_r[compound_type_r.val=arg1] >> *(',' >> postfix_type_r);
	    postfix_type_r
		= list_type_r[var(self.list_len)=arg1] >> '<' >> compound_type_r[postfix_type_r.val=construct_<pair<int,type_t> >(self.list_len,arg1)] >> '>'
	        | atomic_type_r[postfix_type_r.val=arg1] >> !('[' >> constraints_r >> ']');
	    list_type_r
		= str_p("list")[list_type_r.val=0] >> !('[' >> uint_p[list_type_r.val=arg1] >> ']');
	    atomic_type_r
		= sym_p[var(self.cur_type)=arg1] >> eps_p(var(self.cur_type.first)==SYM_TYPENAME)[atomic_type_r.val=self.cur_type.second]
		| sym_p[var(self.cur_type)=arg1] >> eps_p(var(self.cur_type.first)==SYM_ENUM)[atomic_type_r.val=construct_<ti_enum_t>(self.cur_type.second)]
		| '(' >> compound_type_r[atomic_type_r.val=arg1] >> ')';

	    id_r
		= lexeme_d[ (alpha_p|'_') >> +(alnum_p|'_') ];
            new_sym
                = (id_r - sym_p) | sym_p[var(cout) << "symbol redefined\n"] >> nothing_p;
            newconf_sym
                = new_sym[add_sym(sym_p, SYM_CONFIG, self.symid)];
            newitem_sym
                = new_sym[add_sym(sym_p, SYM_ITEM, self.symid)];
            newenum_sym
                = new_sym[add_sym(sym_p, SYM_ENUM, self.symid)];
            newelem_sym
                = new_sym[add_sym(sym_p, SYM_ELEM, self.symid)];
	    elemseq_r
		= newelem_sym >> *(',' >> newelem_sym);
        }

        rule<ScannerT> const& start() const { return config_r; }
    };
    mutable int symid;
    mutable pair<symtype_t,int> cur_type;
    mutable unsigned int list_len;
};
