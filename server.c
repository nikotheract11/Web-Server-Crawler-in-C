#include <stdio.h>
#include <sys/wait.h>	     /* sockets */
#include <sys/types.h>	     /* sockets */
#include <sys/socket.h>	     /* sockets */
#include <netinet/in.h>	     /* internet sockets */
#include <netdb.h>	         /* gethostbyaddr */
#include <unistd.h>	         /* fork */
#include <stdlib.h>	         /* exit */
#include <ctype.h>	         /* toupper */
#include <signal.h>          /* signal */
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <time.h>

#include "./linked-lists/QueueInterface.h"

Queue fds;    /* Queue holding fd's of sockets (requests) to be served */

pthread_mutex_t  q_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv;
int count;    /* Stores the # of fds in the queue */

time_t start;

int PAGES;
int BYTES;
int EXIT;

char *root_dir;

void child_server(int newsock);
void perror_exit(char *message);
void sigchld_handler (int sig);
void create_threads(int thread_count, pthread_t *threads);
void server_command(int sock);
int mygetopt(int argc, char * const argv[], int *port,int *portc,int *threads,char **root_dir);

int main(int argc, char *argv[]) {

    pthread_cond_init(&cv, NULL);
    start = time(NULL);
    int port, c_port, thread_count=0, sock, newsock;
    mygetopt(argc,argv,&port,&c_port,&thread_count,&root_dir);
    struct sockaddr_in server, server2, client;
    socklen_t clientlen=0;
    pthread_t *threads = malloc(thread_count*sizeof(pthread_t));
   // create_threads(thread_count,threads);

    struct sockaddr *serverptr=(struct sockaddr *)&server;
    struct sockaddr *clientptr=(struct sockaddr *)&client;
    struct sockaddr *serverptr2=(struct sockaddr *)&server2;


   /* Initialize Queue */
    pthread_mutex_lock(&q_mutex);
    InitializeQueue(&fds);
    pthread_mutex_unlock(&q_mutex);

    if (argc != 9) {
        printf("Please give port number\n");exit(1);}
    int sock_c;

    signal(SIGPIPE, SIG_IGN);   /* Handle SIGPIPE, ignoring this signal */

    /* Create sockets */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");
    if((sock_c = socket(AF_INET,SOCK_STREAM,0)) < 0)
      perror_exit("sock_c");


    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    server.sin_family = AF_INET;       /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);      /* The given port */

    server2.sin_family = AF_INET;       /* Internet domain */
    server2.sin_addr.s_addr = htonl(INADDR_ANY);
    server2.sin_port = htons(c_port);

    /* Bind socket to address */
    if (bind(sock, serverptr, sizeof(server)) < 0)
        perror_exit("bind");
    if(bind(sock_c,serverptr2,sizeof(server2))<0)
        perror_exit("bind");

    /* Listen for connections */
    if (listen(sock, 500) < 0) perror_exit("listen");
    if (listen(sock_c, 5) < 0) perror_exit("listen");

    struct pollfd clients[2];
    clients[0].fd = sock;
    clients[0].events = POLLRDNORM;

    clients[1].fd = sock_c;
    clients[1].events = POLLRDNORM;

    printf("Listening for connections to port %d\n", port);
    create_threads(thread_count,threads);   /* Create threadpool */
    while (1) {

      /* accept connection */
      poll(clients,2,-1);
      if (clients[0].revents & POLLRDNORM) {  /* serving port */
         if ((newsock = accept(sock, clientptr, &clientlen)) < 0) perror_exit("accept");

      /* Insert socket to the queue so that a thread serves it */
         pthread_mutex_lock(&q_mutex);
         Insert(&newsock,&fds,0,sizeof(int));
         count++;
         pthread_mutex_unlock(&q_mutex);
         pthread_cond_signal(&cv);

    	   printf("Accepted connection\n");
   }
   if (clients[1].revents & POLLRDNORM) { /* command port */
      int newsock_c;
      if((newsock_c = accept(sock_c,clientptr,&clientlen)) < 0) perror_exit("accept");
      server_command(newsock_c);
    }
    if(EXIT == 1) {
      pthread_cond_signal(&cv);
      break;
    }
}
int *ret;
for(int i=0;i<thread_count;i++){
  pthread_cond_signal(&cv);
  pthread_join(threads[i],(void**)&ret);
  pthread_cond_signal(&cv);
}
free(threads);
free(root_dir);
pthread_exit(NULL);
}
void *serve_th();

void create_threads(int thread_count, pthread_t *threads){
   for(int i = 0; i < thread_count; i++) {
        if(pthread_create(threads+i, NULL,serve_th, NULL) != 0) {
         //   return NULL;
         }
      }
}

