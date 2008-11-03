#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <conf4cpp.hpp>
#include "conf.out.hpp"

static bool tm_equal(const struct tm& t1, const struct tm& t2) {
/*
    printf("tm_sec:  %d,%d\n", t1.tm_sec,  t2.tm_sec);
    printf("tm_min:  %d,%d\n", t1.tm_min,  t2.tm_min);
    printf("tm_hour: %d,%d\n", t1.tm_hour, t2.tm_hour);
    printf("tm_mday: %d,%d\n", t1.tm_mday, t2.tm_mday);
    printf("tm_mon:  %d,%d\n", t1.tm_mon,  t2.tm_mon);
    printf("tm_year: %d,%d\n", t1.tm_year, t2.tm_year);
*/
    return t1.tm_sec  == t2.tm_sec        /* 秒 */
        && t1.tm_min  == t2.tm_min        /* 分 */
        && t1.tm_hour == t2.tm_hour       /* 時間 */
        && t1.tm_mday == t2.tm_mday       /* 日 */
        && t1.tm_mon  == t2.tm_mon        /* 月 */
        && t1.tm_year == t2.tm_year;      /* 年 */
}

static bool ipv6_equal(const struct in6_addr& a1, const struct in6_addr& a2) {
    return memcmp(a1.s6_addr, a2.s6_addr, 16) == 0;
}

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

    assert(!conf.has_int_defval());
    assert(!conf.has_int_defval2());
    assert(conf.int_defval()==5);
    assert(conf.int_defval2()==0);
    assert(conf.bool_defval()==true);
    assert(conf.bool_defval2()==false);
    assert(conf.string_defval()=="hello");
    assert(conf.string_defval2()=="");
    assert(conf.real_defval()==1.0123);
    assert(conf.real_defval2()==0.0);
    assert(boost::get<0>(conf.rib_tuple_def())==-3.14);
    assert(boost::get<1>(conf.rib_tuple_def())==5);
    assert(boost::get<2>(conf.rib_tuple_def())==false);
    assert(conf.stri_list_def().size()==3);
    assert(boost::get<0>(conf.stri_list_def()[1])=="hoge");
    assert(boost::get<1>(conf.stri_list_def()[2])==1234);

    time_t t_ = 0; 
    struct tm t = *localtime(&t_);
    assert(tm_equal(t,conf.date_value()));
    t.tm_year = 2003-1900;
    t.tm_mon = 12-1;
    t.tm_mday = 31;
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;
    assert(tm_equal(t,conf.date_value2()));
    t.tm_year = 2008-1900;
    t.tm_mon = 1-1;
    t.tm_mday = 3;
    t.tm_hour = 17;
    t.tm_min = 20;
    t.tm_sec = 0;
    assert(tm_equal(t,conf.date_value3()));
    t.tm_year = 1972-1900;
    t.tm_mon = 9-1;
    t.tm_mday = 24;
    t.tm_hour = 5;
    t.tm_min = 12;
    t.tm_sec = 30;
    assert(tm_equal(t,conf.date_value4()));

    assert(conf.ipv4_value().s_addr==0);
    assert(conf.ipv4_value2().s_addr==inet_addr("192.168.0.1"));
    assert(conf.ipv4_value3().s_addr==inet_addr("4.3.2.1"));

    struct in6_addr ipv6 = {{{0}}};
    assert(ipv6_equal(ipv6,conf.ipv6_value()));
    inet_pton(AF_INET6, "dead:beaf::1", &ipv6);
    assert(ipv6_equal(ipv6,conf.ipv6_value2()));
    inet_pton(AF_INET6, "dead:1234:ffff:ffff:2311:1111:0000:0232", &ipv6);
    assert(ipv6_equal(ipv6,conf.ipv6_value3()));
    inet_pton(AF_INET6, "::", &ipv6);
    assert(ipv6_equal(ipv6,conf.ipv6_value4()));
    inet_pton(AF_INET6, "::1", &ipv6);
    assert(ipv6_equal(ipv6,conf.ipv6_value5()));
    inet_pton(AF_INET6, "1:2:3::", &ipv6);
    assert(ipv6_equal(ipv6,conf.ipv6_value6()));
    inet_pton(AF_INET6, "1:2:3:4:5:6:7:8", &ipv6);
    assert(ipv6_equal(ipv6,conf.ipv6_value7()));

    conf.dump(cout);

    return 0;
}
