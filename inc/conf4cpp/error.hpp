/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>
#include <exception>
#include <boost/lexical_cast.hpp>

using namespace std;

namespace conf4cpp
{
    enum error_t {
	error_none,
	file_error,
	parse_error,
	invalid_length,
	type_mismatch,
	item_redefined,
	item_undefined,
    };
    const char *err2str(error_t err) {
	static const char* tbl[] = {
	    "no error",
	    "file error",
	    "parse error",
	    "invalid length",
	    "type mismatch",
	    "item redefined",
	    "item undefined",
	};
	return tbl[err];
    }
    class error : public exception
    {
    public:
	error(error_t err) throw()
	    : msg_(err2str(err)) {}
	error(error_t err, int line) throw()
	    : msg_(string(err2str(err)) + " at line: " +  boost::lexical_cast<string>(line)) {}
	error(error_t err, string item) throw()
	    : msg_(string(err2str(err)) + ": " +  item) {}
	virtual ~error() throw() {}
	const char* what() const throw() { return msg_.c_str(); }
    private:
	const string msg_;
    };
}

#endif /* ERROR_HPP */
