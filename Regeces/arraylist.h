#ifndef ARRAY_LIST_H

#include "CLI.h"
#include "linkedlist.h"

#define ArrayList struct _arrlist
#define ALNode struct _llnode

struct _arrlist
{
	//Total length of all items in the arraylist
	int iLen;

	//length of "occupied" area of the 
	//		below array
	//When an item is appended or inserted in the index area of the
	//		AL that the array occupies, this is incremented
	int iOccuLen;

	//List of prepended items
	//		any time ALprepend is called on a list
	//		the items are stored here
	LinkedList * preList;

	//Array of quick, random access items
	ALNode ** array;
	int iArrLen;

	//List of extra items that overflowed
	//		from the original array
	LinkedList * postList;
};

ArrayList * getAL(int);
Error * ALprepend(ArrayList *, void *);
Error * ALappend(ArrayList *, void *);
bool ALremoveItemIndex(ArrayList *, int);
bool ALremoveItemAddr(ArrayList *, void *);
void * ALgetItemAddr(ArrayList *, void*);
void * ALgetItemIndex(ArrayList *, int);
void ** ALgetAll(ArrayList *);
ArrayList * ALaddAL(ArrayList *, ArrayList *);
ArrayList * ALcopy(ArrayList *);
void * ALgetItemByPredicate(ArrayList *, Predicate, void *);
ArrayList * ALfilter(ArrayList *, Predicate, void *);
bool ALreplaceItem(ArrayList *, Predicate, void *, void *);
ArrayList * ALify(void **, int);
ArrayList * ALsetify(ArrayList *, Predicate);
Error * freeAL(ArrayList *);
Error * printAL(ArrayList *, PrintFunc);


void freeArrayList(ArrayList *);


#endif // !ARRAY_LIST_H
