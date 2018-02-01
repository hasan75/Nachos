#include "synch.h"
#include "copyright.h"

class Table{
	Lock *tableLock;
	int tableSize;
	void **tableArray;
	
	public:
	Table(int size);
	int Alloc(void *object);
	void *Get(int index);
	void Release(int index);
	
};

