#include <conf4cpp.hpp>
#include "conf.out.hpp"

int main(int argc, char* args[])
{
    if (argc < 2) {
	cout << "no input file.\n";
	return -1;
    }

    compile_guard_conf conf(args[1]);
    compile_guard_conf conf2(conf);
    compile_guard_conf conf3 = conf;
    compile_guard_conf conf4(args[1]);
    conf4 = conf;
     
    conf.dump(cout);

    return 0;
}
