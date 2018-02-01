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

#include "console.h"
#include "synch.h"
#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "openfile.h"
#include "addrspace.h"
#include "ProcessTable.h"
#include "MemoryManager.h"
#include "ProcessTable.h"

extern Table *processTable;
extern MemoryManager* memoryManager;
extern Lock *consoleLock;
extern Console *console;
extern Semaphore *readAvail;
extern Semaphore *writeDone;

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

void processStart(void *arg){
	currentThread->space->InitRegisters();
	currentThread->space->RestoreState();
	machine->Run();
	//ASSERT(false);
}

SpaceId myExec(){
	printf("\n\nIn myExec\n\n");

	int addr = machine->ReadRegister(4);
	int fileSize = machine->ReadRegister(5);
	char *fileName = new char[fileSize+1];
	int i = 0,readSomething;
	printf("Process Execution Called\n");
	while(true){
		bool is = machine->ReadMem(addr+i,1,&readSomething);
		if(is == false) return 0; //error occured
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

	AddrSpace *addrSpace = new AddrSpace();
	int is = addrSpace->Initialize(executable);
    if(is == -1){
    	printf("Unable to allocate physical memory and failed to create addrSpace\n");
    	return 0;
    }
	Thread *newThread = new Thread("PorcessThread");
	int Id = processTable->Alloc((void*)newThread);
	if(Id == -1){
		printf("Process Allocation failed\n");
		return 0;
	}

	newThread->processId = Id;
	newThread->space = addrSpace;
	newThread->Fork((VoidFunctionPtr)processStart,NULL);
	printf("\n\nA thread of this process Id %d is created\n\n",newThread->processId);
	return Id;


}

void myExit(int status)
{
	printf("\nExit status: %d\n",status);
	int pSize = machine->pageTableSize;
	for(int i=0;i<pSize;i++)
	{

		if(machine->pageTable[i].valid){
			memoryManager->FreePage(machine->pageTable[i].physicalPage);
		}

	}
	processTable->Release(currentThread->processId);
	if(processTable->isEmpty()){
		interrupt->Halt();
	}
	currentThread->Finish();
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

int myRead(){
	consoleLock->Acquire();

	int virAddr = machine->ReadRegister(4);
	int fileSize = machine->ReadRegister(5);

	int count = 0,i = 0;

	while(fileSize > 0){
		readAvail->P();
		char readData;
		readData = console->GetChar();
		if(readData == '\0') break;
		bool is = machine->WriteMem(virAddr +i,1,readData);
		if(is == false) return -1;
		count++;
		fileSize--;
		i++;

	}
	machine->WriteRegister(2,count );
	consoleLock->Release();
	return count;
}

int myWrite(){
	consoleLock->Acquire();
	int virAddr = machine->ReadRegister(4);
	int fileSize = machine->ReadRegister(5);

	int i = 0;

	while(fileSize > 0){
		int writeData;
		bool is;
		is = machine->ReadMem(virAddr+i,1,&writeData);
		if(is == false) return -1;
		char data = (char)writeData;
		//printf("Character is : %c\n",data);
		console->PutChar(data);
		writeDone->P();
		fileSize--;
		i++;
	}
	consoleLock->Release();
	return 1;
}

void
ExceptionHandler(ExceptionType which)
{
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
    int type = machine->ReadRegister(2);

	if(which == SyscallException){
		incrementPC();
		if (type == SC_Halt) {
	    	printf("SC_Halt\n");
			DEBUG('a', "Shutdown, initiated by user program.\n");
		   	interrupt->Halt();
	    }
	    else if(type == SC_Exec){
	    	printf("SC_Exec\n");
	    	SpaceId Id = myExec();
	    	machine->WriteRegister(2,Id);
	    }
	    else if(type == SC_Exit) {
	    	printf("SC_Exit\n");
			int status = machine->ReadRegister(4);

			myExit(status);
		}
		else if( type == SC_Read) {
	    	//printf("SC_Read\n");
			int count = myRead();
			if(count == -1) printf("Console Read Failed\n");


		}
		else if( type == SC_Write) {
	    	printf("SC_Write\n");
			int is = myWrite();
			if(is == -1) printf("Console Write Failed\n");
			//else printf("Console Write Successful\n");

		}

		else {
			printf("Unexpected user mode exception hihi %d %d\n", which, type);
			ASSERT(false);
	    }

	}
	else if(which == PageFaultException){

		int virAddr = machine->ReadRegister(39);
		int virtualPageNum = virAddr / PageSize;
		printf("\nPageFaultException for vpn %d\n",virtualPageNum);
		int physicalPageNum = -1;
		if(memoryManager->NumOfAvailablePages() > 0){
			//physicalPageNum = memoryManager->AllocPage();
			physicalPageNum = memoryManager->Alloc(currentThread->processId,&(machine->pageTable[virtualPageNum]));
		}
		else{
			//will force to free a page
			//will do this later
			physicalPageNum = memoryManager->AllocByForce(currentThread->processId,&(machine->pageTable[virtualPageNum]));

		}
		currentThread->space->loadIntoFreePage(virAddr,physicalPageNum);

		//interrupt->Halt();
	}
	else if(which == ReadOnlyException){
		printf("ReadOnlyException\n");

	}
	else if(which == BusErrorException){
		printf("BusErrorException\n");

	}
	else if(which == AddressErrorException){
		printf("AddressErrorException\n");

	}
	else if(which == OverflowException){
		printf("OverflowException\n");

	}
	else if(which == IllegalInstrException){
		printf("IllegalInstrException\n");

	}
	else if(which == NumExceptionTypes){
		printf("NumExceptionTypes\n");

	}

	else {
		printf("Unexpected user mode exception %d %d\n", which, type);
		ASSERT(false);
	}



    interrupt->SetLevel(oldLevel);
}
