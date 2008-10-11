#include <conf4cpp.hpp>
#include "conf.out.hpp"

int main(int argc, char* args[])
{
    if (argc < 2) {
	cout << "no input file.\n";
	return -1;
    }

    test_conf conf(args[1]);

    assert(!conf.has_string_val());
    assert(conf.has_real_val());
    assert(conf.real_val()==-32134.5643);
    assert(conf.string_list().size()==3);
    assert(conf.string_list()[0]=="foo");
    assert(conf.string_list()[1]=="bar");
    assert(conf.string_list()[2]=="baz");
    assert(conf.string0_list().size()==0);
    assert(conf.int1_list().size()==1);
    assert(conf.int1_list()[0]==10);
    assert(conf.bool9_list().size()==9);
    assert(conf.bool9_list()[8]==true);
    assert(conf.real_llist().size()==5);
    assert(conf.real_llist()[3].size()==0);
    assert(conf.real_llist()[4][2]==1323.0);
    assert(boost::get<1>(conf.stri_list()[1])==-1234);
    assert(boost::get<0>(conf.rib_tuple())==1.234);
    assert(boost::get<1>(conf.rib_tuple())==5);
    assert(boost::get<2>(conf.rib_tuple())==true);
    assert(boost::get<0>(boost::get<0>(conf.string_tuple()))=="hoge");
    assert(boost::get<0>(conf.week_fruits())==test_conf::MON);
    assert(boost::get<1>(conf.week_fruits()).size()==4);
    assert(boost::get<1>(conf.week_fruits())[0]==test_conf::Apple);

    conf.dump(cout);

    return 0;
}
