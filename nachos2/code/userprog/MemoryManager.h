#include "copyright.h"
#include "system.h"
#include "utility.h"
#include "bitmap.h"
#include "synch.h"

class MemoryManager{
    BitMap *bitmap;
    Lock *memoryLock;

    public:
    MemoryManager(int numPages);
    int AllocPage();
	void FreePage(int physPageNum);
	bool PageIsAllocated(int physPageNum);

};
