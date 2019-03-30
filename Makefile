CC=gcc
CFLAGS=  -c -Wall

all:    mirror_client \
	sender \
	receiver \
	deleter

mirror_client:   mirror_client.o my_functions.o
	$(CC)  mirror_client.o my_functions.o -o mirror_client

sender:   sender.o my_functions.o
	$(CC)  sender.o my_functions.o -o sender

receiver:   receiver.o my_functions.o
	$(CC)  receiver.o my_functions.o -o receiver

deleter:   deleter.c
	$(CC)  $(CFLAGS) deleter.c
	$(CC)  deleter.o -o deleter

my_functions.o:   my_functions.c my_functions.h
	$(CC)  $(CFLAGS) my_functions.c

mirror_client.o:   mirror_client.c
	$(CC)  $(CFLAGS) mirror_client.c

sender.o:   sender.c
	$(CC)  $(CFLAGS) sender.c

receiver.o:   receiver.c
	$(CC)  $(CFLAGS) receiver.c

deleter.o:   deleter.c
	$(CC)  $(CFLAGS) deleter.c

clean:
	rm -f   \
		mirror_client.o mirror_client \
		sender.o sender \
		receiver.o receiver \
		deleter.o deleter \
		my_functions.o
