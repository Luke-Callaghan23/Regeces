#include "LinkedList.h"
#include <stdio.h>
#include <stdlib.h>



const string linkedlist_stack = "LinkedList.c";


LinkedList * getLL()
{
	LinkedList * ll = (LinkedList *)malloc(sizeof(LinkedList));
	if (ll)
	{
		ll->iLen = 0;
		ll->front = NULL;
		ll->back  = NULL;
	}
	else
	{
		return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::getLL")), true);
	}
	return ll;
}

Error * LLprepend(LinkedList * linkedlist, void * data)
{
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLprepend")), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Trying to prepend to a NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLprepend")), true);
	//As long as list is not null, there should be no failure for
	//		adding, so auto-increment LinkedList's size
	linkedlist->iLen += 1;
	LLNode * in = (LLNode *)malloc(sizeof(LLNode));
	if (in)
	{
		//Initializing data of node
		in->data = data;

		//Setting new node's next to the current beginning of the LL
		in->next = linkedlist->front;

		//Linking items
		if(linkedlist->front)
			linkedlist->front->prev = in;

		//Setting new back node's back to NULL -- there is no back
		in->prev = NULL;


		//Setting ll's data pointer to the new item
		linkedlist->front = in;

		//If no back exists, add it
		if (!linkedlist->back)
		{
			linkedlist->back = in;
		}
	}
}

Error * LLappend(LinkedList * linkedlist, void * data) 
{
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLappend")), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Trying to apppend to a NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLappend")), true);
	//As long as list is not null, there should be no failure for
	//		adding, so auto-increment LinkedList's size
	linkedlist->iLen += 1;
	LLNode * in = (LLNode *)malloc(sizeof(LLNode));
	if (in)
	{
		//Initializing data of node
		in->data = data;

		//Setting new back node's next to NULL -- there is no next
		in->next = NULL;

		//Setting new node's next to the current beginning of the LL
		in->prev = linkedlist->back;

		//More links
		if(linkedlist->back)
			linkedlist->back->next = in;

		//Setting ll's data pointer to the new item
		linkedlist->back = in;

		//If no front exists, add it
		if (!(linkedlist->front))
		{
			linkedlist->front = in;
		}
	}
}

//Simple getter method for a node's next or prev
//		used as function pointers in these doubly linked lists
LLNode * secretGetNextFront(LLNode * front) { return front->next;  }
LLNode *  secretGetNextBack(LLNode *  back) { return back->prev;   }


//For some reason this function turned out to be very syntacically, uh, enhanced.  Beware.
void * LLgetItemIndex(LinkedList* linkedlist, int index)
{
	//If LL does not exist, or index we're searching for
	//		is greater than the size, item is not in LL,
	//		return NULL
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLgetItemIndex")), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Trying to retrieve item from NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLgetItemIndex")), true);

	//Convert index
	int realIndex = indexConvert(index, linkedlist->iLen);

	LLNode * iter = (index >= 0) ? 
		linkedlist->front : 
		linkedlist->back;
	LLNode * (*itf)(LLNode *) = (index >= 0) ?
		& secretGetNextFront :
		& secretGetNextBack;

	//I wanted to show off a little so I wrote the loop in the for loop header:
	for (int loop = 0; 
		iter && loop < linkedlist->iLen && loop != realIndex; 
		loop += !(iter = (*itf)(iter)) || 1);

	/*
	Translated:
	
	for (int loop = 0; loop < linkedlist->iLen; loop++)
	{
		//Need to add an additional if statement if not doing fancy way -- poor design -- bad marks
		if(!iter)
		{
			break;
		}

		//Declaring a variable ------ in A LOOP ---- inefficient ------ auto-fail
		int modifiedIndex = (*indf)(linkedlist->iLen, loop)

		//Ew ... another inefficient if statement D:|
		if(modifiedIndex == index)
		{
			break;
		}

		//Awesomely efficient function pointer iteration 
		iter = (*itf)(iter);
	}
	
	*/

	//If iter is NULL, return iter,
	//		else return data of iter
	return iter ? iter->data : iter;
}


void * LLgetItemAddr(LinkedList* linkedlist, void * addr)
{
	//If LL does not exist, item is not in LL,
	//		return NULL
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLgetItemAddr")), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Trying to retrieve an item from a NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLgetItemAddr")), true);

	if (linkedlist->iLen == 0) return NULL;

	//Enclosing iteration:
	//		left advances forward and right advances backwards until either the item 
	//		is found (by checking both end of the LL) or
	//		the left and right pointers of the LL meet in the middle
	LLNode * left  = linkedlist->front;
	LLNode * right = linkedlist->back;
	for (int loop = 0; loop <= linkedlist->iLen / 2; loop++)
	{
		if (left->data == addr)
		{
			return left->data;
		}
		if(right->data == addr)
		{
			return right->data;
		}

		left  = left->next;
		right = right->prev;
	}

	return NULL;

}



