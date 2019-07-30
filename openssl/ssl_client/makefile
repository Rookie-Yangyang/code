CC=gcc
AR=ar

LDFLAGS := -L/usr/local/lib -lssl -lcrypto
MAIN := ssl-client.c
DEST := -o client

client:
	${CC} ${MAIN} ${DEST} ${LDFLAGS}

clean:
	rm -rf client
	rm -rf *.o
	rm -rf .ssl-client.c.swp

