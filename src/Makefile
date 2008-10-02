PREFIX	= /usr/local

TARGET	= conf4cpp
SRCS	= conf4cpp.cpp
CXXFLAGS= -Wall
CPPFLAGS= -I/usr/include/boost-1_33_1 -I../inc
LIBS	=


OBJS	= $(SRCS:.cpp=.o)
DEPS	= $(SRCS:.cpp=.d)

%.o	: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

$(TARGET)	: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS)

.PHONY: clean
clean	:
	rm -fr $(OBJS) $(TARGET) $(DEPS)

%.d	: %.cpp
	@echo "generating dependency file: $@"
	@$(SHELL) -ec '$(CXX) -MM $(CPPFLAGS) $< \
		| sed "s/\($*\)\.o[ :]*/\1.o $@ : /g" > $@; \
		[ -s $@ ] || rm -f $@'

-include $(DEPS)
