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
		= lexeme_d[ alpha_p >> +alnum_p ];
        }

        rule<ScannerT> const& start() const { return confdef_r; }
    };
};

int main(int argc, char* args[])
{
    if (argc < 2) {
	cout << "no input file.\n";
	return -1;
    }

    file_iterator<> first(args[1]);
    if (!first) {
	cout << "unable to open file: " << args[1] << "\n";
	return -1;
    }
    file_iterator<> last = first.make_end();

    confdef_g g;
    parse_info<file_iterator<> > info = parse(first, last, g, space_p|comment_p("#"));

    if (!info.full) {
	cout << "parse error\n";
	return -1;
    }

    return 0;
}
