TARGET=../bin/tcp-client
CPPFLAGS+=-Wall -O2
LDLIBS+=-pthread
#LDLIBS+=-lws2_32 # for mingw

all: $(TARGET)

$(TARGET): tcp-client.o ../mingw_net.o
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f $(TARGET) *.o
