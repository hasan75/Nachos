#include "copyright.h"
#include "MemoryManager.h"
#include "ProcessTable.h"

extern Table* processTable;

MemoryManager::MemoryManager(int numP)
{
    clock = 0;
    numPages = numP;
    memoryLock = new Lock("Memory Lock");
    bitmap = new BitMap(numPages);
    processMap = new int[numPages];
    entries = new TranslationEntry*[numPages];
    for(int i = 0; i < numPages; i++) entries[i] = NULL;
}

int MemoryManager::AllocPage()
{
    memoryLock->Acquire();
    int num = bitmap->Find();
    memoryLock->Release();
    return num;
}

int MemoryManager::Alloc(int processNo,TranslationEntry *entry)
{
    memoryLock->Acquire();
	int num = bitmap->Find();
    if(num != -1){
        processMap[num] = processNo;
        entries[num] = entry;
        printf("In Alloc\n");
        printf("TimeStamp for allocated page %d\n",entries[num]->timeStamp);
        //entries[num]->timeStamp++;
        printf("Allocated PageFrame No %d for process Num %d\n",num,processNo);
    }
	memoryLock->Release();
    return num;
}

//LRU

int MemoryManager::AllocByForce(int processNo,TranslationEntry *entry)
{
    memoryLock->Acquire();
    int num = -1;
    int timeMin = 100000000;
    for(int i = 0; i < numPages;i++){
        if(entries[i]->timeStamp < timeMin){
            num = i;
            timeMin = entries[i]->timeStamp;
        }
    }
    printf("In AllocByForce\n" );
    printf("Allocated PageFrame No %d for process Num %d\n",num,processNo);
    printf("TimeStamp for allocated page %d\n",entries[num]->timeStamp);
    //int num = rand() % numPages;
    //entries[num]->timeStamp++;

	Thread *thread = (Thread*)processTable->Get(processMap[num]);
	if(thread){
		thread->space->swapspace->saveIntoSwapSpace(entries[num]);
	}
    entries[num] = entry;
    processMap[num] = processNo;
    memoryLock->Release();
    return num;
}

void MemoryManager::FreePage(int physPageNum)
{

	memoryLock->Acquire();
	bitmap->Clear(physPageNum);
    processMap[physPageNum] = 0;
    entries[physPageNum] = NULL;
	memoryLock->Release();
}

unsigned int MemoryManager::NumOfAvailablePages()
{
	memoryLock->Acquire();
	unsigned int count = bitmap->NumClear();
	memoryLock->Release();
	return count;
}

bool MemoryManager::PageIsAllocated(int physPageNum)
{
	memoryLock->Acquire();
	bool is = bitmap->Test(physPageNum);
    memoryLock->Release();
	return is;


}
