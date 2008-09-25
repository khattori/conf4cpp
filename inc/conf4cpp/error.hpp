/*=============================================================================
 *
 * Copyright (c) 2008- Kenta Hattori
 *
 *===========================================================================*/
#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>
#include <exception>
#include <conf4cpp/type.hpp>

using namespace std;

namespace conf4cpp
{
    class file_error : public exception
    {
    public:
	file_error(const string& fname, const string& reason) throw()
	    : fname_(fname), reason_(reason) {}
	virtual ~file_error() throw() {}
	const char* what() const throw() { return ""; }
    private:
	const string fname_;
	const string reason_;
    };

    class parse_error : public exception
    {
    public:
	parse_error(int line) throw()
	    : line_(line) {}
	virtual ~parse_error() throw() {}
	const char* what() const throw() { return ""; }
    private:
	const int line_;
    };

    class type_error : public exception
    {
    public:
	type_error(int line, const type_t& expect, const type_t& bad) throw()
	    : line_(line), expect_(expect), bad_(bad) {}
	virtual ~type_error() throw() {}
	const char* what() const throw() { return ""; }
    private:
	const int line_;
	const type_t expect_;
	const type_t bad_;
    };
}

#endif /* ERROR_HPP */
