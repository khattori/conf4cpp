PREFIX	= /usr/local

TARGET	= conf4cpp
SRCS	= conf4cpp.cpp
CFLAGS	= -Wall -I/usr/include/boost1_33_1 -I../inc
LIBS	=

OBJS	= $(SRCS:.cpp=.o)

$(TARGET)	: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LIBS)

.PHONY: clean
clean	:
	rm -fr $(OBJS) $(TARGET)

# $Id$
