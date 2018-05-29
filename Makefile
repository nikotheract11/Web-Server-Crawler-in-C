OBJS_server = QueueImplementation.o server.o
SOURCE_server = ./linked-lists/QueueImplementation.c server.c
HEADER_server = ./linked-lists/QueueInterface.h
OUT_server = myhttpd
OBJS_crawler = QueueImplementation.o crawler.o
SOURCE_crawler = ./linked-lists/QueueImplementation.c crawler.c
HEADER_crawler = ./linked-lists/QueueInterface.h
OUT_crawler = mycrawler
CC = gcc
j=jexec
all : $(OBJS_server)
	$(CC) $(OBJS_server) -o $(OUT_server) -pthread
crawler : $(OBJS_crawler) $(j)
	$(CC) $(OBJS_crawler) ./jobEx_forCrawler/objs/* -o $(OUT_crawler) -pthread -lm
QueueImplementation.o: ./linked-lists/QueueImplementation.c
	$(CC) -c ./linked-lists/QueueImplementation.c
server.o: server.c
	$(CC) -c server.c
crawler.o: crawler.c
	$(CC) -c crawler.c
jexec:
	cd jobEx_forCrawler && $(MAKE)
clean :
	rm -f $(OBJS_server) $(OUT_server)
