#include "arraylist.h"
#include <stdlib.h>


const string arraylist_stack = "C.Interpreter.Collections.ArrayList.arraylist.c";

Error * ALreclaim(ArrayList *);

//Standard way of retrieving an arraylist
ArrayList * getAL(int len)
{
	if(len <= 0)
	{
		char buf[200];
		sprintf_s(buf, "Out of bounds Error while trying to create an ArrayList; negative length!  Inputted length: %d", len);
		return propagateError(getStringPtr(buf), 
            strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::getAL")), true);
	}
	ArrayList * arraylist = (ArrayList *)malloc(sizeof(ArrayList));
	if (!arraylist) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::getAL")), true);

	arraylist->iOccuLen = 0;
	arraylist->preList  = getLL();
	if (arraylist->preList == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::getAL")), false);
	arraylist->postList = getLL();
	if (arraylist->postList == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::getAL")), false);
	arraylist->iLen     = 0;
	arraylist->iArrLen  = len;
	arraylist->array    = (ALNode *)calloc(sizeof(ALNode), len);		//Be sure to calloc the memory because we need to keep track of emptiness in the array and uninitialized memory is bad for that
	if (!arraylist->array) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::getAL")), true);

	return arraylist;
}

Error * ALprepend(ArrayList * arraylist, void * data)
{

	
	if (arraylist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALprepend")), false);
	if (!arraylist) return propagateError(getStringPtr("NullPointerError!  Trying to prepend to a NULL ArrayList!"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALprepend")), true);

	//Checking if a reclamation is needed:
	//		a reclamation is needed whenever the sum of the length of the pre/post Lists
	//		has a length >= the length of the array
	if (arraylist->preList->iLen + arraylist->postList->iLen + 1 >= arraylist->iArrLen)
	{
		//Reclaim the LinkedLists and leave a l-wide buffer in the beginning of the array
		//		to save room for the incoming prepend element
		Error * e = ALreclaim(arraylist, -1);
		if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALprepend")), false);

		//Allocate inserted node
		ALNode * in = (LLNode *)malloc(sizeof(LLNode));
		if (!in) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALprepend")), true);

		//Initialize inserted node
		in->data = data;
		in->next = (arraylist->iArrLen > 0) ? arraylist->array[1] : NULL;
		in->prev = NULL;

		//Add new item into the first element of the array
		arraylist->array[0] = in;

		arraylist->iOccuLen++;
		arraylist->iLen++;
		e = ALreclaim(arraylist, arraylist->iLen);
		if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALprepend")), false);


		//then, return
		return NULL;
	}

	//Prepend to the pre list
	Error * e = LLprepend(arraylist->preList, data);
	if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALprepend")), false);
	//Increment size of list
	arraylist->iLen++;
	return NULL;
}

Error * ALappend(ArrayList * arraylist, void * data)
{
	//Standard validity checking
	if (arraylist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALappend")), false);
	if (!arraylist) return propagateError(getStringPtr("NullPointerError!  Trying to append to a NULL ArrayList!"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALappend")), true);



	//Checking if a reclamation is needed:
	//		a reclamation is needed whenever the sum of the length of the pre/post Lists
	//		has a length >= the length of the array
	if (arraylist->preList->iLen + arraylist->postList->iLen + 1 >= arraylist->iArrLen)
	{
		//Reclaim the LinkedLists and leave a l-wide buffer in the ending of the array
		//		to save room for the incoming append element
		Error * e = ALreclaim(arraylist, arraylist->iLen + 2);
		if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALappend")), false);


		//Allocate inserted node
		ALNode * in = (LLNode *)malloc(sizeof(LLNode));
		if (!in) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALappend")), true);


		//Initialize inserted node
		in->data = data;
		in->next = NULL;
		in->prev = (arraylist->iArrLen - 1 > 0) ? arraylist->array[arraylist->iArrLen - 2] : NULL;

		//Add new item into the last element of the array
		arraylist->array[arraylist->iOccuLen] = in;

		arraylist->iOccuLen++;
		arraylist->iLen++;

		//then, return;
		return NULL;
	}


	//Case: adding to the array portion of the arraylist
	if (arraylist->iOccuLen < arraylist->iArrLen)
	{
		//Allocate inserted node
		ALNode * in = (LLNode *)malloc(sizeof(LLNode));
		if (!in) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALappend")), true);


		//Initialize inserted node
		in->data = data;
		in->next = NULL;
		in->prev = (arraylist->iOccuLen > 0) ? arraylist->array[arraylist->iOccuLen - 1] : NULL;

		//Insert into array at current occupied end
		arraylist->array[arraylist->iOccuLen] = in;

		//Increment occupied length
		arraylist->iOccuLen++;
	}
	//Case: adding to the postList
	else
	{
		Error * e = LLappend(arraylist->postList, data);
		if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALappend")), false);
	}

	//Length of the arraylist is incremented in either case
	arraylist->iLen++;
	return NULL;
}

bool ALremoveItemIndex(ArrayList * arraylist, int index)
{
	
	if (arraylist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALremoveItemIndex")), false);
	if (!arraylist) return propagateError(getStringPtr("NullPointerError!  Trying to remove an item from a NULL ArrayList!"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALremoveItemIndex")), true);

	if (arraylist->iLen == 0) return false;

	//One item, just reset the list
	if (arraylist->iLen == 1)
	{
		//free old lists
		Error * e = freeLL(arraylist->preList);
		if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALremoveItemIndex")), false);
		free(arraylist->array);
		e = freeLL(arraylist->postList);
		if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALremoveItemIndex")), false);


		//Create new ones
		arraylist->preList = getLL();
		arraylist->postList = getLL();
		arraylist->array = (ALNode **)malloc(sizeof(ALNode *) * arraylist->iArrLen);
		
		//Error chacking
		if (arraylist->preList  == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALremoveItemIndex")), false);
		if (arraylist->postList == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALremoveItemIndex")), false);
		if (!arraylist->array) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALremoveItemIndex")), true);
	
		//Reseting lengths
		arraylist->iOccuLen = 0;
		arraylist->iLen = 0;

		//Success
		return true;
	}

	//Convert index
	int realIndex = indexConvert(index, arraylist->iLen);

	if (realIndex < arraylist->preList->iLen)
	{
		//If index is contained in the preList's length
		//		call LLremoveItemIndex on the prelist and the index
		bool bRemoved = LLremoveItemIndex(arraylist->preList, realIndex);
		if (bRemoved == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALremoveItemIndex")), false);
		arraylist->iLen--;
		return true;
	}
	else if (realIndex < arraylist->preList->iLen + arraylist->iOccuLen)
	{
		realIndex -= arraylist->preList->iLen;
		//Localize the deleted item
		ALNode * del = arraylist->array[realIndex];
		
		//Get the previous item
		ALNode * prev = del->prev;

		//Circumvent deleted item by connecting around it
		if (prev) prev->next = del->next;
		if (del->next) del->next->prev = prev;

		//Shift items back in the list
		for (int array = realIndex + 1; array < arraylist->iArrLen; array++)
		{
			arraylist->array[array - 1] = arraylist->array[array];
		}
		if (arraylist->postList->iLen <= 0) arraylist->iOccuLen--;
		arraylist->iLen--;

		//If there are items in the postList, shift the first item in the list down
		//		into the last spot in the arraylist's array
		if (arraylist->postList->iLen > 0)
		{
			arraylist->array[arraylist->iOccuLen - 1] = arraylist->postList->front;
			if (arraylist->postList->iLen == 1)
			{
				//If the list only had one item
				//		reset both the front and back pointers
				//		as well as the length
				arraylist->postList->front = NULL;
				arraylist->postList->back  = NULL;
				arraylist->postList->iLen  = 0;
			}
			else
			{
				//If there are more than one items in the list
				//		then decrement postList's length
				arraylist->postList->front = arraylist->postList->front->next;
				arraylist->postList->iLen--;
			}
		}
		else
		{
			arraylist->array[arraylist->iArrLen - 1] = NULL;
		}
		return true;
	}
	else if (realIndex < arraylist->preList->iLen + arraylist->iOccuLen + arraylist->postList->iLen)
	{
		//Call linkedlist remove index on postList and real index
		//		pushed backwards by the sizes of the previous two listss
		bool bRemoved = LLremoveItemIndex(arraylist->postList, realIndex - (arraylist->preList->iLen + arraylist->iOccuLen));
		if (bRemoved == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALremoveItemIndex")), false);
		//Decrement size
		arraylist->iLen--;
		return true;
	}
	return false;
}

bool ALremoveItemAddr(ArrayList * arraylist, void * addr)
{
	//Standard validity checking
	
	if (arraylist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALremoveItemAddr")), false);
	if (!arraylist) return propagateError(getStringPtr("NullPointerError!  Trying to remove an item from a NULL ArrayList!"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALremoveItemAddr")), true);

	if (arraylist->iLen == 0) return false;


	//Attempt to remove deom the postList
	bool preMoved = LLremoveItemAddr(arraylist->preList, addr);
	if (preMoved == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALremoveItemAddr")), false);
	if (preMoved) return true;
	
	//Attempting to remove from the array
	//Storing a prev pointer
	ALNode * prev = NULL;
	//Loop through items in array
	for (int outer = 0; outer < arraylist->iOccuLen; outer++)
	{
		if (arraylist->array[outer]->data == addr)
		{
			//Circumventing deleted item by linking around it
			if (prev) prev->next = arraylist->array[outer]->next;
			if (arraylist->array[outer]->next) arraylist->array[outer]->next->prev = prev;

			//Shifting items back in arraylist's array
			for (int inner = outer + 1; inner < arraylist->iArrLen; inner++)
			{
				arraylist->array[inner - 1] = arraylist->array[inner];
			}
			arraylist->iOccuLen--;
			arraylist->iLen--;

			//If there are items in the postList, shift the first item in the list down
			//		into the last spot in the arraylist's array
			if (arraylist->postList->iLen > 0)
			{
				arraylist->array[arraylist->iOccuLen] = arraylist->postList->front;
				if (arraylist->postList->iLen == 1)
				{
					//If the list only had one item
					//		reset both the front and back pointers
					//		as well as the length
					arraylist->postList->front = NULL;
					arraylist->postList->back  = NULL;
					arraylist->postList->iLen  = 0;
				}
				else
				{
					//If there are more than one items in the list
					//		then decrement postList's length
					arraylist->postList->front = arraylist->postList->front->next;
					arraylist->postList->iLen--;
				}
			}
			else
			{
				arraylist->array[arraylist->iArrLen - 1] = NULL;
			}
			return true;
		}
		prev = arraylist->array[outer];
	}

	//Attempt to remove from the postList
	bool postMoved = LLremoveItemAddr(arraylist->postList, addr);
	if (postMoved == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALremoveItemAddr")), false);
	if (postMoved) return true;

	//If item was never removed, return false
	return false;
}

void * ALgetItemAddr(ArrayList * arraylist, void * addr)
{
	//Standard validity checking
	if (arraylist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALgetItemAddr")), false);
	if (!arraylist) return propagateError(getStringPtr("NullPointerError!  Trying to retrieve an item from a NULL ArrayList!"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALgetItemAddr")), true);

	if (arraylist->iLen == 0) return NULL;


	//Search preList
	void * preFound = LLgetItemAddr(arraylist->preList, addr);
	if (preFound == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALgetItemAddr")), false);
	if (preFound) return preFound;

	//Search array
	for (int loop = 0; loop < arraylist->iOccuLen; loop++)
	{
		if (arraylist->array[loop]->data == addr)
		{
			return arraylist->array[loop];
		}
	}

	//Search postList
	void * postFound = LLgetItemAddr(arraylist->postList, addr);
	if (postFound == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALgetItemAddr")), false);
	if (postFound) return postFound;

	//If item was never found in any of the searches,
	//		return NULL
	return NULL;
}

void * ALgetItemIndex(ArrayList * arraylist, int index)
{
	//Standard validity checking
	if (arraylist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALgetItemIndex")), false);
	if (!arraylist) return propagateError(getStringPtr("NullPointerError!  Trying to retrieve an item from a NULL ArrayList!"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALgetItemIndex")), true);

	if (arraylist->iLen == 0) return NULL;

	
	//Convert index
	int realIndex = indexConvert(index, arraylist->iLen);

	if (realIndex < arraylist->preList->iLen)
	{
		//If item is contained withing the preList,
		//		call LLgetItemIndex
		void * aGot = LLgetItemIndex(arraylist->preList, realIndex);
		if (aGot == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALgetItemIndex")), false);
		return aGot;
	}
	else if (realIndex < arraylist->preList->iLen + arraylist->iOccuLen)
	{
		//If item is within the array section of the ArrayList, return with O(1) time :D
		return arraylist->array[realIndex - arraylist->preList->iLen]->data;
	}
	else if (realIndex < arraylist->preList->iLen + arraylist->iOccuLen + arraylist->postList->iLen)
	{
		//If item is contained in the postList
		//		call LLgetItemIndex
		void * aGot = LLgetItemIndex(arraylist->postList, realIndex - arraylist->iOccuLen - arraylist->preList->iLen);
		if (aGot == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALgetItemIndex")), false);
		return aGot;
	}

	//If index was never found, return NULL
	return NULL;
}

ArrayList * ALify(void ** array, int iLen)
{
	if (array == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALify")), false);
	if (!array) return propagateError(getStringPtr("NullPointerError!  Trying to convert NULL array to LinkedList!"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALify")), true);

	if (iLen < 0)
	{
		char buf[200];
		sprintf_s(buf, "Out of bounds Error while trying to convert pointer array to ArrayList; negative array length!  Inputted Array length: %d", iLen);
		return propagateError(getStringPtr(buf), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALify")), true);
	}

	ArrayList * arraylist = (ArrayList *)malloc(sizeof(ArrayList));
	if (!arraylist) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALify")), true);

	//Initializing lengths
	arraylist->iLen    = iLen;
	arraylist->iArrLen = iLen;

	//Initializing linked lists
	arraylist->preList  = getLL();
	arraylist->postList = getLL();

	//Allocating array
	arraylist->array = (ALNode **)malloc(sizeof(ALNode *) * iLen);

	if (arraylist->preList == error)  return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALify")), false);
	if (arraylist->postList == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALify")), false);
	if (!arraylist->array) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALify")), true);

	//Allocate first inserted node
	ALNode * first = (LLNode *)malloc(sizeof(ALNode));
	if (!first) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALify")), true);

	//Initialize first inserted node
	first->data = array[0];
	first->next = NULL;
	first->prev = NULL;

	ALNode * prev = first;

	for (int loop = 1; loop < iLen; loop++)
	{
		//Allocate inserted node
		ALNode * in = (LLNode *)malloc(sizeof(LLNode));
		if (!in) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALify")), true);

		//Initialize inserted node
		in->data = array[loop];
		//Next is null (for now)
		in->next = NULL;
		//Prev is previous node in array
		in->prev = prev;

		//Setting previous' next to current
		prev->next = in;

		//Resetting prev
		prev = in;

		//Insertion
		arraylist->array[loop] = in;
	}

	//Return result
	return arraylist;
}

void ** ALgetAll(ArrayList * arraylist)
{
	if (arraylist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALgetAll")), false);
	if (!arraylist) return propagateError(getStringPtr("NullPointerError!  Trying to retrieve items from a NULL ArrayList!"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALgetAll")), true);

	//First, reclaim the array list, with no buffers
	Error * e = ALreclaim(arraylist, 0);
	if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALgetAll")), false);

	//Allocate a new array
	void ** array = (void **)calloc(sizeof(void *), arraylist->iLen);
	if(!array) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALgetAll")), true);

	//Fill in array with ALNodes' data
	for (int loop = 0; loop < arraylist->iLen; loop++)
	{
		array[loop] = (arraylist->array[loop])->data;
	}

	return array;

}

ArrayList * ALaddAL(ArrayList * front, ArrayList * add)
{
	//If front doesn't exist, LL become add
	if (!front || front->iLen <= 0)
	{
		if (add)
		{
			Error * e = ALreclaim(add, 0);
			if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALaddAL")), false);
		}
		return add;
	}

	if (!add || add->iLen <= 0)
	{
		Error * e = ALreclaim(front, 0);
		if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALaddAL")), false);
		return front;
	}
	
	//Store the al lengths
	int iFrontLen = front->iLen;
	int iAddLen   = add->iLen;

	//Reclaim the front list, but leave a buffer at the end of the array of the size 
	//		of the added list
	Error * e = ALreclaim(front, iAddLen);
	if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::AladdAL")), false);
	//Reclaim the added list, leaving no buffer
	e = ALreclaim(add, 0);
	if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::AladdAL")), false);

	//Add every element in the reclaimed add list
	//		to the back end of the reclaimed front list
	for (int loop = 0; loop < iAddLen; loop++)
	{
		front->array[iFrontLen + loop] = add->array[loop];
	}

	//Return edited front list
	return front;
}

//Copies the given arraylist
ArrayList * ALcopy(ArrayList * arraylist)
{
	if (arraylist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALcopy")), false);
	if (!arraylist) return propagateError(getStringPtr("NullPointerError!  Trying copy a NULL ArrayList!"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALcopy")), true);


	//Allocate new arraylist
	ArrayList * al = getAL(arraylist->iLen);
	if (al == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALcopy")), false);

	//Might as well reclaim the old arraylist while we're at it
	ALreclaim(arraylist, 0);

	//Allocating first ndoe
	ALNode * first = (ALNode *)malloc(sizeof(ALNode));
	if (!first) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALcopy")), true);

	//Initializing first node
	first->prev = NULL;
	first->next = NULL;
	first->data = (arraylist->array[0])->data;

	//Inserting first node into front of the array
	al->array[0] = first;

	//Initializing 'prev' node
	ALNode * prev = first;

	//Fill in array with new ALNodes
	for (int loop = 1; loop < arraylist->iLen - 1; loop++)
	{
		//Allocating node
		ALNode * in = (ALNode *)malloc(sizeof(ALNode));
		if (!in) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALcopy")), true);

		//Initializing node
		in->next = NULL;
		in->prev = prev;
		in->data = (arraylist->array[loop])->data;

		//Setting prev's next
		prev->next = in;

		//Setting next iteration's prev to the current element
		prev = in;

		al->array[loop] = in;
	}

	return al;
}

void * ALgetItemByPredicate(ArrayList * arraylist, Predicate pred, void * argOne)
{
	if (arraylist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALgetItemByPredicate")), false);
	if (!arraylist) return propagateError(getStringPtr("NullPointerError!  Trying to retrieve items from NULL ArrayList!"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALgetItemByPredicate")), true);

	if (arraylist->preList->iLen > 0)
	{
		void * data = LLgetItemByPredicate(arraylist->preList, pred, argOne);
		if (data == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALgetItemByPredicate")), false);
		if (data) return data;
	}
	
	//Filtering array
	ALNode * arrItem;
	for (int array = 0; array < arraylist->iOccuLen; array++)
	{
		arrItem = arraylist->array[array];
		if ((*pred)(argOne, arrItem->data))
		{
			return arrItem->data;
		}
	}

	if (arraylist->postList->iLen > 0)
	{
		void * data = LLgetItemByPredicate(arraylist->postList, pred, argOne);
		if (data == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALgetItemByPredicate")), false);
		if (data) return data;
	}

	return NULL;

}

ArrayList * ALfilter(ArrayList * arraylist, Predicate pred, void * argOne)
{
	if (arraylist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALfilter")), false);
	if (!arraylist) return propagateError(getStringPtr("NullPointerError!  Trying copy a NULL ArrayList!"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALfilter")), true);

	ArrayList * ret = getAL(arraylist->iLen);
	if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALfilter")), false);

	ALNode * prev = NULL;

	//Filtering pre list
	LLNode * left  = arraylist->preList->front;
	LLNode * right = arraylist->preList->back;
	for (int loop = 0; loop <= arraylist->preList->iLen / 2; loop++)
	{
		//Appending left or right items into list if they 
		//		meet the predicate
		if ((*pred)(argOne, left->data))
		{
			//Allocate
			ALNode * in = (ALNode *)malloc(sizeof(ALNode));
			if (!in) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::getAL")), true);

			//Initialize
			in->data = left->data;
			in->next = NULL;
			in->prev = prev;

			//Set prev variables
			if (prev) prev->next = in;
			prev = in;

			//Insert into array
			ret->array[ret->iOccuLen] = in;

			//Incremenet iOccuLen
			ret->iOccuLen++;
		}
		if ((*pred)(argOne, right->data))
		{
			//Allocate
			ALNode * in = (ALNode *)malloc(sizeof(ALNode));
			if (!in) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::getAL")), true);

			//Initialize
			in->data = right->data;
			in->next = NULL;
			in->prev = prev;

			//Set prev variables
			if (prev) prev->next = in;
			prev = in;

			//Insert into array
			ret->array[ret->iOccuLen] = in;

			//Incremenet iOccuLen
			ret->iOccuLen++;
		}

		//Bad things happen if we left left and right pass each other
		//		break loop
		if (right->prev == left)
		{
			break;
		}
		//advance loop ptrs
		left  = left->next;
		right = right->prev;
	}

	//Filtering array
	ALNode * arrItem;
	for (int array = 0; array < arraylist->iOccuLen; array++)
	{
		arrItem = arraylist->array[array];
		if ((*pred)(argOne, arrItem->data))
		{
			//Allocate
			ALNode * in = (ALNode *)malloc(sizeof(ALNode));
			if (!in) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::getAL")), true);

			//Initialize
			in->data = arrItem->data;
			in->next = NULL;
			in->prev = prev;

			//Set prev variables
			if (prev) prev->next = in;
			prev = in;

			//Insert into array
			ret->array[ret->iOccuLen] = in;

			//Incremenet iOccuLen
			ret->iOccuLen++;
		}
	}


	//Filtering post list
	left  = arraylist->postList->front;
	right = arraylist->postList->back;
	for (int loop = 0; loop <= arraylist->preList->iLen / 2; loop++)
	{
		//Appending left or right items into list if they 
		//		meet the predicate
		if ((*pred)(argOne, left->data))
		{
			//Allocate
			ALNode * in = (ALNode *)malloc(sizeof(ALNode));
			if (!in) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::getAL")), true);

			//Initialize
			in->data = left->data;
			in->next = NULL;
			in->prev = prev;

			//Set prev variables
			if (prev) prev->next = in;
			prev = in;

			//Insert into array
			ret->array[ret->iOccuLen] = in;

			//Incremenet iOccuLen
			ret->iOccuLen++;
		}
		if ((*pred)(argOne, right->data))
		{
			//Allocate
			ALNode * in = (ALNode *)malloc(sizeof(ALNode));
			if (!in) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::getAL")), true);

			//Initialize
			in->data = right->data;
			in->next = NULL;
			in->prev = prev;

			//Set prev variables
			if (prev) prev->next = in;
			prev = in;

			//Insert into array
			ret->array[ret->iOccuLen] = in;

			//Incremenet iOccuLen
			ret->iOccuLen++;
		}

		//Bad things happen if we left left and right pass each other
		//		break loop
		if (right->prev == left)
		{
			break;
		}
		//advance loop ptrs
		left  = left->next;
		right = right->prev;
	}

	return ret;
}

ArrayList * ALfilterOrdered(ArrayList * arraylist, Predicate pred, void * argOne)
{
	if (arraylist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALfilterOrdered")), false);
	if (!arraylist) return propagateError(getStringPtr("NullPointerError!  Trying to replace an item in a NULL ArrayList!"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALfilterOrdered")), true);


	//If empty list, return new empy list
	if (arraylist->iOccuLen + arraylist->preList->iLen + arraylist->postList->iLen == 0)
	{
		ArrayList * filt = getAL(5);
		if (filt == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALfilterOrdered")), false);
		return filt;
	}
	//Allocate new arraylist
	ArrayList * ret = getAL(arraylist->iLen);
	if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALfilterOrdered")), false);

	//Filter preList
	ALNode * prePtr = arraylist->preList->front;
	while (prePtr)
	{
		if ((*pred)(argOne, prePtr->data))
		{
			Error * e = ALappend(ret, prePtr->data);
			if (e == error) propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALfilterOrdered")), false);
		}

		prePtr = prePtr->next;
	}

	//Filter array
	ALNode * arrPtr;
	for (int array = 0; array < arraylist->iOccuLen; array++)
	{
		arrPtr = arraylist->array[array];
		if ((*pred)(argOne, arrPtr->data))
		{
			Error * e = ALappend(ret, arrPtr->data);
			if (e == error) propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALfilterOrdered")), false);
		}
	}

	//Filter postList
	ALNode * postPtr = arraylist->postList->front;
	while (postPtr)
	{
		if ((*pred)(argOne, postPtr->data))
		{
			Error * e = ALappend(ret, postPtr->data);
			if (e == error) propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALfilterOrdered")), false);
		}

		postPtr = postPtr->next;
	}

	return ret;
}

bool ALreplaceItem(ArrayList * arraylist, Predicate pred, void * argOne, void * replace)
{
	//Standard validity check
	if (arraylist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALreplaceItem")), false);
	if (!arraylist) return propagateError(getStringPtr("NullPointerError!  Trying to replace an item in a NULL ArrayList!"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALreplaceItem")), true);


	//Call LLreplace item on preList if preList is occupied
	//		if replaced, return true
	if (arraylist->preList->iLen > 0)
	{
		bool found = LLreplaceItem(arraylist->preList, pred, argOne, replace);
		if (found == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALreplaceItem")), false);
		if (found) return true;
	}

	//Loop through every item in array
	//		searching for one that matches input 
	//		predicate
	ALNode * arrItem;
	for (int array = 0; array < arraylist->iOccuLen; array++)
	{
		arrItem = arraylist->array[array];
		if ((*pred)(argOne, arrItem))
		{
			//Allocating replace item
			ALNode * rep = (ALNode *)malloc(sizeof(ALNode));
			if (!rep) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALreplaceItem")), true);

			//Set data of replace node to replace
			rep->data = replace;
			
			//Set prev/next links to old item's prev/next
			rep->next = arrItem->next;
			rep->prev = arrItem->prev;

			//Setting replace item
			arraylist->array[array] = rep;

			//Return true
			return true;
		}
	}

	//If still not found,
	//		call LLreplaceItem on postList
	if (arraylist->postList->iLen > 0)
	{
		bool found = LLreplaceItem(arraylist->postList, pred, argOne, replace);
		if (found == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALreplaceItem")), false);
		if (found) return true;
	}

	//If not found in any of those lists, return false
	return false;
}

//A reclaimation occurs on an arraylist whenever the arraylist becomes unbalanced
//		An unbalanced arraylist is defined as:
//				an arraylist whose LinkedLists are larger than the size of the array
//A reclamation will recreate the array in the arraylist to include all items currently in the arraylist
Error * ALreclaim(ArrayList * arraylist, int iBuffer)
{
	
	if (arraylist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALreclaim")), false);
	if (!arraylist) return propagateError(getStringPtr("NullPointerError!  Trying to reclaim a NULL ArrayList!"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALreclaim")), true);

		
	//Standard validity checking ... do nothing if params are not valid
	//A reclaim can only happen if either the post list or the pre list
	//		has at least one item
	//If this is not true, simply return
	if (!arraylist->iArrLen) return NULL;

	


	//Create new array of the size of the initial array plus the linkedlist size
	int iNewLen =  (arraylist->preList->iLen + arraylist->iArrLen + arraylist->postList->iLen + abs(iBuffer));
	ALNode ** aNewArr = (ALNode *)calloc(sizeof(ALNode), iNewLen);		//Arrays are calloced, to keep the array clean and usable
	if (!aNewArr) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALreclaim")), true);


	//Initializing length variables
	int iPreLen  = arraylist->preList->iLen;
	int iArrLen  = arraylist->iArrLen;
	int iPostLen = arraylist->postList->iLen;

	int iRealBuffer;

	//iBuffer:
	//		when iBuffer is negative, then all items in the created array will be offset forwards
	//				by the magnitude of iBuffer -- this is to leave room for |iBuffer| preprends onto the array
	//		when iBuffer is positive, then all items in the creater array will not be offset,
	//				but there will be a buffer of iBuffer's magnitude at the right end of the array -- this is to leave room for |iBuffer| appends onto the array
	if (iBuffer < 0)
	{
		iRealBuffer = abs(iBuffer);
	}
	else
	{
		iRealBuffer = 0;
	}

	//Loop through the pre linked list and insert LLNodes from there into
	//		new array
	//Iterating by LLNode, 'preptr'
	LLNode * preptr = arraylist->preList->front;
	for (int initial = 0; initial < iPreLen; initial++)
	{
		aNewArr[initial + iRealBuffer] = preptr;
		preptr = preptr->next;
	}

	//Adding array elements into resized array
	for (int array = 0; array < iArrLen; array++)
	{
		aNewArr[iPreLen + array + iRealBuffer] = arraylist->array[array];
	}

	//Loop through the post linked list and insert LLNodes from there into
	//		new array
	//Iterating by LLNode, 'postptr'
	LLNode * postptr = arraylist->postList->front;
	for (int final = 0; final < iPostLen; final++)
	{
		aNewArr[iPreLen + iArrLen + final + iRealBuffer] = postptr;
		postptr = postptr->next;
	}

	//Free old data
	free(arraylist->array);
	//Pre and post lists are only free (and then recreated) when they have a size greater than 0
	if (arraylist->preList->iLen > 0) 
	{ 
		free(arraylist->preList);  
		arraylist->preList = getLL();  
		if (arraylist->preList == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALreclaim")), false);

	}
	if (arraylist->postList->iLen > 0) 
	{ 
		free(arraylist->postList); 
		arraylist->postList = getLL(); 
		if (arraylist->postList == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALreclaim")), false);

	}

	//Set new al data
	arraylist->array   = aNewArr;
	arraylist->iArrLen = iNewLen;
	//arraylist->iLen    = iNewLen;

	arraylist->iOccuLen = arraylist->iLen - iPreLen;
	return NULL;
}

ArrayList * ALsetify(ArrayList * arraylist, Predicate equalityOp)
{
	if ( arraylist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALsetify")), false);
	if (!arraylist || arraylist->iLen <= 0)
	{
		//If the arraylist doesn't exist,
		//		make a new arraylist with a small initial size
		ArrayList * ret = getAL(3);
		if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALsetify")), false);
		return ret;
	}

	//Create a new arraylist with the same starting size as the 
	//		input list
	ArrayList * set = getAL(arraylist->iLen);
	if (set == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALsetify")), false);

	//Allocate
	ALNode * first = (ALNode *)malloc(sizeof(ALNode));
	if (!first) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALsetify")), true);

	//Initialize
	first->prev = NULL;
	first->next = NULL;
	
	ALNode * prev = NULL;

	//If the ArrayList exists and is not empty, add the
	//		the front item to the set
	if (arraylist->preList->iLen > 0)
	{
		//Set data to the preList's first element
		first->data = arraylist->preList->front->data;
		set->array[0] = first;
		set->iOccuLen = 1;
		set->iLen = 1;
		

		int iIndex = 1;
		LLNode * prePtr  = arraylist->preList->front->next;
		prev = first;
		while (prePtr)
		{
			bool inSet = false;
			for (int inner = 0; inner < set->iOccuLen; inner++)
			{
				if (set->array[inner] && equalityOp(set->array[inner]->data, prePtr->data))
				{
					printf("%d already in set!\n", prePtr->data);
					inSet = true;
					break;
				}
			}
			
			if (!inSet)
			{
				//Allocate
				ALNode * in = (ALNode *)malloc(sizeof(ALNode));
				if (!in) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALsetify")), true);

				//Initialize
				in->next = NULL;
				in->prev = prev;
				in->data = prePtr->data;

				//Insert
				set->array[set->iOccuLen++] = in;
				set->iLen++;

				//Advance prev
				prev->next = in;
				prev = in;
			}


			//Advance loop iterators
			iIndex++;
			prePtr = prePtr->next;
		}

	}
	else
	{

		//Set data to the array's first element
		first->data = (arraylist->array[0]) ? (arraylist->array[0])->data : NULL;
		set->iOccuLen = 1;
		set->array[0] = first;
		prev = first;
	}
	

	//Looping through elements in the original's array
	ALNode * arrItem;
	for (int array = 0; array < arraylist->iOccuLen; array++)
	{

		arrItem = arraylist->array[array];

		//Searching if item is already in the list
		bool inSet = false;
		for (int inner = 0; inner < set->iOccuLen; inner++)
		{
			if (set->array[inner] && equalityOp(set->array[inner]->data, arrItem->data))
			{
				
				inSet = true;
				break;
			}
		}
			
		if (!inSet)
		{
			//Allocate
			ALNode * in = (ALNode *)malloc(sizeof(ALNode));
			if (!in) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALsetify")), true);


			//Initialize
			in->next = NULL;
			in->prev = prev;
			in->data = arrItem->data;

			//Insert
			set->array[set->iOccuLen++] = in;
			set->iLen++;

			//Advance prev
			prev->next = in;
			prev = in;
		}
	}

	
	if (arraylist->postList->iLen > 0)
	{
		int iIndex = 1;
		LLNode * postPtr  = arraylist->postList->front->next;
		while (postPtr)
		{
			bool inSet = false;
			for (int inner = 0; inner < set->iOccuLen; inner++)
			{
				if (set->array[inner] && equalityOp(set->array[inner]->data, postPtr->data))
				{
					
					inSet = true;
					break;
				}
			}
			
			if (!inSet)
			{
				//Allocate
				ALNode * in = (ALNode *)malloc(sizeof(ALNode));
				if (!in) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::ALsetify")), true);

				//Initialize
				in->next = NULL;
				in->prev = prev;
				in->data = postPtr->data;

				//Insert
				set->array[set->iOccuLen++] = in;
				set->iLen++;

				//Advance prev
				prev->next = in;
				prev = in;
			}


			//Advance loop iterators
			iIndex++;
			postPtr = postPtr->next;
		}

	}
	return set;
}

Error * freeAL(ArrayList * arraylist)
{
	
	//If arraylist doesn't exist, return
	if (arraylist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::freeAL")), false);
	if (!arraylist) return propagateError(getStringPtr("NullPointerError!  Attempting to free a NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::freeAL")), true);
	//If arraylist has an invalid size, free and then return
	if (arraylist->iLen <= 0)
	{
		free(arraylist);
		return;
	}

	LinkedList * removedAddresses = getLL();
	if (removedAddresses == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::freeAL")), false);
	
	if (arraylist->preList->iLen > 0)
	{
		//Enclosing iteration:
		//		left advances forward and right advances backwards until either the item 
		//		is found (by checking both end of the LL) or
		//		the left and right pointers of the LL meet in the middle
		LLNode * left  = arraylist->preList->front, * nextLeft;
		LLNode * right = arraylist->preList->back, * nextRight;
		for (int loop = 0; loop <= arraylist->preList->iLen / 2 && left && right; loop++)
		{
			//Store next links
			nextLeft = left->next;
			nextRight = right->prev;

			//Free items, if they have not already been freed
			//		and add freed items to addresses LinkedList
			if (!LLgetItemAddr(removedAddresses, left))
			{
				//Free left, if it has not already been freed
				Error * e = LLappend(removedAddresses, left);
				if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::freeAL")), false);
				free(left);
			}
			if (!LLgetItemAddr(removedAddresses, right))
			{
				//Free right, if it has not already been freed
				Error * e = LLappend(removedAddresses, right);
				if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::freeAL")), false);
				free(right);
			}

			//Advance iteration vars
			left  = nextLeft;
			right = nextRight;
		}
	}
	free(arraylist->preList);

	ALNode * front, * back;
	for (int loop = 0; loop <= arraylist->iOccuLen / 2; loop++)
	{
		front = arraylist->array[loop];
		back  = arraylist->array[arraylist->iOccuLen - 1 - loop];

		if (!LLgetItemAddr(removedAddresses, front))
		{
			//Free front if not already freed
			Error * e = LLappend(removedAddresses, front);
			if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::freeAL")), false);
			free(front);
		}
		if (!LLgetItemAddr(removedAddresses, back))
		{
			//Free back if not already freed
			Error * e = LLappend(removedAddresses, back);
			if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::freeAL")), false);
			free(back);
		}
	}
	free(arraylist->array);


	if (arraylist->postList->iLen > 0)
	{
		//Enclosing iteration:
		//		left advances forward and right advances backwards until either the item 
		//		is found (by checking both end of the LL) or
		//		the left and right pointers of the LL meet in the middle
		LLNode * left  = arraylist->postList->front, * nextLeft;
		LLNode * right = arraylist->postList->back, * nextRight;
		for (int loop = 0; loop <= arraylist->postList->iLen / 2 && left && right; loop++)
		{
			//Store next links
			nextLeft = left->next;
			nextRight = right->prev;

			//Free items, if they have not already been freed
			//		and add freed items to addresses LinkedList
			if (!LLgetItemAddr(removedAddresses, left))
			{
				//Free left, if it has not already been freed
				Error * e = LLappend(removedAddresses, left);
				if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::freeAL")), false);
				free(left);
			}
			if (!LLgetItemAddr(removedAddresses, right))
			{
				//Free right, if it has not already been freed
				Error * e = LLappend(removedAddresses, right);
				if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::freeAL")), false);
				free(right);
			}

			//Advance iteration vars
			left  = nextLeft;
			right = nextRight;
		}
	}
	//Free LL of removed addressess
	LLNode * frontPtr = removedAddresses->front;
	LLNode * next;
	while (frontPtr)
	{
		next = frontPtr->next;
		free(frontPtr);
		frontPtr = next;
	}
	free(removedAddresses);
	free(arraylist->postList);
	free(arraylist);

	return NULL;
}

Error * printAL(ArrayList* arraylist, PrintFunc pf)
{
	if (arraylist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::printAL")), false);
	if (!arraylist) return propagateError(getStringPtr("NullPointerError!  Trying to print a NULL ArrayList!"), strAddStringFreeBoth(getStringPtr(arraylist_stack), getStringPtr("::printAL")), true);

	if (!arraylist->iLen) { printf("EMPTY"); return NULL; }
	
	printf("\n");
	printf("Len: %d\n",     arraylist->iLen);
	printf("ArrLen: %d\n",  arraylist->iArrLen);
	printf("OccuLen: %d\n", arraylist->iOccuLen);
	printf("PreLen: %d\n",  arraylist->preList->iLen);
	printf("PostLen: %d\n", arraylist->postList->iLen);
	printf("\n");

	//Print preList by calling printLL
	printf("Pre List: ");
	printLL(arraylist->preList, pf);
	printf("\n");

	printf("Array: ");
	//Print array by iterating each item and calling the pf function on it
	for (int loop = 0; loop < arraylist->iArrLen; loop++)
	{
		if (arraylist->array[loop])
		{
			pf(arraylist->array[loop]->data);
		}
		else
		{
			printf("_");
		}
		if (loop != arraylist->iArrLen - 1) printf(", ");
	}
	printf("\n");

	//Print postList by calling printLL
	printf("Post List: ");
	printLL(arraylist->postList, pf);

	return NULL;

}