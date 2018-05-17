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

#include "./arrays/QueueInterface.h"

Queue fds;
pthread_mutex_t  q_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv;
int count;
time_t start;
int PAGES;
int BYTES;
int EXIT;



void child_server(int newsock);
void perror_exit(char *message);
void sigchld_handler (int sig);
void create_threads(int thread_count, pthread_t *threads);
void server_command(int sock);

int main(int argc, char *argv[]) {
   // time_t start = time(NULL);
    pthread_cond_init(&cv, NULL); 
    start = time(NULL);
    int             port, sock, newsock;
    struct sockaddr_in server, server2, client;
    socklen_t clientlen;
    int thread_count = 3;  // atoi(argv[1])
    pthread_t *threads = malloc(thread_count*sizeof(pthread_t));
    create_threads(thread_count,threads);

    struct sockaddr *serverptr=(struct sockaddr *)&server;
    struct sockaddr *clientptr=(struct sockaddr *)&client;
    struct sockaddr *serverptr2=(struct sockaddr *)&server2;
   // struct hostent *rem;


   /* Initialize Queue, dont forget to lock mutex */
    pthread_mutex_lock(&q_mutex);
    InitializeQueue(&fds); 
    pthread_mutex_unlock(&q_mutex);

    if (argc != 3) {
        printf("Please give port number\n");exit(1);}
    port = atoi(argv[1]);
    int c_port = atoi(argv[2]);
    int sock_c;
    /* Reap dead children asynchronously */
    signal(SIGPIPE, SIG_IGN);
    /* Create socket */
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
    if (listen(sock, 5) < 0) perror_exit("listen");
    if (listen(sock_c, 5) < 0) perror_exit("listen");

    struct pollfd clients[2];
    clients[0].fd = sock;
    clients[0].events = POLLRDNORM;

    clients[1].fd = sock_c;
    clients[1].events = POLLRDNORM;

    printf("Listening for connections to port %d\n", port);
    create_threads(thread_count,threads);
    while (1) {

        /* accept connection */
      poll(clients,2,-1);
      if (clients[0].revents & POLLRDNORM) {
         if ((newsock = accept(sock, clientptr, &clientlen)) < 0) perror_exit("accept");

      /* Insert socket to the queue so that a thread serves it */
         pthread_mutex_lock(&q_mutex);
         Insert(newsock,&fds);
         count++;
         pthread_mutex_unlock(&q_mutex);
         pthread_cond_signal(&cv);

    	    printf("Accepted connection\n");
   }
   if (clients[1].revents & POLLRDNORM) {
      int newsock_c;
      if((newsock_c = accept(sock_c,clientptr,&clientlen)) < 0) perror_exit("accept");
      server_command(newsock_c);
    //	close(newsock); /* parent closes socket to client            */
			/* must be closed before it gets re-assigned */
    }
    if(EXIT == 1) break;
}
free(threads);
}
void *serve_th();

void create_threads(int thread_count, pthread_t *threads){
   for(int i = 0; i < thread_count; i++) {
        if(pthread_create(&(threads[i]), NULL,serve_th, NULL) != 0) {
         //   return NULL;
         }
      }
}

void send_site(int sock,char *url){
   printf("inside send_site %s\n",url);
   char *root_dir = "/home/nikos/Web-Server-Crawler-in-C/www";
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
         return;
      }
   //   return 0;
   }
   int sz = lseek(fd,0,SEEK_END);
   if(sz<0) perror("lseek:");
   lseek(fd,0,SEEK_SET);

   pthread_mutex_lock(&q_mutex);
   PAGES++;
   BYTES += sz;
   pthread_mutex_unlock(&q_mutex);

   char rep[1024];/* = "HTTP/1.1 200 OK\r\n"
   "Date: Mon, 27 May 2018 12:28:53 GMT\r\n"
   "Server: myhttpd/1.0.0 (Ubuntu64)\r\n"
   "Content-Length: 2000\r\n"
   "Content-Type: text/html\r\n"
   "Connection: Closed\r\n"
   "\r\n\r\n";*/

   sprintf(rep,"HTTP/1.1 200 OK\r\n"
   "Date: Mon, 27 May 2018 12:28:53 GMT\r\n"
   "Server: myhttpd/1.0.0 (Ubuntu64)\r\n"
   "Content-Length: %d\r\n"
   "Content-Type: text/html\r\n"
   "Connection: Closed\r\n"
   "\r\n\r\n",sz);

   /* Write header to socket */
   write(sock,rep,strlen(rep));
   /* Write page Content to socket */
   while((n=read(fd,buf,1023)) > 0){
      if(n==0) break;
      if ( write(sock,buf,1023) < n) perror("bad write");
   }

//   return 0;
}


void serve_request(int sock){
   printf("inside serve_request\n");
   char buf[1024],url[512],method[10],ver[32];
   int n,flag=0;

   while((n=read(sock,buf,1023))>0){
      printf("n=%d\n",n );
      buf[n] = '\0';
      printf("%s\n",buf );
      if(strstr(buf,"\r\n") != NULL && !flag){
         printf("before scanf\n");
         sscanf(buf,"%s %s %s",method,url,ver);
         printf("after scanf\n");
         flag = 1;
      }
      if(n<1023) break;
   }
   printf("end serve\n");
   send_site(sock,url);

   //return 0;
}

void *serve_th(){
   int sock;
   printf("inside serve_th\n");
   pthread_mutex_lock(&q_mutex);
  // pthread_cond_signal(&cv);
   while(1){ 
      while(count < 1){
         pthread_cond_wait(&cv, &q_mutex);
      }
      Remove(&fds,&sock);
      count--;
      pthread_mutex_unlock(&q_mutex);


      serve_request(sock);
      close(sock);
   }
   /* do stuff with sock */

   return NULL;

}

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
void shutd(){
   printf("---> Server is shutting down! <---\nIn 3\n");
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
   if((n=read(sock,buf,31)) < 0) perror("read");
   if(n==0) break;
   buf[n-2] = '\0';

   if(!strcmp(buf,"STATS")) stats(sock);
   else if(!strcmp(buf,"SHUTDOWN")) {
      shutd();
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
