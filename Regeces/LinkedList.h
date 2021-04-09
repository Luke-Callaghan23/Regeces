#ifndef LINKED_LIST_H
#define LINKED_LIST_H 0

#define LinkedList struct _linkedlist
#define LLNode     struct _llnode
#include "CLI.h"
#include "string.h"



struct _llnode
{
	void * data;
	struct _llnode * next;
	struct _llnode * prev;
};

struct _linkedlist
{
	int iLen;
	struct _llnode * back;
	struct _llnode * front;
};


LinkedList * getLL();
Error * LLprepend(LinkedList *, void *);
Error * LLappend(LinkedList *, void *);
bool LLremoveItemIndex(LinkedList *, int);
bool LLremoveItemAddr(LinkedList *, void *);
void * LLgetItemAddr(LinkedList *, void*);
void * LLgetItemIndex(LinkedList *, int);
void ** LLgetAll(LinkedList *);
LinkedList * LLaddLL(LinkedList *, LinkedList *);
LinkedList * LLcopy(LinkedList *);
void * LLgetItemByPredicate(LinkedList *, Predicate, void *);
bool LLremoveItemByPredicate(LinkedList *, Predicate, void *);
LinkedList * LLfilter(LinkedList *, Predicate, void *);
LinkedList * LLfilterOrdered(LinkedList *, Predicate, void *);
bool LLreplaceItem(LinkedList *, Predicate, void *, void *);
LinkedList * LLify(void **, int);
LinkedList * LLsetify(LinkedList *, Predicate);
LinkedList * LLdifferenceAddr(LinkedList *, LinkedList *);
Error * freeLL(LinkedList *);
Error * printLL(LinkedList *, PrintFunc);
void * LLmax(LinkedList *, Compare);


void freeLinkedList(LinkedList *);

#endif // ! LINKED_LIST_H