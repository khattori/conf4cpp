TARGET	= conf_error
SRCS	= \
	conf.out.cpp \
	conf_error.cpp

CONF4CPP= ../../src/conf4cpp
CXXFLAGS= -Wall
CPPFLAGS= -I/usr/include/boost-1_33_1 -I../../inc
LIBS	=

OBJS	= $(SRCS:.cpp=.o)

%.o	: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

$(TARGET)	: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS)

conf.out.cpp : conf_error.conf.def
	$(CONF4CPP) $<

.PHONY: clean
clean	:
	rm -fr $(OBJS) $(TARGET) $(DEPS) conf.out.cpp conf.out.hpp
