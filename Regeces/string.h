#ifndef STRING_H
#define STRING_H 0

#include "CLI.h"

struct _str 
{
	int len;
	string data;
};

String getString(string);
String * getStringPtr(string);
Error * freeStr(String *);
int str_len(string);
bool strequals(String *, String *);
String * strCopy(String *);
String * toUpper(String *);
String * toLower(String *);
String * substring(String *, int, int);
String * getStringFromChar(char);
bool isSubstring(String *, String *);
Error * printString(void *);
Error * printStringLong(void *);
String * strAddStringFreeBoth(String *, String *);
String * getStrKnownLen(string, int);

void freeString(String *);


#endif // !STRING_H
