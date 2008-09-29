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
#include <conf4cpp/type.hpp>

using namespace std;

namespace conf4cpp
{
    enum error_t {
	error_none,
	invalid_length,
	type_mismatch,
    };

    class file_error : public exception
    {
    public:
	file_error(const string& fname, const string& reason) throw()
	    : fname_(fname), reason_(reason), msg_(reason_ + ": " + fname_) {}
	virtual ~file_error() throw() {}
	const char* what() const throw() { return msg_.c_str();	}
    private:
	const string fname_;
	const string reason_;
	const string msg_;
    };

    class parse_error : public exception
    {
    public:
	parse_error(int line) throw() : line_(line), msg_("parse error at line: " + boost::lexical_cast<string>(line_)) {}
	virtual ~parse_error() throw() {}
	const char* what() const throw() { return msg_.c_str(); }
    private:
	const int line_;
	const string msg_;
    };

    class type_error : public exception
    {
    public:
	type_error(int line) throw()
	    : line_(line), msg_("type mismatch at line: " +  boost::lexical_cast<string>(line_)) {}
	virtual ~type_error() throw() {}
	const char* what() const throw() { return msg_.c_str(); }
    private:
	const int line_;
	const string msg_;
    };
}

#endif /* ERROR_HPP */
