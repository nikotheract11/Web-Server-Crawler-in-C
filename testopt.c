#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
int mygetopt(int argc, char * const argv[], int *port,int *portc,int *threads,char **root_dir){
   int opt,k;
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
            if(k<1){
               perror("K can't be negative or 0");
               return -1;
            }
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
  // *p=k;
   *root_dir= file;
   return 0;

}

int main(int a,char * const argv[]){
	int port,portc,threads;
	char *root_dir;
	mygetopt(a,argv,&port,&portc,&threads,&root_dir);
}