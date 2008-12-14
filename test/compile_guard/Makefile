TARGET	= compile_guard
SRCS	= \
	conf.out.cpp \
	compile_guard.cpp

CONF4CPP= ../../src/conf4cpp
CXXFLAGS= -Wall -g
CPPFLAGS= -I/usr/include/boost-1_33_1 -I../../inc
LIBS	=

OBJS	= $(SRCS:.cpp=.o)

%.o	: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

$(TARGET)	: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS)

conf.out.cpp : compile_guard.conf.def
	$(CONF4CPP) $<

.PHONY: clean
clean	:
	rm -fr $(OBJS) $(TARGET) $(DEPS) conf.out.cpp conf.out.hpp
