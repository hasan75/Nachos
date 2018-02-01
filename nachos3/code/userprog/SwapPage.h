#include "translate.h"
#include "filesys.h"
#include "bitmap.h"

#ifndef SwapPage_H
#define SwapPage_H




class SwapPage{
private:
    OpenFile *swapfile;
    int *map;
    int numPages;

public:
	
    SwapPage(int numPages, char *fileName);
    ~SwapPage();
	void saveIntoSwapSpace(TranslationEntry *pte);
	void loadfromSwapSpace(TranslationEntry *pte);
	bool isSwapPageExists(TranslationEntry *pte);

};

#endif
