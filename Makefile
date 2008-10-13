PREFIX	= /usr/local

TARGET	= conf4cpp

all	: $(TARGET)

$(TARGET):
	$(MAKE) -C src/

.PHONY: install
install	: $(TARGET)
	cp -r inc/* $(PREFIX)/include/
	cp src/$(TARGET) $(PREFIX)/bin/$(TARGET)

.PHONY	: clean
clean	:
	$(MAKE) -C src clean
