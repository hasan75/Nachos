// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "thread.h"
#include "producer.h"
#include "consumer.h"

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------



Producer *pro = new Producer();
Consumer *con = new Consumer();

void Produce(int id)
{
    for(int e = 0; e < 3; e++){
        pro->produce(id);
        currentThread->Yield();
    }
}

void Consume(int id){
    for(int e = 0; e < 3; e++){
        con->consume(id);
        currentThread->Yield();
    }
}







//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{

    DEBUG('t', "Entering SimpleTest");


    Thread *producers = new Thread("Producer");
    Thread *consumers = new Thread("Consumer");

    char *name;

    //Thread *t = new Thread("forked thread");


    //t->Fork((VoidFunctionPtr)SimpleThread,(void *)1);
    //SimpleThread(0);
    //SimpleThread(1);


    producers->Fork((VoidFunctionPtr)Produce,(void*)1);
    consumers->Fork((VoidFunctionPtr)Consume,(void *)1);
    Produce(0);
    Produce(1);
    Produce(2);
    Produce(3);
    Produce(4);

    Consume(0);
    Consume(1);
    Consume(2);
    Consume(3);
    Consume(4);




}

