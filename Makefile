CC=gcc
CFLAGS=  -c -Wall

all:    mirror_client \
	sender \
	receiver

mirror_client:   mirror_client.c
	$(CC)  $(CFLAGS) mirror_client.c
	$(CC)  mirror_client.o -o mirror_client

sender:   sender.c
	$(CC)  $(CFLAGS) sender.c
	$(CC)  sender.o -o sender

receiver:   receiver.c
	$(CC)  $(CFLAGS) receiver.c
	$(CC)  receiver.o -o receiver

clean:
	rm -f   \
		mirror_client.o mirror_client \
		sender.o sender \
		receiver.o receiver
