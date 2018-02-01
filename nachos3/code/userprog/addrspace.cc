// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "translate.h"
#include "MemoryManager.h"


extern MemoryManager *memoryManager;

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace()
{
	pageTable = NULL;
	numPages = 0;
}


int AddrSpace::Initialize(OpenFile *execute)
{
	executable = execute;

    unsigned int i, size;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;



	//ASSERT(numPages <= memoryManager->NumOfAvailablePages());

	char fileName[15];
	sprintf(fileName,"Swap%d",currentThread->processId);
	swapspace = new SwapPage(numPages,fileName);
// first, set up the translation


    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
        pageTable[i].virtualPage = i;
        //pageTable[i].physicalPage = memoryManager->AllocPage();
		pageTable[i].physicalPage = -1;

        /*if(pageTable[i].physicalPage == -1) {
        	for(j = 0; j< i ;j++){
				memoryManager->FreePage(pageTable[j].physicalPage);
			}

        	return -1;
        }*/
        //pageTable[i].valid = true;
		pageTable[i].valid = false;
        pageTable[i].use = false;
        pageTable[i].dirty = false;
        pageTable[i].readOnly = false;
    }







    return 1;
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
	delete executable;
	unsigned int i;
    if (pageTable != NULL){
		for (i=0;i<numPages;i++)
			if (pageTable[i].valid){
				memoryManager->FreePage(pageTable[i].physicalPage);
            }
		delete pageTable;
	}

}

int AddrSpace::loadIntoFreePage(int addr,int physicalPageNo)
{
	printf("\nIn Addrspace loadIntoFreePage\n");
	int vpn = addr / PageSize;
	addr = vpn*PageSize;
	memoryManager->clock++;
	if(memoryManager->clock == 100000000) memoryManager->clock = 1;
	machine->pageTable[vpn].virtualPage = vpn;
	machine->pageTable[vpn].physicalPage = physicalPageNo;
	machine->pageTable[vpn].valid = true;
	machine->pageTable[vpn].use = false;
	machine->pageTable[vpn].dirty = false;
	machine->pageTable[vpn].readOnly = false;
	machine->pageTable[vpn].timeStamp = memoryManager->clock;


	//if in code segment
	if(swapspace->isSwapPageExists(&machine->pageTable[vpn]) == false){
		printf("SwapPage for vpn %d does not exist and loading it from executable\n",vpn);
		if(addr >= noffH.code.virtualAddr && addr < noffH.code.virtualAddr + noffH.code.size){
			printf("Page starts in code segment , ");
			int codeOffset = addr - noffH.code.virtualAddr;
			int codeSize = noffH.code.size - (codeOffset/PageSize)*PageSize;
			//int codeSize = noffH.code.size;
			if(PageSize < codeSize) codeSize = PageSize;
			executable->ReadAt(&(machine->mainMemory[physicalPageNo*PageSize]),codeSize,noffH.code.inFileAddr+codeOffset);

			if(codeSize < PageSize){
				printf("ends in initData segment\n");
				int initDataSize = PageSize - codeSize;
				executable->ReadAt(&(machine->mainMemory[physicalPageNo*PageSize+codeSize]),initDataSize,noffH.initData.inFileAddr);

			}
			else{
				printf("ends in code segment\n");
			}

		}

		//if initData segment
		else if(addr >= noffH.initData.virtualAddr && addr < noffH.initData.virtualAddr + noffH.initData.size){
			printf("Page starts in initData segment , ");
			int initDataOffset = addr - noffH.initData.virtualAddr;
			int initDataSize = noffH.initData.size - (initDataOffset/PageSize)*PageSize;
			//int initDataSize = noffH.initData.size;
			if(PageSize < initDataSize) initDataSize = PageSize;
			executable->ReadAt(&(machine->mainMemory[physicalPageNo*PageSize]),initDataSize,noffH.initData.inFileAddr+initDataOffset);

			if(initDataSize < PageSize){
				printf("ends in uninitData segment\n");
				int uninitDataSize = PageSize - initDataSize;
				bzero(&(machine->mainMemory[physicalPageNo*PageSize+initDataSize]),uninitDataSize);

			}
			else{
				printf("ends in initData segment\n");
			}

		}

		//if uninitData segments
		else if(addr >= noffH.uninitData.virtualAddr && addr < noffH.uninitData.virtualAddr + noffH.uninitData.size){
			printf("Page starts and ends in uninitData segment\n");
			//int uninitdataOffset = (addr - noffH.uninitData.virtualAddr)/PageSize;
			bzero(&(machine->mainMemory[physicalPageNo*PageSize]),PageSize);
		}
		else{
			//works here
		}
	}
	else{
		printf("vpn %d loads from SwapSpace\n",vpn);
		swapspace->loadfromSwapSpace(&machine->pageTable[vpn]);
	}


	printf("\n");



	return 1;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}
