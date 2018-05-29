OBJS = dir.o fifo.o app_functions.o api.o trie.o
SOURCE = dir.c fifo.c app_functions.c api.c trie.c
HEADER = trie.h
OUT = jobExecutor
CC = gcc
all : $(OBJS)
	mv *.o ./objs
dir.o: dir.c
	$(CC) -c dir.c
fifo.o : fifo.c
	$(CC) -c fifo.c
app_functions.o : app_functions.c
	$(CC) -c app_functions.c
	# clean house
api.o: api.c
	$(CC) -c api.c
trie.o: trie.c
	$(CC) -c trie.c
clean :
	rm -f $(OBJS) $(OUT)
