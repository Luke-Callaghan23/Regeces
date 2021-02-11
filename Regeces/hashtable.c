#include <stdio.h>
#include <stdlib.h>
#include "hashtable.h"
#include "string.h"


const string hashtable_stack = "C.Interpreter.Collections.HashTable.hashtable.c";

HashTable * getHT(int len)
{
    if(len <= 0)
    {
        char buf[200];
        sprintf_s(buf, "Out of bounds Error while trying to create a HashTable; negative length!  Inputted length: %d", len);
        return propagateError(getStringPtr(buf), 
            strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::getHT")), true);
    }
   HashTable * ht = (HashTable *)malloc(sizeof(HashTable));
    if (ht != NULL)
    {
        ht->size = 0;
        ht->len = len;
        ht->keys = getLL();
        if (ht->keys == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::getHT")), false);
        ht->hashTable = (kvPair **)malloc(sizeof(kvPair *) * len);
        if (ht->hashTable != NULL)
        {
            for (int loop = 0; loop < len; loop++)
            {
                *(ht->hashTable + loop) = NULL;
            }
        }
        else
        {
		    return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::getHT")), true);
        }
    }
    else
    {
		return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::getHT")), true);
    }
    return ht;
}



Error * HTinsert(HashTable * table, String * key, void * data)
{

    
	if (table == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::Htinsert")), false);
	if (!table) return propagateError(getStringPtr("NullPointerError!  Trying to insert item into NULL HashTable!"), strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::Htinsert")), true);

	if (key == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::Htinsert")), false);
	if (!key) return propagateError(getStringPtr("NullPointerError!  Trying to insert NULL-keyed item into HashTable!"), strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::Htinsert")), true);


    //Get hash code for item
    uint32_t hashed = SuperFastHash(key->data, key->len);
    
    //Creating insert
    kvPair * in = (kvPair *)malloc(sizeof(kvPair));
	if (in == NULL) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTinsert")), true);
    
    
    //Initializing insert
    in->str  = key;
    in->key  = hashed;
    in->data = data;
    in->next = NULL;

    //Searching set of keys for item current key
    bool inKeys = false;
    kvPair ** keys = LLgetAll(table->keys);
    if (keys == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::Htinsert")), false);

    LinkedList * list = table->keys;
    for (int loop = 0; loop < list->iLen; loop++)
    {
        if (keys[loop]->key == in->key && strequals(keys[loop]->str, key))
        {
            bool bReplaced = LLreplaceItem(list, (*stdEquals), &(keys[loop]), &(in));
            if (bReplaced == error) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTinsert")), true);
            inKeys = true;
            break;
        }
    }
    if (!inKeys)
    {
        Error * e = LLprepend(table->keys, in);
        if (e == error) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTinsert")), true);
    }


    //Incrementing size of table
    table->size += 1;


    //If new size of the table is greater than the capacity of the table
    //      (table len ^ 2)
    //      Then resize the table then insert, otherwise, just insert
    if (table->size >= table->len * table->len)
    {
        //If resizing:
        //      - create new hashtable with 2x the indices of the old one
        //      - take all items in old hash table and put them into a linked list
        //      - for every item in above LL, add to new, larger hash table
        kvPair ** tb = table->hashTable;
        
        ////Adding all items of old hash table to single LL, called start
        kvPair * start  = NULL;
        kvPair * endPtr = NULL;
        for (int loop = 0; loop < table->len; loop++)
        {
            //If kvp LL exists at index, 
            //      add LL to complete LL
            if (tb[loop])
            {
                kvPair * LL   = tb[loop];
                kvPair * ptr  = NULL;
                kvPair * prev = NULL;

                //If first iteration of loop, set start to head of cur LL
                //      set ptr to start
                if (start == NULL)
                {
                    start = LL;
                    ptr = start;
                }
                //else, add cur LL to end of total LL,
                //      set ptr to endPtr
                else
                {
                    endPtr->next = LL;
                    ptr = endPtr;
                }
                //Find new endPtr
                while (ptr)
                {
                    prev = ptr;
                    ptr  = ptr->next;
                }
                //reset end ptr
                endPtr = prev;
            }
        }

        //Shoving inserted item into beginning of full hashtable LL
        in->next = start;
        start = in;

        //Free the old hashtable
        free(table->hashTable);

        //Create new ht of twice the len
        table->len *= 2;
        table->hashTable = (kvPair **)malloc(sizeof(kvPair *) * table->len);
        if (table->hashTable == NULL) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTinsert")), true);
        for (int loop = 0; loop < table->len; loop++)
        {
            (table->hashTable)[loop] = NULL;
        }


        if (table->hashTable)
        {
            //Outer pointer to beginning of total LL
            kvPair * pointer = start;
            while (pointer)
            {
                //Finding index
                int index = pointer->key % table->len;

                //Getting item at index
                kvPair * target = (table->hashTable)[index];


                //If current loop item has the same key as
                //      inserted item, yet it is not the 
                //      inserted item, skip adding it to LL
                //      reduce size of ht because it is a duplicate
                if (pointer->key == in->key && pointer != in)
                {
                    kvPair * replace = pointer;
                    pointer = replace->next;
                    table->size -= 1;
                    continue;
                }

                //If inserting first item at this index
                if (!target)
                {
                    (table->hashTable)[index] = pointer;
                    pointer = pointer->next;
                    ((table->hashTable)[index])->next = NULL;
                }
                else
                {
                    //If other items already exist at this index

                    //Search LL of kvp's for end
                    kvPair * ptr  = (table->hashTable)[index];
                    kvPair * prev = NULL;
                    while (ptr)
                    {
                        prev = ptr;
                        ptr  = ptr->next;
                    }
                    if (prev && pointer)
                    {
                        //At end, place current item
                        prev->next = pointer;
                        //Advance outer LL
                        pointer = pointer->next;
                        //Set current item's next to NULL
                        prev->next->next = NULL;
                    }
                }
            }
        }
    }
    //If there is no need to resize the ht, simply insert the new item in normally
    else
    {
        //Finding index in hash table of inserted item
        int index = hashed % table->len;

        //Storing target for placement
        kvPair * target = (table->hashTable)[index];
            
        //If inserted item is the first item at its index in the ht
        if (!target)
        {
            //Simply set item at index in hashed table to inserted item
            (table->hashTable)[index] = in;
        }
        else
        {
            //Else search for end of kvp LL at index and insert the new item there
            kvPair * ptr  = target;
            kvPair * prev = NULL;
            while (ptr)
            {
                //If the key of the current item in LL == key of insert,
                //      overwrite old key data, decrement table size, and return
                if (ptr->key == in->key)
                {
                    table->size -= 1;
                    ptr->data = in->data;
                    return;
                }
                prev = ptr;
                ptr  = ptr->next;
            }
            //insert item as end's next
            prev->next = in;
        }
    }
    return NULL;
}

//Searches through hash table for keyed item
//      if found, return that kvp
//      else, return NULL
kvPair * HTgetItem(HashTable * table, String * key)
{
	if (table == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HtgetItem")), false);
	if (!table) return propagateError(getStringPtr("NullPointerError!  Trying to insert item into NULL HashTable!"), strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HtgetItem")), true);

    
	if (key == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HtgetItem")), false);
	if (!key) return propagateError(getStringPtr("NullPointerError!  Trying to insert NULL-keyed item into HashTable!"), strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HtgetItem")), true);


    //Getting key's hash
    uint32_t target = SuperFastHash(key->data, key->len);
    
    //Getting index that the item should be in
    int index = target % table->len;

    //Searching through LL at index for a kvp with the 
    //      same key as the target
    kvPair * ptr = (table->hashTable)[index];
    if(ptr != NULL)
    {
        while (ptr)
        {
            if (target == ptr->key)
            {
                return ptr;
            }
            ptr = ptr->next;
        }
        //if not found, return NULL
        return NULL;
    }
    else
    {
        return NULL;
    }
}


HashTable * HTgroupLLBy(LinkedList * linkedlist, GetGroupingElement getGrouping)
{
	if (linkedlist == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTgroupLLBy")), false);
	if (!linkedlist) return propagateError(getStringPtr("NullPointerError!  Trying to insert item into NULL HashTable!"), strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTgroupLLBy")), true);

    if (linkedlist->iLen == 0)
    {
        // If LL is empty, then return an empty HashTable
        HashTable * ht = getHT(2);
        if (ht == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTgroupLLBy")), false);
        return ht;
    }

    // Create HT
    HashTable * ht = getHT(linkedlist->iLen);
    if (ht == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTgroupLLBy")), false);

    

    for (LLNode* llnPtr = linkedlist->front; llnPtr; llnPtr = llnPtr->next)
    {
        // For every item in the LL:
        void * item = llnPtr->data;
        // Get bucket that the item fits in
        String * targetBucket = getGrouping(item);
        // Check if that item fits in any of the existing buckets of the HT
        kvPair * kvp = HTgetItem(ht, targetBucket);
        if (kvp != NULL)
        {
            // If the item does fit into a bucket,
            //      add that item to the bucket
            LinkedList * bucket = (LinkedList *) kvp->data;
            Error * e = LLappend(bucket, item);
            if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTgroupLLBy")), false);
        }
        else
        {
            // else, 
            //      create a new bucket for that item
            // Create an empty LinkedList
            LinkedList * insert = getLL();
            if (insert == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTgroupLLBy")), false);

            // Insert current item into LL
            Error * e = LLappend(insert, item);
            if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTgroupLLBy")), false);

            // insert into HT
            e = HTinsert(ht, targetBucket, insert);
            if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTgroupLLBy")), false);
        }
    }
    // return HT
    return ht;
}


//Searches through hash table for keyed item
//      returns a boolean indicating whther or not the item was found
bool HTsearch(HashTable* table, String* key)
{
	if (table == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTsearch")), false);
	if (!table) return propagateError(getStringPtr("NullPointerError!  Trying to insert item into NULL HashTable!"), strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTsearch")), true);

	if (key == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTsearch")), false);
	if (!key) return propagateError(getStringPtr("NullPointerError!  Trying to insert NULL-keyed item into HashTable!"), strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTsearch")), true);

    //Getting target kvp's hash
    uint32_t target = SuperFastHash(key->data, key->len);
    
    //Getting index that the target kvp should exist at
    int index = target % table->len;

    //Searching through LL at index for kvp with the
    //      same key as the target
    kvPair * ptr = (table->hashTable)[index];
    if (ptr != NULL)
    {
        while (ptr)
        {
            if (target == ptr->key)
            {
                return true;
            }
            ptr = ptr->next;
        }
        //if not found return false
        return false;
    }
    else
    {
        return false;
    }
}

uint32_t SuperFastHash(const char * data, int len)
{
    uint32_t hash = len, tmp;
    int rem;


    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) 
    {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

kvPair ** HTgetAll(HashTable * table)
{
    
	if (table == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTgetAll")), false);
	if (!table) return propagateError(getStringPtr("NullPointerError!  Trying to insert NULL-keyed item into HashTable!"), strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTgetAll")), true);

    kvPair ** kvps = LLgetAll(table->keys);
    if (kvps == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::HTgetAll")), false);
    return kvps;
}


Error * printHT(HashTable * hashtable, PrintFunc pf)
{
	if (hashtable == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::printHT")), false);
	if (!hashtable) return propagateError(getStringPtr("NullPointerError!  Trying to insert NULL-keyed item into HashTable!"), strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::printHT")), true);

    LLNode * llnHTPtr = hashtable->keys->front;
    while (llnHTPtr)
    {
        kvPair * k = llnHTPtr->data;
        String *  str = k->data;

        kvPair * data = HTgetItem(hashtable, ((kvPair *) llnHTPtr->data)->str);
	    if (data == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(hashtable_stack), getStringPtr("::printHT")), false);

        printf("'%s' -> ", ((kvPair *) llnHTPtr->data)->str->data);
        pf(data->data);
        printf("\n");

        llnHTPtr = llnHTPtr->next;
    }
    return NULL;

}