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
    class error : public exception
    {
    public:
	error(const string& msg) throw() : msg_(msg) {}
	error(const string& msg, unsigned int line) throw()
	    : msg_(msg + " at line: " +  boost::lexical_cast<string>(line)) {}
	error(const string& msg, const string& item) throw()
	    : msg_(msg + ": " +  item) {}
	virtual ~error() throw() {}
	const char* what() const throw() { return msg_.c_str(); }
    private:
	const string msg_;
    };
}

#endif /* ERROR_HPP */
