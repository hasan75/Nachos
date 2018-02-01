#include "copyright.h"
#include "system.h"
#include "buffer.h"
#include "producer.h"


Producer::Producer()
{

}

Producer::~Producer()
{
    delete buf;
    delete lock;
    delete can_produce;
    delete can_consume;
}

void Producer::produce(int producerNo)
{

    lock->Acquire();
    if(len == BUF_SIZE){
        can_produce->Wait(lock);
    }
    int randomNum = rand()%1000000;
    printf("Producer %d produced %d\n",producerNo,randomNum);
    buf[len++] = randomNum;
    can_consume->Signal(lock);
    lock->Release();
}