bool LLremoveItemIndex(LinkedList* linkedlist, int index)
{
	//If LL does not exist, or index we're searching for
	//		is greater than the size, item is not in LL,
	//		return NULL
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLremoveItemIndex")), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Trying to remove items from NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLremoveItemIndex")), true);

	if (linkedlist->iLen == 0) return false;

	//Convert index
	int realIndex = indexConvert(index, linkedlist->iLen);

	//Side of linked list to start iterating from
	LLNode * iter = (index >= 0) ? 
		linkedlist->front : 
		linkedlist->back;
	LLNode * (*itf)(LLNode *) = (index >= 0) ?
		& secretGetNextFront :
		& secretGetNextBack;

	LLNode * prev = NULL;
	for (int loop = 0; loop < linkedlist->iLen; loop++)
	{
		if (loop == realIndex)
		{
			//All this abstraction now leads to another issue:
			//		in the edge cases where ither the left-going or right-going pointers reach the target 
			//				index on the first iteration
			//		This causes an issue because if either the first or the last item in the list is changed, the linked list's pointer 
			//				must be changed
			//If prev does not exist:
			if (!prev)
			{
				if (index >= 0)
				{
					if (iter->next)
					{
						iter->next->prev = NULL;
					}
				}
				else
				{
					if (iter->prev)
					{
						iter->prev->next = NULL;
					}
				}


				//If iter's prev does not exist we know to reset the front pointer in the linked list, because the only item
				//		in a doubly linked list that has no prev is the front item
				if (!iter->prev)
				{
					linkedlist->front = linkedlist->front->next;
					//If front is now NULL, set back to NULL as well
					//		covers for cases of 1-wide linkedlists
					if (!(linkedlist->front))
					{
						linkedlist->back = NULL;
					}
				}
				//Else, we know that we need to edit the linkedlist's back pointer because the only item with both a previous and an modified index of 0
				else
				{
					linkedlist->back = linkedlist->back->prev;
					//If back is now NULL, set back to NULL as well
					//		covers for cases of 1-wide linkedlists
					if (!(linkedlist->back))
					{
						linkedlist->front = NULL;
					}
				}
			}
			else
			{
				//Getting next item by, again, calling itf with iter
				LLNode * next = (*itf)(iter);

				//Removing item normally
				prev->next = next;
				if (next)
				{
					next->prev = prev;
				}
				else
				{
					linkedlist->back = prev;
				}

			}
			//Free LLNode shell
			//		don't free data in shell ... it may be being used elsewhere
			free(iter);
			break;
		}

		//Iterate
		prev = iter;
		iter = (*itf)(iter);  //Iter advances by the iteration function
	}

	//Decrement linked list's size
	linkedlist->iLen -= 1;

	return true;
}

bool LLremoveItemAddr(LinkedList* linkedlist, void * addr)
{
	//If LL does not exist, or index we're searching for
	//		is greater than the size, item is not in LL,
	//		return NULL
	
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLremoveItemAddr")), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Trying to remove items from NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLremoveItemAddr")), true);

	if (linkedlist->iLen == 0) return false;


	LLNode * leftPtr   = linkedlist->front;
	LLNode * leftPrev  = NULL;
	LLNode * rightPtr  = linkedlist->back;
	LLNode * rightPrev = NULL;
	for (int loop = 0; loop <= linkedlist->iLen / 2; loop++)
	{
		if (leftPtr->data == addr)
		{
			//Standard doubly linkedlist removal
			if (!leftPrev)
			{
				linkedlist->front = leftPtr->next;
				//If front of linkedlist is now null, set back to null as well
				if (!(linkedlist->front))
				{
					linkedlist->back = NULL;
				}
				else
				{
					linkedlist->front->prev = NULL;
				}
			}
			else
			{
				//very standard removal
				leftPrev->next = leftPtr->next;
				leftPrev->next->prev = leftPrev;
			}


			//Free the LLNode * shell around the removed item --- not the data itself ,
			//		it may be referenced elsewhere (besides, you can't free void ptrs)
			free(leftPtr);

			linkedlist->iLen--;
			return true;
		}
		if(rightPtr->data == addr)
		{
			//Standard doubly linkedlist removal
			if (!rightPrev)
			{
				linkedlist->back = rightPtr->prev;
				//If back of linkedlist is now null, set front to null as well
				if (!linkedlist->back)
				{
					linkedlist->front = NULL;
				}
				else
				{
					linkedlist->back->next = NULL;
				}
			}
			else
			{
				//very standard removal
				rightPrev->prev = rightPtr->prev;
				rightPtr->prev->next = rightPrev;
			}

			//Free the LLNode * shell around the removed item --- not the data itself ,
			//		it may be referenced elsewhere (besides, you can't free void ptrs)
			free(rightPtr);
			linkedlist->iLen--;
			return true;
		}
		
		//Advancing loop vars
		rightPrev = rightPtr;
		rightPtr  = rightPtr->prev;
		leftPrev  = leftPtr;
		leftPtr   = leftPtr->next;
	}
	return false;
}


