/* This is the file QueueImplementation.c */

#include <stdio.h>
#include <stdlib.h>
#include "QueueInterface.h"
#include <string.h>

void InitializeQueue(Queue *Q)
{
    Q->Front=NULL;
    Q->Rear=NULL;
}

int Empty(Queue *Q)
{
   return(Q->Front==NULL);
}

int Full(Queue *Q)
{
   return(0);
}
/* We assume an already constructed queue */
/* is not full since it can potentially grow */
/* as a linked structure.                   */

void Insert(ItemType R, Queue *Q,int type,int len)
{
   QueueNode *Temp;

   Temp=(QueueNode *)malloc(sizeof(QueueNode));

   if (Temp==NULL){
      printf("System storage is exhausted");
   } else {

      if(type == 0){
         Temp->Item = malloc(sizeof(int));
          *(int *)Temp->Item=*(int *)R;
       }
       else {
          Temp->Item = malloc(len+1);
          memcpy(Temp->Item,R,len);
          ((char*) Temp->Item)[len] = '\0';
       }
      Temp->Link=NULL;
      if (Q->Rear==NULL){
         Q->Front=Temp;
         Q->Rear=Temp;
      } else {
         Q->Rear->Link=Temp;
         Q->Rear=Temp;
      }
   }
}


int Remove(Queue *Q, ItemType F,int type)
{
   QueueNode *Temp;


   if (Q->Front==NULL){
      return 0;
   } else {
      if(type == 0 ) *(int *)F=*(int *)Q->Front->Item;     // for int
      else {
         memcpy((char *)F,(char *)Q->Front->Item,strlen((char *)Q->Front->Item));             // for string
         ((char *)F)[strlen((char *)Q->Front->Item)] = '\0';
      }

      Temp=Q->Front;
      Q->Front=Temp->Link;
      free(Temp);
      if (Q->Front==NULL) Q->Rear=NULL;

   }
   return 1;
}

int search_q(Queue Q,char *str){
  if(Empty(&Q)) return 0; // if queue is empty str not in queue

  QueueNode *tmp = Q.Front;
  while(tmp != NULL){
    char *buf=malloc(1024);
    strcpy(buf,(char*)tmp->Item);
    while(buf[0] == ' ') buf = (char*)((long) buf + 1);
    int a=strlen(buf);
    while(buf[a-1]== ' ') buf[--a] = '\0';
    if (!strcmp(str,(char*)tmp->Item)) return 1; // found str
    tmp = tmp->Link;
    free(buf);
  }
  return 0; // not found


}
