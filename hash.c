#include <stdio.h>
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

#include <dirent.h>
int main(int argc, char const *argv[]) {
 /*  int fd = open("/home/nikos/Web-Server-Crawler-in-C/site1/page1_2315.html",O_RDONLY);
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
   printf("%s\n",s);}
   close(fd);
   DIR *sd = opendir("./scr");
   if(sd == NULL) perror("open");
   char *d = "/home/nikos/Web-Server-Crawler-in-C/scr";
   char *f = "new";
   char gi[200];
   sprintf(gi,"%s/%s",d,f);
   printf("%s\n",gi );
   if(mkdir(gi,0777)<0) perror("mkdir");
   char p[200];
   sprintf(p,"%s/file",gi);
   fopen(p,"w");*/
	int a;
   if((a=open(argv[1],O_RDONLY))<0) printf("NIKOS OK\n");
   else {
	   printf("NIKOS BAD\n");
	   close(a);
   }
   return 0;
}
