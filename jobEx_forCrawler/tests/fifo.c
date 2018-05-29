#include  <sys/types.h>
#include  <sys/stat.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/poll.h>
#include <time.h>
#include "./trie.h"

#define MAXBUFF 1024
#define FIFO1   "/tmp/fifo.1"
#define FIFO2   "/tmp/fifo.2"
#define PERMS   0666



/* Insert list node with a path for jobExecutor */
int insertAtStart(char *p,list **l){
	if(*l == NULL){
		(*l) = malloc(sizeof(list));
		(*l)->path = malloc(strlen(p)+1);
		strcpy((*l)->path,p);
		(*l)->next = NULL;
		return 0;
	}
	list *tmp = *l;
	list *new = malloc(sizeof(list));
	new->path = malloc((strlen(p)+1));
	strcpy(new->path,p);
	new->next = tmp;
	*l = new;
	return 0;
}

void handler(int signum){
	exited = 1;
}

int jobExecutor(int *wr,int *rfd,int w,pid_t *workers,char *docfile){
	int fd,n;
	char str[50],c;
	int counter=0,j=0;

	list **paths=NULL	;		// a list whith paths for each worker
	paths = malloc(w*sizeof(list*));
	for(int i=0;i<w;i++) paths[i] = NULL;
	if(paths == NULL) return -1;

//	sleep(2);
	if((fd = open(docfile,0)) < 0) perror("open");

	struct pollfd *pfds;
	pfds = malloc(w*sizeof(struct pollfd));
	for(int i=0;i<w;i++){
		pfds[i].fd = wr[i];
		pfds[i].events = POLLOUT|POLLIN;
	}

	poll(pfds,w,-1);

	/* Send paths to workers */
	while((n=read(fd,&c,1)) > 0){
		if(c == '\n') {
			str[j] = '\0';
			if((pfds[counter%w].revents & POLLOUT) && write(wr[counter%w],str,j+1)<=0) perror("write error"); 		// j anti j-1?
			insertAtStart(str,&paths[counter++%w]);
			j=0;
		}
		else str[j++] = c;
		poll(pfds,w,-1);
	}
	sleep(1);

	/* All paths have been sent, now send STOP to let them know */
	poll(pfds,w,-1);
	for(int i=0;i<w;i++){
		if(pfds[i].revents & POLLOUT) write(wr[i],"stop",strlen("stop")+1);
		poll(pfds,w,-1);
	}
	sleep(1);
	/* Here comes the user interface */
	interface(wr,rfd,w,paths,workers);

	return 0;
}



int worker(int rfd,int wfd){
	int n;
	char buff[1024];
	char s[32];
	char **dirs;	// Array with the paths
	int d_sz = 16;	// Size of dirs, if grows more than 16 paths, realloc
	int d_num=0; 	// Total dirs
	time_t arrival;
	sprintf(s,"./log/worker%d",getpid());
	fp = fopen(s,"a");

	dirs = malloc(d_sz*sizeof(char*));

	while((n=read(rfd,buff,MAXBUFF)) > 0){
		/* Here open files, create map and trie */
		if(d_num > d_sz){
			d_sz *= 2;
			dirs = realloc(dirs,d_sz);
			if(dirs == NULL) perror("realloc failed");
		}

		if(!strcmp(buff,"stop")) break;
		dirs[d_num] = malloc(strlen(buff)+1);
		strcpy(dirs[d_num++],buff);
		printf("file:%s\n",dirs[d_num-1]);
	}

	init(dirs,d_num);		// Initialize structures like trie and map
	printf("OK\n");

	/* Now, that files are read, read the query */
	while(1){
		/* Tokenize buffer, read the first token to execute the query */
		arrival = time(NULL);
		char * c_time = ctime(&arrival);
		char *query= readMsg(&rfd,1,-1,c_time);
		if(query == NULL) return 0;
		sendMsg(query,&wfd,1);
	}
	fclose(fp);

	return 0;
}


int main(int argc, char * const argv[]) {
	int *readfd,*writefd,w;
	pid_t p,workers[5];
	char *docfile;

	mkdir("./log",0777);

	mygetopt(argc,argv,&w,&docfile);
	readfd = malloc(w*sizeof(int));
	writefd = malloc(w*sizeof(int));


	for(int i=0;i<w;i++){
		char fifo_1[32],fifo_2[32];

		/*===== Create fifos  =====*/
		sprintf(fifo_1,"./fifo1_%d",i);
		sprintf(fifo_2,"./fifo2_%d",i);

		if((mkfifo(fifo_1,PERMS) < 0) && (errno != EEXIST)){
			perror("cant create fifo");
		}
		if((mkfifo(fifo_2,PERMS) < 0) && (errno != EEXIST)){
			perror("cant create fifo");
		}
	}
	/*======= fork() w Workers =======*/
	int read_fd,write_fd;
	for(int i=0;i<w;i++){
		char fifo_1[32],fifo_2[32];

		p = fork();
		sprintf(fifo_1,"./fifo1_%d",i);
		sprintf(fifo_2,"./fifo2_%d",i);
		if(p == 0) {
			/*======	Open fifos for workers ======*/
			if((write_fd = open(fifo_1,1)) < 0){
				perror("worker:cant open write");
			}
			if((read_fd = open(fifo_2,0)) < 0){
				perror("worker: cant open read fifo");
			}
			break;
		}
		else if(p > 0){
			/*	Open fifo for jobExecutor */
			workers[i]=p;
			if ( (readfd[i] = open(fifo_1, 0|O_NONBLOCK))  < 0)  {
				perror("server: can't open read fifo");
			}
			if ( (writefd[i] = open(fifo_2, 1))  < 0)  {
				perror("server: can't open write fifo");
			}
		}
		else {
			perror("fork:failed");
		}
	}
	if(p==0) {
		worker(read_fd,write_fd);
		close(write_fd);
		close(read_fd);
		return 0;
	}
	else {
		jobExecutor(writefd,readfd,w,workers,docfile);
		for(int i=0;i<w;i++){
			char fifo_1[32],fifo_2[32];
		//	int status;

			sprintf(fifo_1,"./fifo1_%d",i);
			sprintf(fifo_2,"./fifo2_%d",i);
		//	waitpid(workers[i],&status,0);
			close(writefd[i]);
			close(readfd[i]);
			unlink(fifo_1);
			unlink(fifo_2);
		}
	}

	return 0;
}
