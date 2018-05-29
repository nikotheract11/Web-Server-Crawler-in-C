#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "./trie.h"
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>


/*
 * Returns tokens and their #
 */
char **getok(char* ss,int *k){
	char **st;
	int count=0;
	char *token;
	for(unsigned int i=0;i<=strlen(ss);i++){
		if(ss[i] == ' ' || ss[i] == '\0') count++;
	}
	char *s = malloc(strlen(ss)+1);
	strcpy(s,ss);
	st = (char**) malloc(count*sizeof(char*));

	token = strtok(s," ");
	int j=0;
	while(token != NULL) {
		st[j] = (char*) malloc(sizeof(char)*(strlen(token)+1));
		strcpy(st[j++],token);
		token = strtok(NULL," ");
		if(j>11) break;
	}
	*k=j;
	free(token);
	return st;

}

int writeLog(char *op,char *keyword,char *path,char *t){
	t[strlen(t)-1]=' ';
	fprintf(fp, "%s : %s : %s : %s\n",t,op,keyword,path );
	return 0;
}
/*
 * Searching on trie, implementation of /search feature
 */
char * search(char** s,int j,char *date){
	if(j < 4) {
		printf("Wrong arguments for /search\n");
		return NULL;
	}

	char *reply;
	int sz=512;  // first, allocate 512 bytes for reply, realloc if needed
	int total=0;   // total bytes to be written

	reply = malloc(sz*sizeof(char));
	memset(reply,'\0',sz);


	for(int i=1;i<j-2;i++){
		p_list *p = find(s[i]);    // p_list of this word if exists
		if(p == NULL) continue;       // if not exists break

		while(p != NULL) {
			total += sizeof(int) + strlen(files[p->text_id]) \
				 + strlen(str[p->text_id][p->line]) +5; // Bcs reply is Path Line# Line_Content  , +4 bcs of ":",'\n','\0'
			if(total >= sz) {
				sz *= 2;    // Double size
				if((reply=realloc(reply,sz)) == NULL) perror("realloc failed");
			}

			sprintf(reply,"%s%s:%d:%s\n",reply,files[p->text_id],p->line,str[p->text_id][p->line]);

			/* Keep log */
			writeLog("search",s[i],files[p->text_id],date);
			p = p->next;
		}
	}

	char *msg = malloc(strlen(reply)+sizeof(int));
	int szi = strlen(reply);
	memcpy(msg,&szi,sizeof(int));
	memcpy(&(msg[sizeof(int)]),reply,szi);
	return msg;
}

char *maxcount(char **s,int j,char *date){
	if(j < 2) {
		printf("Wrong arguments for /search\n");
		return NULL;
	}
	pair min;
	for(int i=1;i<j;i++){
		p_list *p = find(s[i]);    // p_list of this word if exists
		if(p == NULL) break;       // if not exists break

		pair **Pair = NULL;
		Pair=malloc(p->plen*sizeof(pair*));
		for(int i=0;i<p->plen;i++) Pair[i] = NULL;
		int l=0,pp=0;
		while(p != NULL){
			for(int h=0;h<l;h++)
				if(!strcmp(Pair[h]->path,files[p->text_id])) {
					pp =h;
					break;
				}
			if(pp>0) {
				Pair[pp]->score++;
				pp=0;
			}
			else{
			Pair[l] = malloc(sizeof(pair));
			strcpy(Pair[l]->path,files[p->text_id]);
			Pair[l++]->score = 1;
		}

			p=p->next;
		}

		/* Find the min */
		min = *Pair[0];
		for(int k=1;k<l;k++){
			if(Pair[k]->score > min.score) min = *Pair[k];
		}
	}
	writeLog("maxcount",s[1],min.path,date);

	char *reply = malloc(sizeof(pair)+sizeof(int));
	memset(reply,0,sizeof(pair)+sizeof(int));
	int ll=sizeof(pair);

	memcpy(reply,&ll,sizeof(int));
	memcpy(&(reply[sizeof(int)]),&min,sizeof(pair));

	return reply;
}

