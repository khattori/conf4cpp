#include <conf4cpp.hpp>
#include "conf.out.hpp"

int main(int argc, char* args[])
{
    if (argc < 2) {
	cout << "no input file.\n";
	return -1;
    }

    sample::sample_conf conf(args[1]);
    conf.dump(cout);
    return 0;
}
