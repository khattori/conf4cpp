#include <boost/spirit.hpp>

using namespace std;
using namespace boost::spirit;

struct confdef_g : public grammar<confdef_g>
{
    template <typename ScannerT> struct definition
    {
	rule<ScannerT> confdef_r, itemdef_r, type_r, id_r;

        definition(confdef_g const& self)
        {
            confdef_r
                = str_p("config") >> id_r >> '{' >> *itemdef_r >> '}';
            itemdef_r
		= id_r >> ':' >> type_r >> ';';
	    type_r
		= str_p("bool")
		| str_p("int")
		| str_p("uint")
		| str_p("string");

	    id_r
		= lexeme_d[ (alpha_p|'_') >> +(alnum_p|'_') ];
        }

        rule<ScannerT> const& start() const { return confdef_r; }
    };
};
