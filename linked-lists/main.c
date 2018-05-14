#include <stdio.h>
#include <stdlib.h>
#include "QueueInterface.h"
#include <string.h>

int main(void)
{
   int i,j;
   Queue Q;

   InitializeQueue(&Q);

   for(i=1; i<10; ++i){
      Insert(&i, &Q,0,sizeof(int));
   }


   while (!Empty(&Q)){
      Remove(&Q, &j,0);
      printf("Item %d has been removed.\n", j);
   }
   Insert("kalispera",&Q,1,strlen("kalispera"));
   char buf[32];
   Remove(&Q,buf,1);
   printf("%s\n",buf );
}
