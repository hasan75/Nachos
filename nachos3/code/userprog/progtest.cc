// progtest.cc
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"
#include "MemoryManager.h"
#include "machine.h"
#include "ProcessTable.h"

MemoryManager* memoryManager;
Table* processTable;
Lock *consoleLock;
Console *console;
Semaphore *readAvail;
Semaphore *writeDone;
//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------


void ReadAvail(void* arg) { readAvail->V(); }
void WriteDone(void* arg) { writeDone->V(); }

void
StartProcess(const char *filename)
{
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
		printf("Unable to open file %s\n", filename);
		return;
    }
    processTable = new Table(30);
    //memoryManager = new MemoryManager(NumPhysPages);
    memoryManager = new MemoryManager(5);
    consoleLock = new Lock("ConsoleLock");


    readAvail = new Semaphore("ReadConsoleSemaphore", 0);
    writeDone = new Semaphore("WriteConsoleSemaphore", 0);
    console =  new Console(NULL, NULL, ReadAvail, WriteDone, 0);


    space = new AddrSpace();
    int is = space->Initialize(executable);
    /*if(is == -1){
    	printf("Unable to allocate physical memory and failed to create addrSpace\n");
    	return;
    }*/
    currentThread->space = space;
	currentThread->processId = processTable->Alloc((void*)currentThread);
    //delete executable;			// close file

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register
    printf("StartProcess called\n");

    machine->Run();			// jump to the user progam
    ASSERT(false);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.



//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------




void
ConsoleTest (const char *in, const char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);

    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}
