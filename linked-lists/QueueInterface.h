/* This is the file QueueInterface.h   */

#include "QueueTypes.h"

void InitializeQueue(Queue *Q);
int Empty(Queue *);
int Full(Queue *);
void Insert(ItemType R, Queue *Q,int type,int len);
int Remove(Queue *Q, ItemType F,int type);
