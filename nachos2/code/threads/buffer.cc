#include "synch.h"
#include "buffer.h"

int BUF_SIZE = 15;
int *buf = new int[BUF_SIZE];
int len = 0;
Lock *lock = new Lock("bufLock");
Condition *can_produce = new Condition("CanProduce");
Condition *can_consume = new Condition("CanConsume");
