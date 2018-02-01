#include "copyright.h"
#include "system.h"
#include "buffer.h"
#include "consumer.h"

Consumer::Consumer(){}
Consumer::~Consumer(){}
void Consumer::consume(int consumerNo)
{
    lock->Acquire();

    if(len == 0){
        can_consume->Wait(lock);
    }

    len--;
    printf("Consumer %d consumed %d\n",consumerNo,buf[len]);
    can_produce->Signal(lock);
    lock->Release();
}


