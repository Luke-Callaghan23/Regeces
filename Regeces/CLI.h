#ifndef INTERPRETER_H
#define INTERPRETER_H 0

#include <stdlib.h>
#define string char*
#define String struct _str
#define bool int
#define true 1
#define false 0
#define MAX_LINE 250
//#define ERROR 0x00000069

enum res_type { ERROR, RESULT };

typedef struct err {
	String * errMessage;
	String * trace;
	String * origin;
} Error;


extern Error * error;
Error * propagateError(String *, String *, bool);
Error * printError(Error *);


const enum eNotFound { NOT_FOUND = -1 };
typedef bool(*Predicate)(void*, void*);
typedef void(*PrintFunc)(void *);
typedef int(*Compare)(void *, void *);

bool stdEquals(void *, void *);
void stdPrint(void *);
void stdPrintHex(void *);
int  indexConvert(int, int);

#endif // !INTERPRETER_H
