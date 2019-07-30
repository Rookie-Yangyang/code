CC=gcc
AR=ar

LDFLAGS := -L/usr/local/lib -lssl -lcrypto

server:
	${CC} ssl-server.c -o server ${LDFLAGS}

clean:
	rm -rf server
	rm -rf *.o
	rm -rf .ssl-server.c.swp

