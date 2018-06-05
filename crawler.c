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
#include "./jobEx_forCrawler/trie.h"
#include <dirent.h>


struct arg {
   struct sockaddr *serverptr;
   size_t len;
};


Queue urls;
pthread_mutex_t  q_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv;
int counter;
int active_threads;

time_t start;
int PAGES;
int BYTES;
int EXIT;

char *root_dir;

void *thread_r();
void analyze_site(char *file);
void server_command(int sock);
int mygetopt2(int argc, char * const argv[], int *port,int *portc,int *threads,char **root_dir,char **host,char **s_url);

void perror_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void create_threads(int thread_counter, pthread_t *threads,struct arg *args){
   for(int i = 0; i < thread_counter; i++) {
        if(pthread_create(&(threads[i]), NULL,thread_r, (void*)args) != 0) {
         printf("cant create threads\n");
         }
      }
}
int main(int argc, char * const argv[]) {
   int port, sock_c,c_port,thread_counter=0;
   char *host,*s_url;
   start = time(NULL);

   mygetopt2(argc,argv,&port,&c_port,&thread_counter,&root_dir,&host,&s_url);

   pthread_cond_init(&cv, NULL);
   pthread_mutex_lock(&q_mutex);
   InitializeQueue(&urls);
   pthread_mutex_unlock(&q_mutex);
   signal(SIGPIPE, SIG_IGN);


   struct sockaddr_in server,server2,client;
   struct sockaddr *serverptr = (struct sockaddr*)&server;
   struct sockaddr *serverptr2=(struct sockaddr *)&server2;
   socklen_t clientlen=0;
   struct sockaddr *clientptr=(struct sockaddr *)&client;

   struct hostent *rem;
   pthread_t *threads = malloc(thread_counter*sizeof(pthread_t));

   if (argc <= 3) {
     printf("Please give host name and port number\n");
        exit(1);
     }
  /* Find server address */
   if ((rem = gethostbyname(host)) == NULL) {
     herror("gethostbyname"); exit(1);
   }
   free(host);
   if((sock_c = socket(AF_INET,SOCK_STREAM,0)) < 0)
     perror_exit("sock_c");

   int enable = 1;
    if (setsockopt(sock_c, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

   server.sin_family = AF_INET;       /* Internet domain */
   memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
   server.sin_port = htons(port);         /* Server port */

   /* A struct to store infos about the connection */
   struct arg args;
   args.serverptr = serverptr;
   args.len = sizeof(server);

   /* Create thread_pool */
   create_threads(thread_counter,threads,&args);

   /* insert starting url to the queue */
   pthread_mutex_lock(&q_mutex);
   printf("Start crawling from site: %s\n",s_url );
   Insert((char*)s_url,&urls,1,strlen(s_url)+1);
   counter++;
   pthread_mutex_unlock(&q_mutex);
   pthread_cond_signal(&cv);


   server2.sin_family = AF_INET;       /* Internet domain */
   server2.sin_addr.s_addr = htonl(INADDR_ANY);
   server2.sin_port = htons(c_port);

   if((sock_c = socket(AF_INET,SOCK_STREAM,0)) < 0)
     perror_exit("sock_c");
   if(bind(sock_c,serverptr2,sizeof(server2))<0) perror_exit("bind");
   if(listen(sock_c, 5) < 0) perror_exit("listen");
   int newsock_c;
   printf("sock_c=%d,c_port=%d\n",sock_c,c_port );

   while(1){
       if((newsock_c = accept(sock_c,clientptr,&clientlen)) < 0)
        perror_exit("accept");
      server_command(newsock_c);
      close(newsock_c);
      if(EXIT == 1) break;
    }

   int *ret;
   pthread_cond_signal(&cv);
   for(int i=0;i<thread_counter;i++){
     pthread_join(threads[i],(void**)&ret);
   }
   close(sock_c);
   free(root_dir);
   free(s_url);
   free(threads);
   return 0;
}

void *request(int sock, char *url){
   char req[1024],*str,*token,url2[1024],*tok[10];
   long a=0;
   int n,len;
   int i=0;
   strcpy(url2,url);
   /* Analyze url : get the first token */
   char *del="/";
   token = strtok(url2, del);

   /* walk through other tokens */
   while( token != NULL ) {
      tok[i] = malloc(strlen(token)+1);
      strcpy(tok[i++],token);    // store tokens here
      token = strtok(NULL, del);
   }


   char *dir = malloc(strlen(tok[i-2])+ strlen(root_dir)+1+2);
   sprintf(dir,"%s/%s/",root_dir,tok[i-2]);
   char *file = malloc(strlen(tok[i-1])+1);
   strcpy(file,tok[i-1]);

   DIR *d = opendir(dir);
   if(d == NULL && errno == ENOENT) {
    if(mkdir(dir,0700)<0) perror("mkdir");
  }
   else closedir(d);

   char *filename = malloc(strlen(dir)+strlen(file)+2);
   sprintf(filename,"%s/%s",dir,file);
   FILE *fp = fopen(filename,"w");
   if(fp <= 0 ) perror("cannot open write file: ");

   char dt[1000];
   time_t now = time(0);
   struct tm tm = *gmtime(&now);
   strftime(dt, sizeof dt, "%a, %d %b %Y %H:%M:%S %Z", &tm);

   sprintf(req,"GET %s HTTP/1.0\r\n"
   "Date: %s\r\n"
   "Server: myhttpd/1.0.0 (Ubuntu64)\r\n"
   "Content-Length: 5024\r\n"
   "Content-Type: text/html\r\n"
   "Connection: Closed\r\n""\r\n\r\n",url,dt);
   if (write(sock, req, strlen(req)) < 0)
      perror_exit("write");

  if ((n = read(sock, req, 1023)) > 0){
          req[n] = '\0';
          if((str = strstr(req,"Content-Length:")) == NULL) perror_exit("strstr");
          sscanf(str+strlen("Content-Length:"),"%d",&len);

          /* Make sure that we have read the whole HEADER */
          while((str = strstr(req,"\r\n\r\n")) == NULL && n > 0) {
             n=read(sock,req,1023);
             req[n] = '\0';
          }
   } else perror_exit("read");

   /* If '\r\n\r\n' found, write the bytes which we read with the header to the file, without writing the header */
   if(str != NULL && str != req){
      a =(long)req + (long) n - (long) str - (long)strlen("\r\n\r\n") + 1;
      long bb = (long) str + strlen("\r\n\r\n")+2;
      fputs((char*)bb,fp);
   }
   char *buf = malloc(len);
   if(buf == NULL) perror("malloc failed");
   n=0;
   int k;
   while((k=read(sock,buf,500)) > 0 ) {
      n+=k;
      if(n >= len - a){
         fwrite(buf,1,k - (n-len + a)+3,fp);
         break;
      }
      memset(buf+k,'\0',500);    // fill with '\0'
      if(fwrite(buf,1,k,fp) < k) break;

   }
   fclose(fp);
   analyze_site(filename);
   free(dir);
   free(file);
   free(filename);
   free(buf);
   for(int k=0;k<i;k++) free(tok[k]);
   return NULL;
}

void *thread_r(struct arg *args){

   int sock;
   struct sockaddr *serverptr ;
   size_t len;

   serverptr = args->serverptr;
   len = args->len;

   char buf[2024];
   while(1){
      pthread_mutex_lock(&q_mutex);
      while(counter < 1){
         pthread_cond_wait(&cv, &q_mutex);
         if(EXIT == 1){
           pthread_mutex_unlock(&q_mutex);
           pthread_cond_signal(&cv);
           pthread_exit(NULL);
         }
      }
      Remove(&urls,buf,1);
      counter--;
      active_threads++;
      pthread_mutex_unlock(&q_mutex);

      if((sock = socket(AF_INET,SOCK_STREAM,0)) < 0)
        perror_exit("sock_c");

      if (connect(sock, serverptr, len) < 0)
  	      perror_exit("connect");


      request(sock,buf);
      close(sock);
      pthread_mutex_lock(&q_mutex);
      active_threads--;
      pthread_mutex_unlock(&q_mutex);

   }
   /* do stuff with sock */

   return NULL;

}

void analyze_site(char *file){
   int fd = open(file,O_RDONLY);
   char *str,*ptr;

   int sz = lseek(fd,0,SEEK_END);
   lseek(fd,0,SEEK_SET);
   pthread_mutex_lock(&q_mutex);
   BYTES += sz;
   pthread_mutex_unlock(&q_mutex);

   str = malloc(sz+1);
   ptr=str;
   read(fd,str,sz);
   long a;
   char *s;

   str[sz] = '\0';
   /* find links */
   while(1){
      s = strstr(str+1,"<a href=");    // find <a href
      if(s==NULL) break;      // <a href not found so break
      s = (char *) ((long) s + strlen("<a href="));   // if found, move pointer to the link
      str = strstr(s,">");    // find closing tag
      if(str==NULL) break;
      a = (long) str - (long) s; // a is ponter to the end of the link
      s[a] = '\0';
      while (s[a-1] == ' ') s[--a] = '\0'; // ignoring spaces
      while (s[0] == ' ') s = (char *) ((long) s + 1);   // ignoring spaces at start



      pthread_mutex_lock(&q_mutex);
      struct stat buffer;
      char *kl = malloc(1024);
      sprintf(kl,"%s%s",root_dir,s+1); // s+1 to ignore '/' bcs root_dir contains it
      int exist;
      if(search_q(urls,s) == 0 && (exist= stat(kl,&buffer)) < 0){    // if link not int the queue and not exists
        Insert(s,&urls,1,strlen(s));
        PAGES++;
        counter++;
      }
      pthread_mutex_unlock(&q_mutex);
      pthread_cond_signal(&cv);
      free(kl);

   }
   close(fd);
   free(ptr);
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
   sprintf(rep,"Crawler up for %s, downloaded %d pages and %d bytes\n",a,PAGES,BYTES );
   if(write(sock,rep,strlen(rep)) < 0) perror("write");
   free(a);
}
void shutd(int sock){
   printf("---> Crawler is shutting down! <---\nIn 3\n");
   char msg[256];
   strcpy(msg,"---> Crawler is shutting down! <---\n");
   if(write(sock,msg,strlen(msg)) < 0) perror("write");
   sleep(1);
   printf("2\n");
   sleep(1);
   printf("1\n");
   sleep(1);
   printf("GoodBye Mr/Mrs\n");

   pthread_mutex_lock(&q_mutex);
   EXIT = 1;
   pthread_mutex_unlock(&q_mutex);



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
      shutd(sock);
      break;
   }

   else {
      if(counter > 0 || active_threads > 0) {
         char msg[128];
         strcpy(msg,"Crawling not finished yet\n");
         write(sock,msg,strlen(msg));
         return;
      }
      char str[2048];
      strcpy(str,buf);
      int i = strlen(buf);
      int n=read(sock,str+i,2048-i);
      if(n<0) perror("read:");
      else {
        jexec(str,"./doc",sock);
      }

   }
 }
}


int mygetopt2(int argc, char * const argv[], int *port,int *portc,int *threads,char **root_dir,char **host,char **s_url){
   int opt;
   char *file;

   *s_url = malloc(strlen(argv[argc-1])+1);
   strcpy(*s_url,argv[argc-1]);

   while((opt = getopt(argc,argv,":d:c:p:t:h:") )!= -1)   {
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

         case 'h':
            *host = malloc(strlen(optarg)+1);
            strcpy(*host,optarg);
         }
   }
   *root_dir= file;
   return 0;

}