void ** LLgetAll(LinkedList* linkedlist)
{
	//If linkedList doesn't exist, return NULL
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLgetAll")), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Trying to retrieve items from NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLgetAll")), true);


	//Initializing return array
	void ** ret = (void **)malloc(sizeof(void *) * linkedlist->iLen);
	if (ret && linkedlist->iLen)
	{
		//Looping through linked list lr-enclosing
		LLNode * left  = linkedlist->front;
		LLNode * right = linkedlist->back;
		int leftIndex  = -1;
		int rightIndex = -1;
		for (int loop = 0; loop <= linkedlist->iLen / 2; loop++)
		{
			//Translating loop into current left and right index positions
			//		these are the positions that will be filled with the items of the enclosing
			//		left and right pointers
			leftIndex  = loop;
			rightIndex = linkedlist->iLen - loop - 1;

			//Setting items at either end in returned array
			ret[leftIndex]  = left->data;
			ret[rightIndex] = right->data;

			//Advancing loop ptrs
			left  = left->next;
			right = right->prev;
		}
	}
	return ret;
}


//INPLACE ADDING two linked lists
LinkedList * LLaddLL(LinkedList* front, LinkedList* add)
{
	
	if (front == error || add == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLcopy")), false);

	//If front doesn't exist, LL becomes add
	if (!front)
	{
		return add;
	}
	else if (!front->front || !front->back)
	{
		front->iLen  = add->iLen;
		front->front = add->front;
		front->back  = add->back;
		return front;
	}

	if (!add || !add->front || !add->back)
	{
		return front;
	}

	//Resizing linkedlist len
	front->iLen += add->iLen;

	//Standard linking
	front->back->next = add->front;
	add->front->prev = front->back;

	//Because this is an inplace operation, we edit the original front linkedlist's data 
	//		to change the back pointer
	front->back = add->back;
	return front;
}

//Copies (and reverses) a LinkedList
//		used for when order of a LinkedList does not matter
LinkedList * LLcopy(LinkedList * linkedlist)
{
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLcopy")), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Trying to copy a NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLcopy")), true);


	LinkedList * copied = getLL();
	if (copied == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLcopy")), false);

	for(LLNode * llnPtr = linkedlist->front; llnPtr; llnPtr = llnPtr->next)
	{
		Error * e = LLappend(copied, llnPtr->data);
		if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLcopy")), false);
	}

	return copied;
}


//Gets the first item in the LinkedList that returns true with the predicate
void * LLgetItemByPredicate(LinkedList* linkedlist, Predicate pred, void * argOne)
{
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLgetItemByPredicate")), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Trying to retrieve items from NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLgetItemByPredicate")), true);

	//Loop vars
	LLNode * ptr = linkedlist->front;
	while (ptr)
	{
		//Checking predicate against left and right pointers
		if ((*pred)(argOne, ptr->data))
		{
			return ptr->data;
		}

		//Advancing loop pointers
		ptr = ptr->next;
	}
	return NULL;
}
bool LLremoveItemByPredicate(LinkedList* linkedlist, Predicate pred, void * argOne)
{
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLremoveItemByPredicate")), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Trying to remove items from NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLremoveItemByPredicate")), true);

	//Loop vars
	LLNode * ptr  = linkedlist->front;
	LLNode * prev = NULL;
	while (ptr)
	{
		//Checking predicate against left and right pointers
		if ((*pred)(argOne, ptr->data))
		{
			if (prev)
			{
				prev->next = ptr->next;
			}
			else
			{
				linkedlist->front = ptr->next;
				if (linkedlist->iLen == 1)
				{
					linkedlist->back = NULL;
				}
			}
			if (ptr->next)
			{
				ptr->next->prev = ptr->prev;
			}
			else
			{
				linkedlist->back = prev;
			}
			linkedlist->iLen--;
			free(ptr);
			return true;
		}

		//Advancing loop pointers
		prev = ptr;
		ptr  = ptr->next;
	}
	return false;
}


