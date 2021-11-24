TARGET=../bin/tcp-server
CPPFLAGS+=-Wall -O2
LDLIBS+=-pthread
#LDLIBS+=-lws2_32 # for mingw

all: $(TARGET)

$(TARGET): tcp-server.o ../mingw_net.o
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f $(TARGET) *.o
