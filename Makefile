CC=gcc

all: m2me

m2me:
	@$(CC) m2me.c byte_order.c wave.c -o m2me.exe
