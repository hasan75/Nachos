#include "copyright.h"
#include "system.h"
#include "utility.h"
#include "bitmap.h"
#include "synch.h"
#include "translate.h"

#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

class MemoryManager{
    int numPages;
    BitMap *bitmap;
    Lock *memoryLock;
    //xtra
    int *processMap;
    TranslationEntry **entries;

    public:
    int clock;
    MemoryManager(int numPages);
    int AllocPage();
    int Alloc(int processNo,TranslationEntry *entry);
    int AllocByForce(int processNo,TranslationEntry *entry);
	void FreePage(int physPageNum);
	bool PageIsAllocated(int physPageNum);
	unsigned int NumOfAvailablePages();
};

#endif
