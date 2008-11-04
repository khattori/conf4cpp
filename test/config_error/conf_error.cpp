#include <conf4cpp.hpp>
#include "conf.out.hpp"

int main(int argc, char* args[])
{
    if (argc < 2) {
	cout << "no input file.\n";
	return -1;
    }

    try {
        conf_error conf(args[1]);
    } catch (const conf4cpp::error& e) {
        cout << e.what() << endl;
        return -1;
    }

    return 0;
}
