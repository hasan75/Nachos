#include "synch.h"
#include "copyright.h"

#ifndef PROCESSTABLE_H
#define PROCESSTABLE_H

class Table{
	Lock *tableLock;
	int tableSize;
	int numOfCurProcess;
	void **tableArray;
	
	public:
	Table(int size);
	int Alloc(void *object);
	void *Get(int index);
	void Release(int index);
	bool isEmpty();
};
#endif

