#include "copyright.h"
#include "ProcessTable.h"

Table::Table(int size){
	tableSize = size;
	tableArray = new void*[size];
	
	for(int i = 0; i < tableSize; i++){
		tableArray[i] = NULL;
	}
	
	tableLock = new Lock("Process Table Lock");
	
	
}

int Table :: Alloc(void *object){
	tableLock->Acquire();
	int is = -1;
	for(int i = 0; i < tableSize; i++){
		if(tableArray[i] == NULL){
			tableArray[i] = object;
			is = i;
			break;
		}
	}
	tableLock->Release();
	
	if(is == -1){
		return -1;
	}
	else{
		return is;
	}
}

void *Table:: Get(int index){
	tableLock->Acquire();
	if(index >= 0 && index < tableSize){
		tableLock->Release();
		return tableArray[index];
	
	}else{
		tableLock->Release();
		return NULL;
	}
}


void Table::Release(int index){
	tableLock->Acquire();
	if(index >= 0 && index < tableSize){
		tableArray[index] = NULL;
	
	}
	tableLock->Release();
	return;
}


















