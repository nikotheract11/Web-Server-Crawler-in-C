#include <sys/stat.h>
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
#include <dirent.h>
#include "/home/nikos/minisearch/trie.h"


struct arg {
   int sock;
   struct sockaddr *serverptr;
   size_t len;
};


Queue urls;
pthread_mutex_t  q_mutex = PTHREAD_MUTEX_INITIALIZER;
time_t start;
int PAGES;
int BYTES;
int EXIT;

void *thread_r();
void analyze_site(char *file);

void perror_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void create_threads(int thread_count, pthread_t *threads,struct arg *args){
   for(int i = 0; i < thread_count; i++) {
        if(pthread_create(&(threads[i]), NULL,thread_r, (void*)args) != 0) {
         //   return NULL;
         }
      }
}
int main(int argc, char const *argv[]) {
   int             port, sock;

   struct sockaddr_in server,server2;
   struct sockaddr *serverptr = (struct sockaddr*)&server;
   struct sockaddr *serverptr2=(struct sockaddr *)&server2;

   struct hostent *rem;
   int thread_count = 3;  // atoi(argv[1])
   pthread_t *threads = malloc(thread_count*sizeof(pthread_t));

   if (argc <= 3) {
     printf("Please give host name and port number\n");
        exit(1);}
  /* Create socket */
   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
     perror_exit("socket");
  /* Find server address */
   if ((rem = gethostbyname(argv[1])) == NULL) {
     herror("gethostbyname"); exit(1);
   }
   /*if((sock_c = socket(AF_INET,SOCK_STREAM,0)) < 0)
     perror_exit("sock_c");*/

   port = atoi(argv[2]); /*Convert port number to integer*/
   server.sin_family = AF_INET;       /* Internet domain */
   memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
   server.sin_port = htons(port);         /* Server port */

   /* A struct to store infos about the connection */
   struct arg *args = malloc(sizeof(struct arg)) ;
   args->sock = sock;
   args->serverptr = serverptr;
   args->len = sizeof(server);

   /* Create thread_pool */
   create_threads(3,threads,args);

   /* insert starting url to the queue */
   pthread_mutex_lock(&q_mutex);
   Insert((char*)argv[3],&urls,1,strlen(argv[3]));
   pthread_mutex_unlock(&q_mutex);

   server2.sin_family = AF_INET;       /* Internet domain */
   server2.sin_addr.s_addr = htonl(INADDR_ANY);
   server2.sin_port = htons(c_port);

   if(bind(sock_c,serverptr2,sizeof(server2))<0)
       perror_exit("bind");
   if (listen(sock_c, 5) < 0) perror_exit("listen");
   int newsock_c;
   struct pollfd clients[1];
   clients[0].fd = sock_c;
   clients[0].events = POLLRDNORM;


   while(1){
     poll(clients,1,-1);
     if (clients[0].revents & POLLRDNORM) {
       if((newsock_c = accept(sock_c,clientptr,&clientlen)) < 0) perror_exit("accept");
          serve_command(newsock_c);
      }
      if(EXIT == 1) break;
    }
   return 0;
}

void *request(int sock, char *url){
   char req[1024],*str,*token,url2[1024],*tok[10];
   long a;
   int n,len;
   int i=0;
   strcpy(url2,url);
   /* Analyze url : get the first token */
   token = strtok(url2, "/");

   /* walk through other tokens */
   while( token != NULL ) {
      tok[i] = malloc(strlen(token)+1);
      strcpy(tok[i++],token);    // store tokens here
      token = strtok(NULL, "/");
   }

   char *dir = malloc(strlen(tok[i-2])+1);
   strcpy(dir,tok[i-2]);
   char *file = malloc(strlen(tok[i-1])+1);
   strcpy(file,tok[i-1]);

   DIR *d = opendir(dir);
   if(d == NULL && errno == ENOENT) mkdir(dir,0777);
   else closedir(d);

   char *filename = malloc(strlen(dir)+strlen(file)+2);
   sprintf(filename,"%s/%s",dir,file);
   FILE *fp = fopen(filename,"a");

   sprintf(req,"GET %s HTTP/1.0\r\n""\r\n\r\n",url);
   if (write(sock, req, strlen(req)) < 0)
      perror_exit("write");

   bzero(req,1024);
   if ((n = read(sock, req, 1023)) > 0){
          req[n] = '\0';
          if((str = strstr(req,"Content-Length:")) == NULL) perror_exit("strstr");
          sscanf(str+strlen("Content-Length:"),"%d",&len);
          str = strstr(req,"\r\n\r\n");
   } else perror_exit("read");
   if(str != NULL && str != req){
      a = (long) str + strlen("\r\n\r\n") - (long) req ;
      printf("%ld\n", a);
   }
   char *buf = malloc(len - a);
   read(sock,buf,len-a);
   fputs(buf,fp);
   analyze_site(filename);

   return NULL;
}

void *thread_r(struct arg *args){

   printf("inside serve_th\n");
   int flag=0,sock;
   struct sockaddr *serverptr ;
   size_t len;

   sock = args->sock;
   if((sock = socket(AF_INET,SOCK_STREAM,0)) < 0)
     perror_exit("sock_c");
   serverptr = args->serverptr;
   len = args->len;

   char buf[1024];
   while(1){
      while(!flag){
         pthread_mutex_lock(&q_mutex);
         flag = Remove(&urls,buf,1);
         pthread_mutex_unlock(&q_mutex);
      }
      if (connect(sock, serverptr, len) < 0)
  	      perror_exit("connect");
      request(sock,buf);

      flag=0;
   }
   /* do stuff with sock */

   return NULL;

}

void analyze_site(char *file){
   int fd = open(file,O_RDONLY);
   char *str;
   int sz = lseek(fd,0,SEEK_END);
   lseek(fd,0,SEEK_SET);
   str = malloc(sz+1);
   read(fd,str,sz);
   long a;
   char *s;

   str[sz] = '\0';
   while(1){
      s = strstr(str+1,"<a href=");
      if(s==NULL) break;
      s = (char *) ((long) s + strlen("<a href="));
      str = strstr(s,">");
      if(str==NULL) break;
      a = (long) str - (long) s;
      s[a] = '\0';
      printf("%s\n",s);

      pthread_mutex_lock(&q_mutex);
      p_list *p = find(s);
      if(p==NULL){
         Insert(s,&urls,1,strlen(s));
         insert(s,0);
      }
      pthread_mutex_unlock(&q_mutex);
   }
   close(fd);
}
