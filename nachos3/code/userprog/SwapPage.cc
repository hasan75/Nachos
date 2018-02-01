#include "system.h"
#include "SwapPage.h"


SwapPage::SwapPage(int numP, char *fileName)
{
    numPages = numP;
	fileSystem->Create(fileName,numPages*PageSize);
	swapfile = fileSystem->Open(fileName);
	map = new int[numPages];

}
SwapPage::~SwapPage()
{
    delete map;

}

void SwapPage::saveIntoSwapSpace(TranslationEntry *pte)
{
    pte->valid =false;
    if(isSwapPageExists(pte) == false)
	{
		swapfile->WriteAt(&(machine->mainMemory[pte->physicalPage*PageSize]), PageSize,pte->virtualPage*PageSize);
        map[pte->virtualPage] = 1;
	}
}

void SwapPage::loadfromSwapSpace(TranslationEntry *pte)
{
	swapfile->ReadAt(&(machine->mainMemory[pte->physicalPage*PageSize]),PageSize,pte->virtualPage*PageSize);
}

bool
SwapPage::isSwapPageExists(TranslationEntry *pte)
{
	   int is = map[pte->virtualPage];
       if(is) return true;
       else return false;

}
