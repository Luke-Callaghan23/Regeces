#include <stdio.h>
#include <stdlib.h>
#include "string.h"

//Ranges for lower-case and upper-case characters respectively
const int lowerRange[] = { 97, 122 };
const int upperRange[] = { 65,  90 };

const string string_stack = "C.Interpreter.Collections.String.string.c";

//Simple getter that recieves a basic char * string, and returns a struct Str String
String getString(const string original)
{
	String result = { .data = original, .len = str_len(original) };
	return result;
}

//Simple getter that recieves a basic char * string, and returns a struct Str String pointer
String* getStringPtr(const string original)
{
	String* result = (String *)malloc(sizeof(String));
	if (result != NULL)
	{
		result->data = original;
		result->len  = str_len(original);
	}
	else
	{
		result->data = "";
		result->len  = 0;
	}
	return result;
}

String * getStringFromChar(char str)
{
	
	string s = (string)malloc(sizeof(char) * 2);
	if (!s) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::getStringFromChar")), true);

	String * ret = (String *)malloc(sizeof(String));
	if (!ret) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::getStringFromChar")), true);

	ret->data = s;
	s[0] = str;
	s[1] = '\0';
	ret->len  = 1;

	return ret;
}



//Gets string length
int str_len(string str)
{
	if (str == error) return propagateError(NULL, getStringPtr(string_stack), false);
	if (str == NULL) return propagateError(getStringPtr("NullPointerError!  Trying to read Length of null String!"), strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::str_len")), true);
	int len;
	for(len = 0; str[len] != '\0'; len++);
	return len;
}
//Gets string equality 
bool strequals(String *first, String *other)
{
	if (first == error || other == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::streqauls")), false);
	if (!first || !other) return false;

	//If len is not equal, return false
	if (first->len != other->len) return false;

	//If the lengths of the two Strings are equal, and
	//		and one of the lengths is zero, then both are
	//		empty strings
	if (first->len == 0) return true;
	
	//Grabbing 'string' stata from Strings
	string sFirstData = first->data;
	string sOtherData = other->data;

	//Comparing sFirstData and sOtherData
	int loop;
	for (loop = 0; sFirstData[loop] != '\0'; loop++)
	{
		if (sFirstData[loop] != sOtherData[loop])
		{
			return false;
		}
	}

	//If never returned, return true
	return true;
}
//Copies and returns a string
String *strCopy(String *original)
{
	if (original == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::strCopy")), false);

	//Grabbing original's length
	int len = original->len;
	//Callocing new string
	string ret = (string)calloc(sizeof(char), len);
	if (ret != NULL)
	{
		int loop;
		for (loop = 0; loop < len; loop++)
		{
			ret[loop] = (original->data)[loop];
		}
		ret[len] = '\0';
	}
	String * res = getStringPtr(ret);

	if (res == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::strCopy")), false);

	return res;
}
//Returns a newly-allocated string where all lower-case letters in "original"
//		become upper-case
String *toUpper(String *original)
{
	if (original == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::toUpper")), false);
	//Copying String *
	String *writeTo = strCopy(original);
	if (writeTo == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::toUpper")), false);


	//Storing string data
	string sWriteToData  = writeTo->data;
	string sOriginalData = original->data;

	int loop;
	for (loop = 0; loop < original->len; loop++)
	{
		//Checking if current char is in lower-case range
		if (sOriginalData[loop] >= lowerRange[0] && sOriginalData[loop] <= lowerRange[1])
		{
			sWriteToData[loop] -= 32;
		}
	}
	return writeTo;
}
//Returns a newly-allocated string where all upper-case letters in "original"
//		become lower-case
String *toLower(String *original)
{
	if (original == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::toLower")), false);
	//Copying String *
	String *writeTo = strCopy(original);

	if (writeTo == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::toLower")), false);

	//Storing string data
	string sWriteToData  = writeTo->data;
	string sOriginalData = original->data;

	int loop;
	for (loop = 0; loop < original->len; loop++)
	{
		//Checking if current char is in upper-case range
		if (sOriginalData[loop] >= upperRange[0] && sOriginalData[loop] <= upperRange[1])
		{
			sWriteToData[loop] += 32;
		}
	}
	return writeTo;
}


String * strAddStringFreeBoth(String * begin, String * append)
{
	if (begin == error || append == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::strAddStringFreeBoth")), false);

	if (begin == NULL && append != NULL)
	{
		String * ret = strCopy(append);
		free(append);
		return ret;
	}
	else if (begin != NULL && append == NULL)
	{
		String * ret = strCopy(begin);
		free(begin);
		return ret;
	}
	else if (begin != NULL && append != NULL)
	{
		int iNewLen = (begin->len + append->len);
		string fuse = (string) calloc(sizeof(char), iNewLen + 1);
		
		for (int iLoop = 0; iLoop < begin->len; iLoop++)
		{
			fuse[iLoop] = begin->data[iLoop];
		}
		for (int iLoop = begin->len; iLoop < append->len + begin->len; iLoop++)
		{
			fuse[iLoop] = append->data[iLoop - begin->len];
		}
		free(begin);
		free(append);

		String * ret = getStrKnownLen(fuse, iNewLen);

		ret->data[iNewLen] = 0;

		if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::strAddStringFreeBoth")), false);

		return ret;
	}
	else if (begin == NULL && append == NULL)
	{
		return getStringPtr("");
	}

}

String * getStrKnownLen(string data, int len)
{
	if (data == error) return propagateError(NULL, getStringPtr(string_stack), false);
	String * res = (String *)malloc(sizeof(String));
	if (!res) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::getStrKnownLen")), true);

	res->data = data;
	res->len = len;

	return res;
}



//Returns a substring of String str, starting from (inclusive) @param from
//		and going to (exclusive) @param to
String * substring(String * str, int from, int to)
{
	if (str == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::substring")), false);

	if (str == NULL) return propagateError(getStringPtr("Nullpointer Exception!  Attempting to create substring from NULL String!"), strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::strAddStringFreeBoth")), true);

	if (from > to)
	{
		string buf = (string) calloc(sizeof(char), 300);
		sprintf_s(buf, "Out of Bounds error!  Lower bound is larger than upper bound!  Lower bound: %d.  Upper bound: %d.  String: %s", from, to, str->data);
		return propagateError(getStringPtr(buf), strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::substring")), true);
	}
	if (from == to) return getStringPtr("");
	if (from < 0)
	{
		string buf = (string) calloc(sizeof(char), 300);
		sprintf_s(buf, "Out of Bounds error!  Lower bound is negative!  Upper bound: %d.  Lower bound: %d.  String: %s", from, to, str->data);
		return propagateError(getStringPtr(buf), strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::substring")), true);
	}
	if (to > str->len)
	{
		
		string buf = (string) calloc(sizeof(char), 300);
		sprintf_s(buf, "Out of Bounds error!  Upper bound exceeds the length of the target String!  Upper bound: %d.  Lower bound: %d.  String: %s", from, to, str->data);
		return propagateError(getStringPtr(buf), strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::substring")), true);
	}
	//Size of substring to be constructed
	int range = to - from;

	//Allocate new substring array
	string data = (string)malloc(sizeof(char) * (range + 1));
	if (!data) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::substring")), true);

	//Setting last byte of substring to null char
	//		because, annoyingly, that's how strings 
	//		work in C
	data[range] = '\0';

	//Main loop, copying chars from input str
	//		to newly allocated string buffer
	for (int loop = from; loop < to; loop++)
	{
		data[loop - from] = str->data[loop];
	}

	String * ret = getStrKnownLen(data, range);

	if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::substring")), false);

	return ret;
}


bool isSubstring(String * s1, String * s2)
{
	if (s1 == NULL) return propagateError(getStringPtr("NullPointerError!  Trying to retrieve substring of NULL String!"), strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::isSubstring")), true);
	if (s2 == NULL) return propagateError(getStringPtr("NullPointerError!  Trying to retrieve substring of NULL String!"), strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::isSubstring")), true);
	
	if (s1 == error || s2 == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::printStringLong")), false);

	
	//If s1 == s2, then obviously they are substrings
	if (strequals(s1, s2)) return true; 
	
	String * largeBoi = NULL,
		   * smollBoi = NULL;

	//Because this function does not test if s1 is a substring of
	//		s2 or vice-versa, and it only checks if either one 
	//		is a substring of the other, we need to set the 
	//		bigger and smaller strings accordingly
	if (s1->len > s2->len) 
	{ 
		largeBoi = s1; 
		smollBoi = s2; 
	}
	else 
	{ 
		largeBoi = s2; 
		smollBoi = s1; 
	}


	for (int outer = 0; outer < largeBoi->len; outer++)
		//For every character of the larger string:
		for (int inner = 0; 
			//Loop continues for as long as the inner + outer < largeBoi length, inner < smollBoi length
			//		and smollBoi character at inner is equal to the current outer loop character of largeBoi + inner
			inner < smollBoi->len && inner + outer < largeBoi->len && smollBoi->data[inner] == largeBoi->data[inner + outer]; 
			inner++)
				//If the loop makes it to the end of smollBoi's length, that means the 
				//		entirety of smollBoi can be contained in largeBoi
				//This time, only wrote *most* of the loop in the for loop's header
				if (inner == smollBoi->len - 1) return true;

	//If no spot was found where all of smollBoi 
	//		could be fit inside of largeBoi, then 
	//		return false
	return false;
}


Error * printString(void * vStr)
{
	if (vStr == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::printString")), false);
	if (vStr == NULL)  return propagateError(getStringPtr("NullPointerError!  Trying to print NULL String!"), strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::printString")), true);

	String * sStr = (String *)vStr;
	printf("%s", sStr->data);
	return NULL;
}
Error * printStringLong(void * vStr)
{
	if (vStr == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::printStringLong")), false);
	if (vStr == NULL)  return propagateError(getStringPtr("NullPointerError!  Trying to print NULL String!"), strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::printStringLong")), true);

	String * sStr = (String *)vStr;
	printf("Data: %s\nLen: %d", sStr->data, sStr->len);
	return NULL;
}

Error * freeStr(String * str)
{

	if (str == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::freeStr")), false);
	if (str == NULL)  return propagateError(getStringPtr("NullPointerError!  Trying to free NULL String!"), strAddStringFreeBoth(getStringPtr(string_stack), getStringPtr("::freeStr")), true);

	if (str->len == 0) return NULL;

	free(str->data);
	free(str);
	return NULL;
}