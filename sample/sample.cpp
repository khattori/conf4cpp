#include <conf4cpp.hpp>
#include "SampleConf.out.hpp"

int main(int argc, char* args[])
{
    if (argc < 2) {
	cout << "no input file.\n";
	return -1;
    }

    SampleConf conf(args[1]);

    return 0;
}
