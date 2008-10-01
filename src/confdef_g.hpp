/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#include <boost/spirit.hpp>

using namespace std;
using namespace boost::spirit;

struct confdef_g : public grammar<confdef_g>
{

    template <typename ScannerT> struct definition
    {
	rule<ScannerT> config_r, spec_r, itemdef_r, enumdef_r;
	rule<ScannerT> type_r, compound_type_r, postfix_type_r, parametric_type_r, atomic_type_r;
	rule<ScannerT> mandatory_r, constraints_r;
	rule<ScannerT> idseq_r, id_r;

	symbols<> enum_sym;

        definition(confdef_g const& self)
        {
            config_r
                = str_p("config") >> id_r >> '{' >> *spec_r >> '}';
	    spec_r
		= (itemdef_r | enumdef_r);
            itemdef_r
		= !mandatory_r >> id_r >> ':' >> type_r >> ';';
	    enumdef_r
		= str_p("enum") >> id_r[enum_sym.add] >> '{' >> idseq_r >> '}';
	    mandatory_r
		= str_p("required") | str_p("optional");
	    constraints_r
		= eps_p;

	    type_r
		= compound_type_r;
	    compound_type_r
		= postfix_type_r >> *(',' >> postfix_type_r);
	    postfix_type_r
		= atomic_type_r >> !('[' >> constraints_r >> ']')
	        | parametric_type_r >> '<' >> compound_type_r >> '>';
	    parametric_type_r
		= str_p("list") >> !('[' >> uint_p >> ']');
	    atomic_type_r
		= str_p("bool")
		| str_p("int")
		| str_p("real")
		| str_p("date")
		| str_p("time")
		| str_p("string")
		| enum_sym
		| '(' >> compound_type_r >> ')';

	    idseq_r
		= id_r >> *(',' >> id_r);
	    id_r
		= lexeme_d[ (alpha_p|'_') >> +(alnum_p|'_') ];
        }

        rule<ScannerT> const& start() const { return config_r; }
    };
};
