#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"

/*
 * create a p_list node
*/
int p_init(p_list ** p,int text_id,int line){
   *p = (p_list*) malloc(sizeof(p_list));
   (*p)->next = NULL;
   (*p)->text_id = text_id;
   (*p)->freq = 1;
   (*p)->line = line;
   (*p)->plen = 0;
   return 0;
}

/*
 * add a p_list node to trie
*/
int addplist(t_node **t,int text_id,int line){
   t_node *tmp = *t;

   // if this word, found for first time, plist = NULL
   if(tmp->plist == NULL) {
      p_init(&(tmp->plist),text_id,line);
      (tmp->plist->plen)++;
      return 0;
   }

   // else append to the end one more p_list node to plist list, or increase frequency
   else{
      p_list *p = tmp->plist;
      while (p != NULL){
         if(p->text_id == text_id && p->line == line) {     //=========== swsto?????
            (p->freq)++;
            return 0;
         }

         // if text_id not in the list
         if(p->next == NULL) {
            p_init(&(p->next),text_id,line);
            (tmp->plist->plen)++;
            break;
         }
         p = p->next;
      }
   }
   return 0;
}

/*
 * Initialize a t_node
*/
int t_init(t_node ** t){
   *t = (t_node*)  malloc(sizeof(t_node));
   (*t)->sibling = NULL;
   (*t)->child = NULL;
   (*t)->plist = NULL;
   (*t)->value = 0;
   return 0;
}

/*
 * This function appends a key to trie t
*/
int append(t_node **t,const char *key,int text_id,int line){
   unsigned int i=0;
   t_node *tmp = *t;
   while(i<(strlen(key)) ){
      if(key[i] == '\0') break;
      tmp->value = key[i];
      if(i == strlen(key)-1) break;
      t_init(&(tmp->child));
      tmp = tmp->child;

      i++;
   }

   //add a plist for the key
   addplist(&tmp,text_id,line);
   return 0;
}

/*
 * This functions inserts key with text_id to trie t
*/

int insert( const char* key, int text_id,int line){
   t_node *tmp = t;
   unsigned int i = 0;

   // If trie is empty, create one
   if(t == NULL) {
      t_init(&t);
      tmp = t;
      append(&tmp,key,text_id,line);
      return 22;
   }
   if(strlen(key) <= 0) return 0;

   // traversing trie
   for(i=0;i<strlen(key);i++){
      while( key[i] != tmp->value ){
         if(tmp->sibling == NULL) break;
         tmp = tmp->sibling;
      }

      // if key not in trie
      if(key[i] != tmp->value){
            t_init(&(tmp->sibling));
            tmp = tmp->sibling;
            append(&tmp,&(key[i]),text_id,line);
            return 3;
      }

      // key in trie, increase frequency
      if(i == (strlen(key) - 1) ) {
         addplist(&tmp,text_id,line);
         return 0;
      }

      // going down the trie
      if(tmp->child != NULL && i < (strlen(key) -1)) tmp = tmp->child;

      else {
         append(&tmp,&(key[i]),text_id,line);
         return 88;
      }
   }
   return -2;
}

/*
 * Print trie
*/
int pr_trie(t_node *t,char *buf,int i){
   t_node *tmp=t;
   if(t == NULL ) return -1;

   buf[i++] = tmp->value;
   if(tmp->plist != NULL) {
      buf[i]='\0';
      printf("%s %d\n",buf,tmp->plist->plen );
   }

   pr_trie(tmp->child,buf,i);
   pr_trie(tmp->sibling,buf,--i);

   return 0;
}

/*
 * Return the # of texts
*/
/*int getLinesNumber(FILE *fp){
	char c;
	int count = 0;
	while(!feof(fp)){
		c = fgetc(fp);
		if(c == '\n') count++;
	}
   fseek(fp,0,0);
	return ++count;        // ++count becuase the last line has not a '\n' in the end
}*/

/*
 * Creates the map
*/
/*char** get(const char* file){

   FILE *fp;
   fp = fopen(file,"r");
   if(fp == NULL) return NULL;
   int i=0,j=0,id;
   char c;

   N = getLinesNumber(fp);
   str = (char**) malloc(N*sizeof(char*));
   for(int k=0;k<N;k++) str[k] = NULL;

   while(i++<N ){
      int a;
      fscanf(fp,"%d",&id);    // Get id

      if(id != j) {
         printf("Index Error\n");
         return NULL;
      }

      fgetc(fp);           // Ignore tabs and whitespaces

      getline(&(str[j]),&a,fp);

      if((c=fgetc(fp)) == EOF) break;
      else ungetc(c,fp);
      j++;
   }
   fclose(fp);


return str;
}*/
