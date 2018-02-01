// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "openfile.h"
#include "addrspace.h"
#include "ProcessTable.h"
#include "MemoryManager.h"

extern Table *processTable;
extern MemoryManager* memoryManager;

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void startProcess(void *arg){
	currentThread->space->InitRegisters();
	currentThread->space->RestoreState();
	machine->Run();
	return;
}

SpaceId ExecIt(int addr,int fileSize){
	printf("Process Execution Called\n");
	char *fileName = new char[fileSize+1];
	int i = 0,readSomething;
	while(1){
		if(machine->ReadMem(addr+i,1,&readSomething) == false) return 0; //error occured
		fileName[i] = (char)readSomething;
		if(fileName[i] == '\0') break;
		i++;
	}
	
	printf("Desired file name %s\n",fileName);
	
	//now open this file
	OpenFile *executable = fileSystem->Open(fileName);
	if (executable == NULL) {
		printf("Unable to open the file %s\n",fileName);
		return 0;
	}
	
	AddrSpace *addrSpace = new AddrSpace(executable);
	Thread *t = new Thread("A thread of this process is going to create");
	int Id = processTable->Alloc((void*)t);
	if(Id == -1){
		printf("Process Allocation failed\n");
		return 0;
	}
	
	
	t->space = addrSpace;
	t->processId = Id;
	t->Fork((VoidFunctionPtr)startProcess,NULL);
	printf("a thread of this process Id %d is created\n",t->processId);
	return t->processId;
	
	
}

void incrementPC(){
	int pc;

	pc = machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg, pc);
	pc = machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg, pc);
	pc += 4;
	machine->WriteRegister(NextPCReg, pc);
	
}

void
ExceptionHandler(ExceptionType which)
{
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {
		DEBUG('a', "Shutdown, initiated by user program.\n");
	   	interrupt->Halt();
    }
    else if((which == SyscallException) && (type == SC_Exec)){
    	int Id = ExecIt(machine->ReadRegister(4),machine->ReadRegister(5));
    	machine->WriteRegister(2,Id);
    }
    else {
		printf("Unexpected user mode exception %d %d\n", which, type);
		ASSERT(false);
    }
    incrementPC();
    interrupt->SetLevel(oldLevel);
}








