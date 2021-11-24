.PHONY : tcp-client tcp-server clean install uninstall

all: tcp-client tcp-server

tcp-client:
	cd client; make; cd ..

tcp-server:
	cd server; make; cd ..

clean:
	cd client; make clean; cd ..
	cd server; make clean; cd ..
	rm -f *.o

install:	
	sudo cp bin/tcp-server /usr/sbin
	sudo cp bin/tcp-client /usr/sbin

uninstall:
	sudo rm /usr/sbin/tcp-client /usr/sbin/tcp-server