void send_site(int sock,char *url){
   char *path = malloc(strlen(url) + strlen(root_dir)+1);

   sprintf(path,"%s%s",root_dir,url);
   int fd = open(path,O_RDONLY);

   int n;
   char buf[1024];


   if (fd < 0) {
      if (errno == ENOENT){
         char *rep="HTTP/1.1 404 Not Found\r\n"
         "Date: Mon, 27 May 2018 12:28:53 GMT\r\n"
         "Server: myhttpd/1.0.0 (Ubuntu64)\r\n"
         "Content-Length: 124\r\n"
         "Content-Type: text/html\r\n"
         "Connection: Closed\r\n"
         "\r\n\r\n"
         "<html>Sorry dude, couldn’t find this file.</html>";

         if((n=write(sock,rep,strlen(rep))) < 0 )  perror("write NE");
        // close(fd);
         free(path);  // Clean up memory  ==========
         return;
      }

      else if( errno == EACCES){
         char *rep="HTTP/1.1 403 Forbidden\r\n"
         "Date: Mon, 27 May 2018 12:28:53 GMT\r\n"
         "Server: myhttpd/1.0.0 (Ubuntu64)\r\n"
         "Content-Length: 124\r\n"
         "Content-Type: text/html\r\n"
         "Connection: Closed\r\n"
         "\r\n\r\n"
         "<html>Trying to access this file but don’t think I can make it</html>";

         if((n=write(sock,rep,strlen(rep))) < 0 )  perror("write NE");
        // close(fd);
         free(path);  // Clean up memory  ==========
         return;
      }
   }

   int sz = lseek(fd,0,SEEK_END); // Page size
   if(sz<0) perror("lseek:");
   lseek(fd,0,SEEK_SET);

   pthread_mutex_lock(&q_mutex);
   PAGES++;
   BYTES += sz;
   pthread_mutex_unlock(&q_mutex);

   char rep[1024];

   sprintf(rep,"HTTP/1.1 200 OK\r\n"
   "Date: Mon, 27 May 2018 12:28:53 GMT\r\n"
   "Server: myhttpd/1.0.0 (Ubuntu64)\r\n"
   "Content-Length: %d\r\n"
   "Content-Type: text/html\r\n"
   "Connection: Closed\r\n"
   "\r\n\r\n",sz);

   /* Write header to socket */
   write(sock,rep,strlen(rep));
   /* Write page Content to socket, read from file, write to socket */
   while((n=read(fd,buf,1023)) > 0){
      if(n==0) break;
      if ( write(sock,buf,1023) < n) perror("bad write");
   }
   free(path);
   close(fd);
}


void serve_request(int sock){
   char buf[1024],url[512],method[10],ver[32];
   int n,flag=0;

   while((n=read(sock,buf,1023))>0){
      buf[n] = '\0';
      if(strstr(buf,"\r\n") != NULL && !flag){
         sscanf(buf,"%s %s %s",method,url,ver);
         flag = 1;
      }
      if(n<1023) break;
   }
   send_site(sock,url);
}

/* The starting point of threads */
void *serve_th(){
   int sock;

   while(1){
      pthread_mutex_lock(&q_mutex);

      while(count < 1){
        if(EXIT==1) {
          printf("================");
          pthread_mutex_unlock(&q_mutex);
          pthread_cond_signal(&cv);
          pthread_exit(NULL);
        }
        pthread_cond_wait(&cv, &q_mutex);
        printf("EXI=%d\n",EXIT);
        if(EXIT==1) {
          printf("================");
          pthread_mutex_unlock(&q_mutex);
          pthread_cond_signal(&cv);
          pthread_exit(NULL);
        }
      }
      Remove(&fds,&sock,0);
      count--;
      pthread_mutex_unlock(&q_mutex);



      serve_request(sock);
      close(sock);
   }
   /* do stuff with sock */

   return NULL;

}

/* Nice format for date in STATS*/
char *conv(long n){
   char *buf = malloc(128*sizeof(char));
   if(n>3600){
		int min = n/60;
		int sec = n%60;
		int hr = min/60;
		min = min%60;
		sprintf(buf,"%d hours %d mins %d secs",hr,min,sec);
	}
	else{
		int min = n/60;
		int sec = n%60;
		sprintf(buf,"%d mins %d secs",min,sec);
	}
   return buf;
}


void stats(int sock){
   printf("---> STATS <---\n");
   time_t cur = time(NULL);
   char* a=conv(difftime(cur,start));
   char rep[2048];
   memset(rep,'\0',2048);
   sprintf(rep,"server up for %s, served %d pages and %d bytes\n",a,PAGES,BYTES );
   if(write(sock,rep,strlen(rep)) < 0) perror("write");
   free(a);
}
void shutd(int sock){
   printf("---> Server is shutting down! <---\nIn 3\n");
   char msg[256];
   strcpy(msg,"---> Server is shutting down! <---\n");
   if(write(sock,msg,strlen(msg)) < 0) perror("write");
   sleep(1);
   printf("2\n");
   sleep(1);
   printf("1\n");
   sleep(1);
   printf("GoodBye Mr/Mrs\n");
   EXIT = 1;


}

void server_command(int sock){
   char buf[32];
   int n;
   for(;;){
     if((n=read(sock,buf,31)) < 0) perror_exit("read");
     if(n==0) break;
     buf[n-2] = '\0'; // n-2 bcs of \r\n

     if(!strcmp(buf,"STATS")) stats(sock);
     else if(!strcmp(buf,"SHUTDOWN")) {
        shutd(sock);
        break;
   }
}
}

void sigchld_handler (int sig) {
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

void perror_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int mygetopt(int argc, char * const argv[], int *port,int *portc,int *threads,char **root_dir){
   int opt;
   char *file;
   while((opt = getopt(argc,argv,":d:c:p:t:") )!= -1)   {
      switch (opt) {
         case 'd':
         file = (char*) malloc((strlen(optarg)+1)*sizeof(char));
            strcpy(file,optarg);
            printf("root_dir:%s\n", file);
            break;

         case 'c':
            *portc=atoi(optarg);
            printf("portc=%d\n",*portc);
            break;
         case 'p':
            *port = atoi(optarg);
            printf("port=%d\n",*port);
            break;

         case 't':
          *threads = atoi(optarg);
          printf("th_num=%d\n",*threads );
          break;
         }
   }
   *root_dir= file;
   return 0;

}
