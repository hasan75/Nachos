#include "synch.h"
extern int *buf;
extern Lock *lock;
extern Condition *can_produce;
extern Condition *can_consume;
extern int BUF_SIZE;
extern int len;
