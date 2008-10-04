/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#include <iostream>
#include <cstdio>
#include <getopt.h>

#include "confdef_g.hpp"

using namespace std;

static const char* version = "0.0.1";
static const char* program = "conf4cpp";
static void print_version()
{
    fprintf(stdout, "%s %s\n", program, version);
}
static void print_usage()
{
    fprintf(stderr, "Usage: %s [options] files...\n", program);
    fprintf(stderr, "options:\n"
	    "  -h, --help               display this message\n"
	    "  -v, --version            display version information\n"
	    "  -o, --output=filename    specify output file name\n");
}

int main(int argc, char* argv[])
{
    char *output_file_name = "conf.out.hpp";

    for (;;) {
	static struct option long_options[] = {
	    {"help",    no_argument, NULL, 'h'},
	    {"version", no_argument, NULL, 'v'},
	    {"output",  no_argument, NULL, 'o'},
	    {0,0,0,0}
	};

	int option_index = 0;
	int c = getopt_long(argc, argv, "hvo:", long_options, &option_index);
	if (c == -1) break;
	switch (c) {
	case 'h':
	    print_usage(); return 0;
	case 'v':
	    print_version(); return 0;
	case 'o':
	    output_file_name = optarg;
	    break;
	case '?':
	    print_usage(); return 0;
	default:
	    fprintf(stderr, "?? getopt returned character code 0%o ??\n", c);
	}
    }

    if (optind >= argc) {
	fprintf(stderr, "%s: no input files\n", program);
	return -1;
    }

    while (optind < argc) {
	const char* input_file = argv[optind++];
	file_iterator<> first(input_file);
	if (!first) {
	    fprintf(stderr, "%s: unable to open file\n", input_file);
	    return -1;
	}
	file_iterator<> last = first.make_end();
	position_iterator<file_iterator<> > begin(first, last, input_file);
	position_iterator<file_iterator<> > end;
	begin.set_tabchars(8);

	confdef_g g;
	parse_info<position_iterator<file_iterator<> > > pinfo
	    = parse(begin, end, g, eol_p|space_p|comment_p("#"));
	if (!pinfo.full) {
	    fprintf(stderr, "%s: %d: parse error\n", input_file, pinfo.stop.get_position().line);
	    return -1;
	}
        cout << "element list:" << endl;
        for (map<string, vector<string> >::iterator iter = g.enumelem_map.begin();
             iter != g.enumelem_map.end();
             ++iter) {
            cout << iter->first << "={";
            for (unsigned int i = 0; i < iter->second.size(); i++) {
                cout << iter->second[i] << ' ';
            }
            cout << '}' << endl;
        }
        cout << "item list:" << endl;
        for (map<string, type_t>::iterator iter = g.itemtype_map.begin();
             iter != g.itemtype_map.end();
             ++iter) {
            cout << iter->first << endl;
        }

    }

    return 0;
}
