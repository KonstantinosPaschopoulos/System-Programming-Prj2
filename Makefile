OBJS 	= mirror_client.o client_functions.o
SOURCE	= mirror_client.c client_functions.c
HEADER  = client_functions.h
OUT  	= mirror_client
CC	= gcc
FLAGS   = -c -Wall

$(OUT): $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

mirror_client.o: mirror_client.c
	$(CC) $(FLAGS) mirror_client.c

client_functions.o: client_functions.c
	$(CC) $(FLAGS) client_functions.c

clean:
	rm -f $(OBJS) $(OUT)