LinkedList* LLfilter(LinkedList * linkedlist, Predicate pred, void * argOne)
{
	
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLfilter")), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Trying to filter items from NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLfilter")), true);

	//Making returned ll
	LinkedList * ret = getLL();
	if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLfilter")), false);

	if (linkedlist->iLen == 0)
	{
		LinkedList * ret = getLL();
		if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLfilter")), false);
		return ret;
	}

	//Loop vars
	LLNode * left  = linkedlist->front;
	LLNode * right = linkedlist->back;
	for (int loop = 0; loop <= linkedlist->iLen / 2; loop++)
	{
		//Appending left or right items into list if they 
		//		meet the predicate
		if ((*pred)(argOne, left->data))
		{
			Error * e = LLappend(ret, left->data);
			if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLfilter")), false);
		}

		if (left != right)
		{
			if ((*pred)(argOne, right->data))
			{
				Error * e = LLprepend(ret, right->data);
				if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLfilter")), false);
			}
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

LinkedList * LLfilterOrdered(LinkedList * linkedlist, Predicate pred, void * argOne)
{
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLfilterOrdered")), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Trying to filter items from NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLfilterOrdered")), true);


	//Initializing return list
	LinkedList * ret = getLL();
	if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLfilter")), false);

	LLNode * ptr = linkedlist->front;
	while (ptr)
	{
		//Applying predicate
		if ((*pred)(argOne, ptr->data))
		{
			//Appending to return list
			Error * e = LLappend(ret, ptr->data);
			if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLfilter")), false);
		}
		ptr = ptr->next;
	}
	//Returning list
	return ret;
}



//Replaces an item in a LinkedList
bool LLreplaceItem(LinkedList * linkedlist, Predicate pred, void * argOne,  void * replace)
{
	
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLreplaceItem")), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Trying to filter items from NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLreplaceItem")), true);

	//Loop var
	LLNode * ptr = linkedlist->front;
	while (ptr)
	{
		//Chacking for address-level equality
		if ((*pred)(argOne, ptr->data))
		{
			//Swapping data pointers
			ptr->data = replace;
			return true;
		}
		ptr  = ptr->next;
	}
	return false;
}

