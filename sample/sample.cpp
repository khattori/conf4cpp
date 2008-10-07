#include <conf4cpp.hpp>
#include <boost/tuple/tuple.hpp>
#include "myconfig.out.hpp"

int main(int argc, char* args[])
{
    if (argc < 2) {
	cout << "no input file.\n";
	return -1;
    }

    MyConfig conf(args[1]);

    return 0;
}