char *mincount(char **s,int j,char *date,int op){
	if(j < 2) {
		printf("Wrong arguments for /search\n");
		return NULL;
	}
	pair min;
	for(int i=1;i<j;i++){
		p_list *p = find(s[i]);    // p_list of this word if exists
		if(p == NULL) break;       // if not exists break

		pair **Pair = NULL;
		Pair=malloc(p->plen*sizeof(pair*));
		for(int i=0;i<p->plen;i++) Pair[i] = NULL;
		int l=0,pp=0;
		while(p != NULL){
			for(int h=0;h<l;h++)
				if(!strcmp(Pair[h]->path,files[p->text_id])) {
					pp =h;
					break;
				}
			if(pp>0) {
				Pair[pp]->score++;
				pp=0;
			}
			else{
			Pair[l] = malloc(sizeof(pair));
			strcpy(Pair[l]->path,files[p->text_id]);
			Pair[l++]->score = 1;
		}

			p=p->next;
		}

		/* Find the min */
		min = *Pair[0];
		for(int k=1;k<l;k++){
			if((Pair[k]->score)*op <= (min.score)*op) min = *Pair[k];
		}
	}
	writeLog("mincount",s[1],min.path,date);

	char *reply = malloc(sizeof(pair)+sizeof(int));
	memset(reply,0,sizeof(pair)+sizeof(int));
	int ll=sizeof(pair);

	memcpy(reply,&ll,sizeof(int));
	memcpy(&(reply[sizeof(int)]),&min,sizeof(pair));

	return reply;
}

char *wc(char ***s){
	int i=0,bytes=0,words=0,lines=0;
	while(s[i] != NULL){
		int j=0;
		while(s[i][j] != NULL){
			lines++;
			bytes += strlen(s[i][j]);
			int tmp=0;
			getok(s[i][j],&tmp);
			words += tmp;
			j++;
		}
		i++;
	}
	char *reply = malloc(3*sizeof(int)+sizeof(int));

	int sz=3*sizeof(int);
	memcpy(reply,&sz,sizeof(int));
	memcpy(&(reply[sizeof(int)]),&bytes,sizeof(int));
	memcpy(&(reply[2*sizeof(int)]),&words,sizeof(int));
	memcpy((&reply[3*sizeof(int)]),&lines,sizeof(int));

	return reply;
}

/*
 * Calls the right function
 */
char * operate( char** s,int j,char *k){
	if(!strcmp(s[0],"/exit")) return NULL;
	if(!strcmp(s[0],"/search")) return search(s,j,k);
	else if(!strcmp(s[0],"/maxcount")) return maxcount(s,j,k);
	else if(!strcmp(s[0],"/mincount")) return mincount(s,j,k,1);
	else if(!strcmp(s[0],"/wc")) return wc(str);
	else {
		printf("Not such a command available\n");
		return NULL;
	}
}

int sendMsg(char *buf,int *wr,int w){
	struct pollfd *pfds;
	pfds = malloc(w*sizeof(struct pollfd));


	for(int i=0;i<w;i++){
		pfds[i].fd = wr[i];
		pfds[i].events = POLLOUT;
	}

	int sent,bytesToSend,n;

	/* first read how many bytes to send */
	memcpy(&bytesToSend,buf,sizeof(int));
	poll(pfds,w,-1);
	for(int i=0;i<w;i++){
		sent = sizeof(int);

		if(pfds[i].revents & POLLOUT){
			while((n=write(wr[i],(buf),bytesToSend+sizeof(int))) > 0){
				sent += n;     // Total bytes sent, dont stop until all bytes have been sent
				if(sent >= bytesToSend+2*sizeof(int)) break;
			}
		}
	}
	return 0;
}

int jsearch(char **s,int w){
	for(int i=0;i<w;i++){
		if(s[i]!=NULL) printf("%s\n",s[i] );
	}
	return 0;
}

int jmax(char **s,int w){
	pair *pairs,max;
	pairs = malloc(w*sizeof(pair));
	for(int i=0;i<w;i++){
		if(s[i]==NULL) continue;
		memcpy(&pairs[i],s[i],sizeof(pair));
		if(i==0) max = pairs[0];
		else
			if(pairs[i].score > max.score) max=pairs[i];
	}
	printf("Max: %s\n",max.path );
	return 0;
}

int jmin(char **s,int w,int op){

	pair *pairs,min;
	pairs = malloc(w*sizeof(pair));
	for(int i=0;i<w;i++){
		if(s[i] == NULL) continue;
		memcpy(&pairs[i],s[i],sizeof(pair));
		if(i==0) min = pairs[0];
		else
			if((pairs[i].score)*op < (min.score)*op) min=pairs[i];
	}

	printf("Min: %s\n",min.path );

	return 0;
}

int jwc(char **s,int w){
	int total_b=0,total_w=0,total_l=0;
	for(int i=0;i<w;i++){
		int bytes=0,words=0,lines=0;
		if(s[i]==NULL) continue;
		memcpy(&bytes,s[i],sizeof(int));
		memcpy(&words,&(s[i][sizeof(int)]),sizeof(int));
		memcpy(&lines,&(s[i][2*sizeof(int)]),sizeof(int));

		total_b += bytes;
		total_w += words;
		total_l += lines;
	}

	printf("b->%d w->%d l->%d\n",total_b,total_w,total_l );
	return 0;
}

