TARGET	= sample
SRCS	= \
	sample_conf.out.cpp \
	sample.cpp

CONF4CPP= ../src/conf4cpp
CXXFLAGS= -Wall
CPPFLAGS= -I/usr/include/boost-1_33_1 -I../inc
LIBS	=

OBJS	= $(SRCS:.cpp=.o)

%.o	: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

$(TARGET)	: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS)

sample_conf.out.cpp : sample.conf.def
	$(CONF4CPP) $<

.PHONY: clean
clean	:
	rm -fr $(OBJS) $(TARGET) $(DEPS) sample_conf.out.cpp sample_conf.out.hpp
