TARGET = poll_test

CC_OPT = -O3
CFLAGS = $(CC_OPT) $(DEVPKG_CFLAGS)
CFLAGS += -I./include
LDFLAGS = $(DEVPKG_LDFLAGS)
OBJS := $(shell find ./ -name "*.c" | sed 's/\.cpp/\.o/g')
OBJCPPS := $(shell find ./ -name "*.cpp" | sed 's/\.cpp/\.o/g')
LIBS =

all: $(TARGET)

$(TARGET):$(OBJS) $(OBJCPPS)
	$(CC) -s $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	@echo "do clean"
	rm -rf *.o src/*.o $(TARGET)

.PHONY: all clean