void catcher(int signum) {
  return;
}


char * readMsg(int *rfd,int w,int d,char *op){
	struct pollfd *pfds;
	char **buf = malloc(w*sizeof(char*));
	int noReply=0;
	pfds = malloc(w*sizeof(struct pollfd));
	for(int i=0;i<w;i++){
		pfds[i].fd = rfd[i];
		pfds[i].events = POLLIN;
	}

	struct sigaction sigact;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigact.sa_handler = catcher;
	sigaction(SIGALRM, &sigact, NULL);

	/* d=-1 for workers */

   if(d>=0){
		alarm(d);
		pause();
	}
	poll(pfds,w,d);

	for(int i=0;i<w;i++){
		if(pfds[i].revents & POLLIN){
			int bytesToRead,bytesRead=0,k;
			if(read(rfd[i],&bytesToRead,sizeof(int)) < 0)
				perror("kokokdoakdoakdoak");

			buf[i] = malloc((bytesToRead+1)*sizeof(char));

			while(bytesRead < bytesToRead){
				if((	k = read(rfd[i],(buf[i]),bytesToRead))<0)
					perror("readMsg");
				else bytesRead += k;
			}
			buf[i][bytesToRead]='\0';
		}
		else {
			noReply++;
			buf[i]=NULL;
		}
	}


	if(d >= 0) printf("%d/%d workers replied\n",w-noReply,w );
	if( noReply == w) return NULL;

	/* op is NULL for workers */
	if(d==-1){
		for(int i=0;i<w;i++){
			if(buf[i]!=NULL) {
				int toks;
				char **tok = getok(buf[i],&toks);
				return operate(tok,toks,op);
			}
		}
		return 0;
	}
	if(!strcmp(op,"/search")) jsearch(buf,w);
	else if(!strcmp(op,"/maxcount")) jmax(buf,w);
	else if(!strcmp(op,"/mincount")) jmin(buf,w,1);
	else if(!strcmp(op,"/wc")) jwc(buf,w);
	return *buf;
}

/*int dead_w(list **paths,int w,pid_t *workers,int *writefd,int *readfd){
	pid_t p;
	int status;

	for(int i=0;i<w;i++){
		exited = 0;
		if((p=waitpid(workers[i],&status,WNOHANG)) > 0){
			p = fork();
			if(p == 0) {
				int write_fd,read_fd;
				char fifo_1[32],fifo_2[32];
				sprintf(fifo_1,"/tmp/fifo1_%d",i);
				sprintf(fifo_2,"/tmp/fifo2_%d",i);

				if((write_fd = open(fifo_1,1)) < 0){
					perror("worker:cant open write");
				}
				if((read_fd = open(fifo_2,0)) < 0){
					perror("worker: cant open read fifo");
				}
				worker(read_fd,write_fd);
				return 0;
			}
			else {
				workers[i] = p;
				char fifo_1[32],fifo_2[32];
				sprintf(fifo_1,"/tmp/fifo1_%d",i);
				sprintf(fifo_2,"/tmp/fifo2_%d",i);

				if ( (readfd[i] = open(fifo_1, 0|O_NONBLOCK))  < 0)  {
					perror("server: can't open read fifo");
				}
				if ( (writefd[i] = open(fifo_2, 1))  < 0)  {
					perror("server: can't open write fifo");
				}
				return p;
			}
		}
	}
	return 0;
}*/

/*
 * The Application interface
 */
int interface(int *wfd,int *rfd,int w,list **paths,pid_t *workers){
	char buf[256];

	while(1){
		printf("Type command: ");
//		scanf("%m[^\n]s\n",&buf );
		gets(buf);
		//getchar();

		int tok_num,d=1;
		char **tok = getok(buf,&tok_num);
		if(!strcmp(tok[0],"/search")) d = atoi(tok[tok_num-1]);

		/* Check for dead workers */
	/*	if(exited)
			if(dead_w(paths,w,workers,wfd,rfd) == 0) return 0;*/

		if(!strcmp(tok[0],"/exit")) return 0;
		char *msg = malloc(strlen(buf)+1+sizeof(int));
		int sz = strlen(buf);

		memcpy(msg,&sz,sizeof(int));
		memcpy(&msg[sizeof(int)],buf,strlen(buf));

		/* Send query to workers */
		sendMsg(msg,wfd,w);
		/* Print replies */
		readMsg(rfd,w,d,tok[0]);

//		free(buf);
	}
//	free(buf);
	return 0;
}
