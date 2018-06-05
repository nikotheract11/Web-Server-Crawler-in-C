#include  <stdio.h>
#include  <sys/types.h>
#include  <dirent.h>
#include <stdlib.h>
#include <string.h>
#include "./trie.h"

char **get(const char* file);
char ***map_(char **dirname,int dir_num,unsigned int *files_num,char ***);


int init(char **d,int d_n){
	//char *d="./ff";
	unsigned int fn;
	int inn;
	char *token,*tmp;
	str = map_(d,d_n,&fn,&files);
	int j=0;
	for(int i=0;i<fn;i++){	// for each file
		j=0;
		if(str[i] == NULL) break;
		while(str[i][j] != NULL){	// for each line
			tmp =(char *) malloc(sizeof(char)*(strlen(str[i][j])+1));
			strcpy(tmp,str[i][j]);
			token = strtok(tmp," ");
			while(token != NULL){
				if(token[strlen(token)-1] =='\n' || token[strlen(token)-1] ==' ') token[strlen(token)-1]='\0'; // ==
				if((inn=insert(token,i,j))<0) printf("ERROR %d\n", inn);
            token  = strtok(NULL," ");
			}
			j++;
         free(tmp);
         free(token);
		}
	}
	return 0;

}

/* map returns a map to texts */
char ***map_(char **dirname,int dir_num,unsigned int *files_num,char ***fs){
	DIR *dir_ptr;
	struct dirent *direntp;
	char ***map,**files;

	size_t sz=32;
	map = (char***) malloc(sz*sizeof(char**));     // 32 places for texts, if more -> realloc
	files = malloc(sz*sizeof(char*));
	unsigned int i=0;
	for(int j=0;j<dir_num;j++){

		/* open directory j */
		if ( ( dir_ptr = opendir(dirname[j])) == NULL )
			fprintf(stderr, "cannot open %s\n",dirname[j]);
		else {
			while ( ( direntp=readdir(dir_ptr) ) != NULL ){
				if (direntp->d_ino == 0 ) continue;

				/* reallocate space when sz texts reached */
				if(i >= sz) {
					sz *= 2;
					map = (char***) realloc(map,sz*sizeof(char ***));
					files = realloc(files,sz*sizeof(char*));
					if(map == NULL){
						perror("Realloc failed");
						return NULL;
					}
				}

				/* store each text to map */
				if(direntp-> d_type != DT_DIR) {   // ignore "." and ".."
					char *file = malloc(strlen(dirname[j])+strlen(direntp->d_name)+2);
					sprintf(file,"%s/%s",dirname[j],direntp->d_name);
					map[i]=get(file);
					files[i] = malloc(sizeof(char)*(strlen(file)+1));
					strcpy(files[i++],file);
				}
			}
			closedir(dir_ptr);
		}
	}
	map[i]=NULL;
	*files_num = i;
	*fs = files;
	return map;
}

	/*
	 * Return the # of texts
	 */
	int getLinesNumber(FILE *fp){
		char c;
		int count = 0;
		while(!feof(fp)){
			c = fgetc(fp);
			if(c == '\n') count++;
		}
		fseek(fp,0,0);
		return ++count;        // ++count becuase the last line has not a '\n' in the end
	}

	char **get(const char* file){

		FILE *fp;
		fp = fopen(file,"r");
		if(fp == NULL) {
			perror("GET file not open");
			return NULL;
		}
		int i=0,j=0;
		char c;
		char **str;

		int n = getLinesNumber(fp);
		str = (char**) malloc(n*sizeof(char*));
		for(int k=0;k<n;k++) str[k] = NULL;

		while(i++<n ){
			size_t a;
			getline(&(str[j]),&a,fp);

			if((c=fgetc(fp)) == EOF) break;
			else ungetc(c,fp);
			j++;
		}
		str[i] = NULL;
		fclose(fp);


		return str;
	}
