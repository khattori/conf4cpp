/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#include <iostream>
#include <fstream>
#include <cstdio>
#include <stdlib.h>
#include <getopt.h>

#include "confdef_g.hpp"
#include "confgen.hpp"

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
	    "  -o, --output=name        specify output file prefix name\n");
}


int main(int argc, char* argv[])
{
    char *output_file_prefix = "conf.out";

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
	    output_file_prefix = optarg;
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

    string interface_file_name = string(output_file_prefix) + ".hpp";
    string implementation_file_name = string(output_file_prefix) + ".cpp";

    char tmp_hpp[] = ".hppXXXXXX";
    char tmp_cpp[] = ".cppXXXXXX";
    int hpp_fd = mkstemp(tmp_hpp);
    if (hpp_fd < 0) {
        fprintf(stderr, "cannot create temporary file\n");
        return -1;
    }
    close(hpp_fd);
    int cpp_fd = mkstemp(tmp_cpp);
    if (cpp_fd < 0) {
        fprintf(stderr, "cannot create temporary file\n");
        return -1;
    }
    close(cpp_fd);
    ofstream ofs_hpp(tmp_hpp);
    ofstream ofs_cpp(tmp_cpp);

    confgen::output_interface_header(ofs_hpp);
    confgen::output_implementation_header(ofs_cpp, interface_file_name);

    while (optind < argc) {
	const char* input_file = argv[optind++];
	file_iterator<> first(input_file);
	if (!first) {
	    fprintf(stderr, "%s: unable to open file\n", input_file);
            ofs_hpp.close(); remove(tmp_hpp);
            ofs_cpp.close(); remove(tmp_cpp);
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
            ofs_hpp.close(); remove(tmp_hpp);
            ofs_cpp.close(); remove(tmp_cpp);
	    return -1;
	}
        //       confgen gen(g.conf_name, g.itemtype_map, g.itemreq_map, g.enumelem_map, g.enumid_map);
        confgen gen(g);

        gen.output_interface(ofs_hpp);
        gen.output_implementation(ofs_cpp);
    }

    if (rename(tmp_hpp, interface_file_name.c_str()) < 0) {
        fprintf(stderr, "cannot create %s\n", interface_file_name.c_str());
        remove(tmp_hpp);
        remove(tmp_cpp);
        return -1;
    }
    if (rename(tmp_cpp, implementation_file_name.c_str()) < 0) {
        fprintf(stderr, "cannot create %s\n", implementation_file_name.c_str());
        remove(interface_file_name.c_str());
        remove(tmp_cpp);
    }

    return 0;
}

