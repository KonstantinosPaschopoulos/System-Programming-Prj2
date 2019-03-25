OBJS 	= mirror_client.o
SOURCE	= mirror_client.c
HEADER  = mytypes.h
OUT  	= mirror_client
CC	= gcc
FLAGS   = -c -Wall

$(OUT): $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

mirror_client.o: mirror_client.c
	$(CC) $(FLAGS) mirror_client.c

clean:
	rm -f $(OBJS) $(OUT)
