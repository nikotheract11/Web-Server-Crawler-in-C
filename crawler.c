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
#include "/home/nikos/jobExecutor/tests/sdi1500076/trie.h"


struct arg {
   int sock;
   struct sockaddr *serverptr;
   size_t len;
};


Queue urls;
pthread_mutex_t  q_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv;
int counter;
time_t start;
int PAGES;
int BYTES;
int EXIT;

void *thread_r();
void analyze_site(char *file);
void server_command(int sock);

void perror_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void create_threads(int thread_counter, pthread_t *threads,struct arg *args){
   for(int i = 0; i < thread_counter; i++) {
        if(pthread_create(&(threads[i]), NULL,thread_r, (void*)args) != 0) {
         //   return NULL;
         }
      }
}
int main(int argc, char const *argv[]) {
   int port, sock_c,c_port;
   pthread_cond_init(&cv, NULL); 
   pthread_mutex_lock(&q_mutex);
   InitializeQueue(&urls); 
   pthread_mutex_unlock(&q_mutex);
   signal(SIGPIPE, SIG_IGN);


   struct sockaddr_in server,server2,client;
   struct sockaddr *serverptr = (struct sockaddr*)&server;
   struct sockaddr *serverptr2=(struct sockaddr *)&server2;
   socklen_t clientlen;
   struct sockaddr *clientptr=(struct sockaddr *)&client;

   struct hostent *rem;
   int thread_counter = 3;  // atoi(argv[1])
   pthread_t *threads = malloc(thread_counter*sizeof(pthread_t));

   if (argc <= 3) {
     printf("Please give host name and port number\n");
        exit(1);}
  /* Create socket */
  /* Find server address */
   if ((rem = gethostbyname(argv[1])) == NULL) {
     herror("gethostbyname"); exit(1);
   }
   if((sock_c = socket(AF_INET,SOCK_STREAM,0)) < 0)
     perror_exit("sock_c");

   int enable = 1;
    if (setsockopt(sock_c, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

   port = atoi(argv[2]); /*Convert port number to integer*/
   c_port = atoi(argv[4]);
   server.sin_family = AF_INET;       /* Internet domain */
   memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
   server.sin_port = htons(port);         /* Server port */

   /* A struct to store infos about the connection */
   struct arg *args = malloc(sizeof(struct arg)) ;
   //args->sock = sock;
   args->serverptr = serverptr;
   args->len = sizeof(server);

   /* Create thread_pool */
   create_threads(3,threads,args);

   /* insert starting url to the queue */
   pthread_mutex_lock(&q_mutex);
   printf("Start crawling from site: %s\n",argv[3] );
   Insert((char*)argv[3],&urls,1,strlen(argv[3])+1);
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
   close(sock_c);
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
   char *root = "/home/nikos/Web-Server-Crawler-in-C/scr";    //======== DYNAMIC 

   char *dir = malloc(strlen(tok[i-2])+ strlen(root)+1+2);
   sprintf(dir,"%s/%s/",root,tok[i-2]);
   char *file = malloc(strlen(tok[i-1])+1);
   strcpy(file,tok[i-1]);

   DIR *d = opendir(dir);
   if(d == NULL && errno == ENOENT) {
    if(mkdir(dir,0700)<0) perror("mkdir");
  }
   else closedir(d);

   char *filename = malloc(strlen(dir)+strlen(file)+200);      // =====================
   sprintf(filename,"%s/%s",dir,file);
   FILE *fp = fopen(filename,"w");
   if(fp < 0 ) perror("cannot open write file: ");

   sprintf(req,"GET %s HTTP/1.0\r\n"
   "Date: Mon, 27 May 2018 12:28:53 GMT\r\n"
   "Server: myhttpd/1.0.0 (Ubuntu64)\r\n"
   "Content-Length: 5024\r\n"
   "Content-Type: text/html\r\n"
   "Connection: Closed\r\n""\r\n\r\n",url);
   if (write(sock, req, strlen(req)) < 0)
      perror_exit("write");

  if ((n = read(sock, req, 1023)) > 0){
          req[n] = '\0';
          if((str = strstr(req,"Content-Length:")) == NULL) perror_exit("strstr");
          sscanf(str+strlen("Content-Length:"),"%d",&len);
          str = strstr(req,"\r\n\r\n");
   } else perror_exit("read");
   if(str != NULL && str != req){
      a =(long)req + (long) n - (long) str - (long)strlen("\r\n\r\n") + 1;
      long bb = (long) str + strlen("\r\n\r\n")+2;
      fputs((char*)bb,fp);
   }
   char *buf = malloc(len);
   if(buf == NULL) perror("malloc failed");
   n=0;
   int k;
   if (fp < 0 ) perror("wtf?");
   while((k=read(sock,buf,500)) > 0 ) {
      n+=k;
      if(n >= len - a){
         fwrite(buf,1,k - (n-len + a)+3,fp);
         break;
      }
      memset(buf+k,'\0',500);
      if(fwrite(buf,1,k,fp) < k) break;

   }
   fclose(fp);
   analyze_site(filename);
   return NULL;
}

void *thread_r(struct arg *args){

   int flag=0,sock;
   struct sockaddr *serverptr ;
   size_t len;

   if((sock = socket(AF_INET,SOCK_STREAM,0)) < 0) //===============
     perror_exit("sock_c");
   serverptr = args->serverptr;
   len = args->len;

   char buf[2024];
   while(1){
      pthread_mutex_lock(&q_mutex);
      while(counter < 1){
         pthread_cond_wait(&cv, &q_mutex);
      }
      Remove(&urls,buf,1);
      counter--;
      pthread_mutex_unlock(&q_mutex);

      if((sock = socket(AF_INET,SOCK_STREAM,0)) < 0)
        perror_exit("sock_c");

      if (connect(sock, serverptr, len) < 0)
  	      perror_exit("connect");
      request(sock,buf);

      close(sock);

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
      while (s[a-1] == ' ') s[--a] = '\0';
      while (s[0] == ' ') s = (char *) ((long) s + 1);
      


      pthread_mutex_lock(&q_mutex);
     /* p_list *p = find(s);
      if(p==NULL){
         Insert(s,&urls,1,strlen(s));
         insert(s,0);
         counter++;
      }*/
      struct stat buffer;
      char *kl = malloc(1024);
      sprintf(kl,"%s%s","./scr",s);
      int exist = stat(kl,&buffer);
      if(search_q(urls,s) == 0 && exist < 0){
        Insert(s,&urls,1,strlen(s));
        counter++;
      }
      pthread_mutex_unlock(&q_mutex);
      pthread_cond_signal(&cv);

   }
   close(fd);
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


void stats(){
   printf("---> STATS <---\n");
   time_t cur = time(NULL);
   char* a=conv(difftime(cur,start));
   printf("server up for %s, served %d pages and %d bytes\n",a,PAGES,BYTES );
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

   if(!strcmp(buf,"STATS")) stats();
   else if(!strcmp(buf,"SHUTDOWN")) {
      shutd();
      break;
   }

   /* Just this changed || */
   /*                   \/ */
   else {
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
