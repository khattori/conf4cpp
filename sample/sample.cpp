#include <conf4cpp.hpp>
#include <boost/tuple/tuple.hpp>

using namespace conf4cpp;

//
// the following code is automatically generated by conf4cpp
//
struct MyConfigParser : public base_config_parser<MyConfigParser>
{
    enum {
	id_foo,
 	id_bar,
	id_baz,
	id_poo,
	id_por,
	id_pom,
	id_far,
    };
    struct keywords : symbols<>
    {
	keywords() {
	    add
		("foo", id_foo)
		("bar", id_bar)
		("baz", id_baz)
		("poo", id_poo)
		("por", id_por)
		("pom", id_pom)
		("far", id_far);
	}
    };
    struct constvals : symbols<>
    {
	constvals() {
	    add
		("hoge", 1)
		("bake", 2)
		("hage", 3)
		("TCP",  4);
	}
    };

    MyConfigParser(value_map_t& vmap_) : base_config_parser<MyConfigParser>(vmap_) {
	timap[id_foo] = TI_INT;
	timap[id_bar] = TI_BOOL;
	timap[id_baz] = TI_STRING;

	vector<type_t> ti_poo;
	ti_poo.push_back(TI_INT);
	ti_poo.push_back(TI_BOOL);
	timap[id_poo] = ti_poo; // (int, bool)

	pair<int, type_t> ti_por(5, TI_INT);
	timap[id_por] = ti_por; // int, int, int, int, int

	pair<int, type_t> ti_pom(0, TI_STRING); // string,string,string, ...
	timap[id_pom] = ti_pom;

	vector<type_t> ti_far;
	pair<int, type_t> ti_far_0(0, TI_STRING);
	vector<type_t> ti_far_1;
	ti_far_1.push_back(TI_INT);
	ti_far_1.push_back(TI_STRING);
	ti_far.push_back(ti_far_0);
	ti_far.push_back(ti_far_1);
	ti_far.push_back(TI_BOOL);
	timap[id_far] = ti_far;	// tuple<vector<string>,tuple<int,string>,bool>
    }
};

class MyConfig : public base_config<MyConfigParser> {
public:
    MyConfig(const string& fname) : base_config<MyConfigParser>(fname) {
	foo_ = var2int(vm_.find(MyConfigParser::id_foo)->second);
	bar_ = var2bool(vm_.find(MyConfigParser::id_bar)->second);
	baz_ = var2string(vm_.find(MyConfigParser::id_baz)->second);

	vector<var_t> poo_0_ = var2vector(vm_.find(MyConfigParser::id_poo)->second);
	poo_ = boost::tuple<int,bool>(var2int(poo_0_[0]), var2bool(poo_0_[1]));

	vector<var_t> por_0_ = var2vector(vm_.find(MyConfigParser::id_por)->second);
	por_ = vector<int>(por_0_.size());
	transform(por_0_.begin(), por_0_.end(), por_.begin(), &var2int);

	vector<var_t> pom_0_ = var2vector(vm_.find(MyConfigParser::id_pom)->second);
	pom_ = vector<string>(pom_0_.size());
	transform(pom_0_.begin(), pom_0_.end(), pom_.begin(), &var2string);

	vector<var_t> far_0_ = var2vector(vm_.find(MyConfigParser::id_far)->second);
	vector<var_t> far_0_0_ = var2vector(far_0_[0]);
	vector<string> far_0_0s_ = vector<string>(far_0_0_.size());
	transform(far_0_0_.begin(), far_0_0_.end(), far_0_0s_.begin(), &var2string);
	vector<var_t> far_0_1_ = var2vector(far_0_[1]);
	boost::tuple<int,string> far_0_1t_(var2int(far_0_1_[0]),var2string(far_0_1_[1]));
	far_ = boost::tuple<vector<string>,boost::tuple<int,string>,bool>(far_0_0s_,far_0_1t_,var2bool(far_0_[2]));
	
	cout << "OK" << endl;
    }

    const vector<string>& pom() const { return pom_; }
    bool has_pom() const { return has_pom_; }

private:
    int foo_;
    bool bar_;
    string baz_;
    boost::tuple<int, bool> poo_;
    vector<int> por_;
    vector<string> pom_;
    bool has_pom_;
    boost::tuple<vector<string>,boost::tuple<int,string>,bool> far_;
};

int main(int argc, char* args[])
{
    if (argc < 2) {
	cout << "no input file.\n";
	return -1;
    }

    MyConfig conf(args[1]);

    return 0;
}
