#include "copyright.h"
#include "MemoryManager.h"

MemoryManager::MemoryManager(int numPages)
{
    memoryLock = new Lock("Memory Lock");
    bitmap = new BitMap(numPages);

}

int MemoryManager::AllocPage()
{
    memoryLock->Acquire();
    int num = bitmap->Find();
    memoryLock->Release();
    return num;
}

void MemoryManager::FreePage(int physPageNum)
{

	memoryLock->Acquire();
	bitmap->Clear(physPageNum);
	memoryLock->Release();
}

bool MemoryManager::PageIsAllocated(int physPageNum)
{
	memoryLock->Acquire();
	bool is = bitmap->Test(physPageNum);
    memoryLock->Release();
	return is;


}
