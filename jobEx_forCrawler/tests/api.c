#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "trie.h"

/*
 * Using getopt to get command line arguments
*/
int mygetopt(int argc, char * const argv[], int *p,char **filename){
   int opt,k;
   char *file;
   while((opt = getopt(argc,argv,":d:w:") )!= -1)   {
      switch (opt) {
         case 'd':
         file = (char*) malloc((strlen(optarg)+1)*sizeof(char));
            strcpy(file,optarg);
            printf("file name=%s\n", file);
            break;

         case 'w':
            k=atoi(optarg);
            if(k<1){
               perror("K can't be negative or 0");
               return -1;
            }
            printf("k=%d\n",k);
            break;
         case ':':
            k = 10;
            printf("k=%d\n",k);
            break;
         }
   }
   *p=k;
   *filename = file;
   return 0;

}






/*
 * Searching for key in trie, and return the p_list if exists
*/
p_list* find(const char *key){
      t_node *tmp = t;
      if(tmp ==NULL) {
         perror("Empty Trie");
         return NULL;
      }

      for(uint i=0;i<strlen(key);i++){
         while(key[i] != tmp->value){
            if(tmp->sibling == NULL) return NULL;
            tmp = tmp->sibling;
         }
         if(tmp->child == NULL && i<(strlen(key)-1)) return NULL;
         else if(i<(strlen(key)-1)) tmp = tmp->child;
         else continue;

      }
      if(tmp->plist == NULL) return NULL;
      else{
         p_list *p = tmp->plist;
         return p;
      }
}

/*
 * Recursive free of a p_list
*/
void p_free(p_list **p){
      p_list *tmp = *p;

      if(tmp == NULL) return;
      p_free(&(tmp->next));
      free(tmp);
}

/*
 * Recursive free of the trie
*/
void t_free(t_node **p){
      t_node *tmp = *p;
      if(tmp->child != NULL) t_free(&(tmp->child));
      if(tmp->sibling != NULL) t_free(&(tmp->sibling));
      if(tmp->plist != NULL) {
         p_free(&(tmp->plist));
      }
      free(tmp);
}

/*
 * Calls p_free and t_free and also frees some globals
*/
