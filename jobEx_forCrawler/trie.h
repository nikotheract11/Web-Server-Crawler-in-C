#ifndef _INCL_GUARD
#define _INCL_GUARD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define  k_1 1.2
#define b 0.75
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/*
struct t_node *t;  // Trie root node
char ***str; // the map to the texts
char **files; // paths for each worker
volatile char exited;

FILE *fp; // log file

struct list{
   char *path;
   struct list *next;
};

typedef struct list list;

int insertAtStart(char *p,list **l);

struct pair {     // A struct using to store id and score in the heap
   char path[512];
   int score;
};


struct p_list{    // this is a posting list node
   int line;      // the line that the word is found
   int text_id;   // the id of the text
   int freq;
   char *path;    // full path to the file


   int plen;      // the length of the plist only found on the first element of each p_list
   struct p_list *next;
};

struct t_node {   // this is a trie node
   struct t_node *child;
   struct t_node *sibling;
   char value;
   struct p_list *plist;

};

typedef struct pair pair;
typedef struct t_node t_node;
typedef struct p_list p_list;
typedef unsigned int uint;

// trie.c
void catcher(int signum);
int p_init(p_list **,int ,int );
int t_init(t_node **);
int addplist(t_node **, int ,int);
int append(t_node **,const char *,int,int);
int insert(const char *,int,int);
char** get(const char*);
int getLinesNumber(FILE *);

// api.c
p_list * find(const char *);
int init(char**, int );
void mfree();
void p_free(p_list **);
void t_free(t_node **);

// app_functions.c
int mygetopt(int , char * const arg[], int *,char **);
int interface(int*,int*,int,list **,pid_t *);
char * search(char** s,int j,char *k);
char * operate(char **,int,char *);
char * readMsg(int *rfd,int w,int d,char *op);
int sendMsg(char *buf,int *wr,int w);
char **getok(char* s,int *k);
char *wc(char ***s);
int worker(int rfd,int wfd);
#endif
*/

struct t_node *t;  // Trie root node
char ***str; // the map to the texts
char **files; // paths for each worker
volatile char exited;

FILE *fp; // log file

struct list{
   char *path;
   struct list *next;
};

typedef struct list list;

int insertAtStart(char *p,list **l);

struct pair {     // A struct using to store id and score in the heap
   char path[512];
   int score;
};


struct p_list{    // this is a posting list node
   int line;      // the line that the word is found
   int text_id;   // the id of the text
   int freq;
   char *path;    // full path to the file


   int plen;      // the length of the plist only found on the first element of each p_list
   struct p_list *next;
};

struct t_node {   // this is a trie node
   struct t_node *child;
   struct t_node *sibling;
   char value;
   struct p_list *plist;

};

typedef struct pair pair;
typedef struct t_node t_node;
typedef struct p_list p_list;
typedef unsigned int uint;

// trie.c
void catcher(int signum);
int p_init(p_list **,int ,int );
int t_init(t_node **);
int addplist(t_node **, int ,int);
int append(t_node **,const char *,int,int);
int insert(const char *,int,int);
char** get(const char*);
int getLinesNumber(FILE *);

// api.c
p_list * find(const char *);
int init(char**, int );
void mfree();
void p_free(p_list **);
void t_free(t_node **);

// app_functions.c
int mygetopt(int , char * const arg[], int *,char **);
int interface(int *wfd,int *rfd,int w,char *buf,int sock);
char * search(char** s,int j,char *k);
char * operate(char **,int,char *);
char * readMsg(int *rfd,int w,int d,char *op,int sock);
int sendMsg(char *buf,int *wr,int w);
char **getok(char* s,int *k);
char *wc(char ***s);
int worker(int rfd,int wfd);


int jexec(char *buf,char *docfile,int sock);
#endif