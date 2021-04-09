#ifndef HASH_TABLE_H 
#define HASH_TABLE_H 0

#include <inttypes.h>
#include <stdint.h>
#include "CLI.h"
#include "linkedlist.h"


#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

#define kvPair struct _kvp
#define HashTable struct _ht

struct _kvp 
{
    String * str;
    uint32_t key;
    void * data;
    struct _kvp * next;
};


struct _ht 
{
    int size;
    int len;
    kvPair ** hashTable;
    LinkedList * keys;
};

typedef String*(*GetGroupingElement)(void*);

uint32_t SuperFastHash(const char *, int);
HashTable * getHT(int);
Error * HTinsert(HashTable *, String *, void *);
bool HTsearch(HashTable *, String *);
kvPair *  HTgetItem(HashTable *, String *);
kvPair ** HTgetAll(HashTable *);
Error * printHT(HashTable *, PrintFunc);
HashTable * HTgroupLLBy(LinkedList *);


void freeHashTable(HashTable *);

#endif 