LinkedList * LLify(void ** arr, int iLen)
{
	if (arr == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLify")), false);
	if (!arr) return propagateError(getStringPtr("NullPointerError!  Trying to convert NULL array to LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLify")), true);

	if (iLen < 0)
	{
		char buf[200];
		sprintf_s(buf, "Out of bounds Error while trying to convert pointer array to LinkedList; negative array length!  Inputted Array length: %d", iLen);
		return propagateError(getStringPtr(buf), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLify")), true);
	}
	LinkedList * ret = getLL();
	if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLify")), false);

	for (int loop = 0; loop < iLen; loop++)
	{
		Error * e = LLappend(ret, arr[loop]);			
		if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLify")), false);

	}
	return ret;
}

LinkedList * LLsetify(LinkedList * linkedlist, Predicate equalityOp)
{
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLsetify")), false);

	//If the LinkedList does not exist, or the LinkedList is empty, the set version
	//		will be an empty LL
	if (!linkedlist || !linkedlist->iLen)
	{
		return getLL();
	}

	//If the LinkedList exists and is not empty, add the
	//		the front item to the set
	LLNode * ptr = linkedlist->front;
	LinkedList * set = getLL();
	if (set == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLsetify")), false);
	
	Error * e = LLappend(set, ptr->data);
	if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLsetify")), false);

	//And advance the pointer
	ptr = ptr->next;

	while (ptr)
	{
		void * aFoundItem = LLgetItemByPredicate(set, equalityOp, ptr->data);
		if (aFoundItem == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLsetify")), false);
		if (!aFoundItem)
		{
			e = LLappend(set, ptr->data);
			if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLsetify")), false);
		}
		ptr = ptr->next;
	}
	return set;
}

Error * freeLL(LinkedList * linkedlist)
{
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::freeLL")), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Attempting to free a NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::freeLL")), true);


	if(linkedlist->iLen == 1)
	{
		free(linkedlist->front);
		free(linkedlist);
		return NULL;
	}

	LinkedList * removedAddresses = getLL();
	if (removedAddresses == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::freeLL")), false);

	//Enclosing iteration:
	//		left advances forward and right advances backwards until either the item 
	//		is found (by checking both end of the LL) or
	//		the left and right pointers of the LL meet in the middle
	LLNode * left  = linkedlist->front, * nextLeft;
	LLNode * right = linkedlist->back,  * nextRight;
	for (int loop = 0; loop <= linkedlist->iLen / 2 && left && right; loop++)
	{
		//Store next links
		nextLeft = left->next;
		nextRight = right->prev;

		//Free items, if they have not already been freed
		//		and add freed items to addresses LinkedList
		void * aRemovedLeft = LLgetItemAddr(removedAddresses, left);
		if (aRemovedLeft == error) propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::freeLL")), false);
		if (!aRemovedLeft)
		{
			//Free left, if it has not already been freed
			Error * e = LLappend(removedAddresses, left);
			if (e == error) propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::freeLL")), false);
			free(left);
		}
		void * aRemovedRight = LLgetItemAddr(removedAddresses, right);
		if (aRemovedRight == error) propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::freeLL")), false);
		if (!aRemovedRight)
		{
			//Free right, if it has not already been freed
			Error * e = LLappend(removedAddresses, right);
			if (e == error) propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::freeLL")), false);
			free(right);
		}

		//Bad things happen if we left left and right pass each other
		//		break loop
		if (nextRight == left)
		{
			break;
		}

		//Advance iteration vars
		left  = nextLeft;
		right = nextRight;
	}

	LLNode * frontPtr = removedAddresses->front;
	LLNode * next;
	while (frontPtr)
	{
		next = frontPtr->next;
		free(frontPtr);
		frontPtr = next;
	}
	free(removedAddresses);
	free(linkedlist);
	return NULL;
}


LinkedList * LLdifferenceAddr(LinkedList * llOne, LinkedList * llTwo)
{
	if (llOne == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLdifferenceAddr")), false);
	if (llTwo == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLdifferenceAddr")), false);
	
	if (!llOne && !llTwo) return propagateError(getStringPtr("NullPointerError!  Trying to convert NULL array to LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLdifferenceAddr")), true);
	if (!llOne && llTwo)
	{
		LinkedList * ret = LLcopy(llTwo);
		if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLdifferenceAddr")), false); 
		return ret;
	}
	else if (llOne && !llTwo)
	{
		LinkedList * ret = LLcopy(llOne);
		if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLdifferenceAddr")), false); 
		return ret;
	}

	LinkedList * llDif = getLL();
	if (llDif == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLdifferenceAddr")), false);

	for (LLNode * llnOnePtr = llOne->front; llnOnePtr; llnOnePtr = llnOnePtr->next)
	{
		void * aInSecondList = LLgetItemAddr(llTwo, llnOnePtr->data);
		if (aInSecondList == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLdifferenceAddr")), false);
		if (!aInSecondList)
		{
			Error * e = LLappend(llDif, llnOnePtr->data);
			if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::LLdifferenceAddr")), false);
		}
	}
	return llDif;
}


// Finds the max element in the parameter LinkedList, according to passed
//		in comparing function
// If equal, favors the item with the lower index
void * LLmax(LinkedList * linkedlist, Compare compare)
{
	const string call = "::LLmax";

	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr(call)), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Trying to print a NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr(call)), true);

	if (linkedlist->iLen == 0)
	{
		LinkedList * ret = getLL();
		if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr(call)), false);
		return ret;
	}

	void * max = linkedlist->front->data;
	for (LLNode * llnPtr = linkedlist->front->next; llnPtr; llnPtr = llnPtr->next)
	{
		void * cur = llnPtr->data;
		int comp = compare(cur, max);
		if (comp == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr(call)), false);

		if (comp > 0)
		{
			max = cur;
		}
	}

	return max;

}


Error * printLL(LinkedList * linkedlist, PrintFunc pf)
{
	//Standard validity
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::printLL")), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Trying to print a NULL LinkedList!"), strAddStringFreeBoth(getStringPtr(linkedlist_stack), getStringPtr("::printLL")), true);

	if (linkedlist->iLen <= 0) { printf("EMPTY"); return NULL; }

	//Standard traversal, calling pf, a function that prints the 
	//		needed contents of each node's data, on each item
	LLNode * frontPtr = linkedlist->front;
	while (frontPtr)
	{
		pf(frontPtr->data);
		if (frontPtr->next) printf(" -> ");
		frontPtr = frontPtr->next;
	}
	return NULL;
}



void freeLinkedList(LinkedList* ll)
{
	LLNode * ptr = ll->front;
	while (ptr)
	{
		LLNode * next = ptr->next;

		free(ptr);
		ptr = next;
	}
	free(ll);
}