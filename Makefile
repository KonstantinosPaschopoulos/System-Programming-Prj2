CC=gcc
CFLAGS=  -c -Wall

all:    mirror_client \
	sender \
	receiver \
	deleter

mirror_client:   mirror_client.c
	$(CC)  $(CFLAGS) mirror_client.c
	$(CC)  mirror_client.o -o mirror_client

sender:   sender.c
	$(CC)  $(CFLAGS) sender.c
	$(CC)  sender.o -o sender

receiver:   receiver.c
	$(CC)  $(CFLAGS) receiver.c
	$(CC)  receiver.o -o receiver

deleter:   deleter.c
	$(CC)  $(CFLAGS) deleter.c
	$(CC)  deleter.o -o deleter

clean:
	rm -f   \
		mirror_client.o mirror_client \
		sender.o sender \
		receiver.o receiver \
		deleter.o deleter
