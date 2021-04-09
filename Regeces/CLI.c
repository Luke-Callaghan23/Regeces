#include <stdio.h>
#include <stdlib.h>
#include "CLI.h"
#include "string.h"
#include "linkedlist.h"
#include "arraylist.h"
#include "hashtable.h"
#include "regex.h"

void htTest();
void reTest();
void llTest();
void alTest();

bool stdEquals(void * element1, void * element2) { return element1 == element2; }
void stdPrint(void * data) { printf("%d", (int)data); }
void stdPrintHex(void * data) { printf("0x%x", (int)data); }

int main(int argc, char** argv)
{
	//initializing the global 'error' variable that 
	//		is used constantly throughout the program
	propagateError(NULL, NULL, false);
	
	
	//If testing LinkedList implementation
	//llTest();

	//If testinf ArrayList implementation
	//alTest();

	//If testing hash table implementation
	//htTest();

	//No command-line arguments
	//Run command interpreter in command line

	int iLineCtr;
	string sline;
	String * sLineStr;
	const String * sQuitStr = getStringPtr("!QUIT");
	const String * sHelpStr = getStringPtr("!HELP");
	const String * sEmptStr = getStringPtr("");

	printf("Welcome to my, Luke Callaghan's, regex engine!\n\n============================================================================================\n\nThis engine was entirely built in C, using data structures I built myself.  These data structures include:\n   - String\n    - LinkedList\n   - ArrayList\n   - HashTable\n\n============================================================================================\n\n");

	const string helpString = "\nTo play with my data structures / regex engine, use:\n   - LinkedList --> 'll'\n   - ArrayList --> 'al'\n   - HashTable --> 'ht'\n   - Regex --> 're'\n\n[Type '!help' for help, and '!quit' to quit]\n\n";

	printf(helpString);


	for (iLineCtr = 0; true; iLineCtr++)
	{
		printf(">  ");							                //line prompt
		sline = (string)calloc(sizeof(char), MAX_LINE);			//allocating space for next string and initializing to 0
		fgets(sline, MAX_LINE, stdin);							//reading stdin
			

		//Converting to struct Str data-type
		sLineStr = getStringPtr(sline);

		//Removing endline character from the end on inputed String
		(sLineStr->data)[sLineStr->len - 1] = '\0';
		(sLineStr->len)--;


		//If user types '!q', or '!quit' (case insensitive), quit the program with EXIT_SUCCESS
		if (strequals(sQuitStr, toUpper(sLineStr)) || ((sline[0] == '!' && sline[1] == 'q' || sline[1] == 'Q') && sline[2] == '\0'))
		{
			freeString(sLineStr);								    //freeing line String
			break;
		}

		if (strequals(sHelpStr, toUpper(sLineStr)) || ((sline[0] == '!' && sline[1] == 'h' || sline[1] == 'H') && sline[2] == '\0'))
		{
			printf(helpString);
		}

		// re --> regex
		if (((sline[0] == 'r' && sline[1] == 'e') || (sline[0] == 'R' || sline[1] == 'E')) && sline[2] == '\0')
		{
			reTest();
			printf(helpString);
		}
		// ll --> linkedlist
		else if (((sline[0] == 'l' && sline[1] == 'l') || (sline[0] == 'L' || sline[1] == 'L')) && sline[2] == '\0')
		{
			llTest();
			printf(helpString);
		}
		// al --> arraylist
		else if (((sline[0] == 'a' && sline[1] == 'l') || (sline[0] == 'A' || sline[1] == 'L')) && sline[2] == '\0')
		{
			alTest();
			printf(helpString);
		}
		// ht --> hashtable
		else if (((sline[0] == 'h' && sline[1] == 't') || (sline[0] == 'H' || sline[1] == 'T')) && sline[2] == '\0')
		{
			htTest();
			printf(helpString);
		}


		freeString(sLineStr);								    //freeing line String
	}

	
	return EXIT_SUCCESS;
}


void llTest()
{
	string sline;
	String * sLineStr;
	const String * sQuitStr = getStringPtr("!QUIT");
	const String * sStatStr = getStringPtr("!STATS");
	const String * sHelpStr = getStringPtr("!HELP");
	const String * sEmptStr = getStringPtr("");

	LinkedList * ll = getLL();

	const string helpString = "\nLinkedList operations:\n   - 'b' --> insert a string into the [b]ack of the LinkedList\n   - 'f' --> insert a string into the [f]ront of the LinkedList\n   - 'r' --> [r]emove the first instance of a string from the LinkedList\n   - 'R' --> [R]emove an item at an index from the LinkedList\n   - 's' --> [s]earch for a string in the LinkedList\n   - 'S' --> [S]earch for a string at an index in the LinkedList\n\n[Type '!help' for help, and '!quit' to quit]\n\n";
	printf(helpString);


	while (true)
	{
		printf(">  ");
		sline = (string)calloc(sizeof(char), MAX_LINE);
		fgets(sline, MAX_LINE, stdin);



		//Converting to struct Str data-type
		sLineStr = getStringPtr(sline);

		//Removing endline character from the end on inputed String
		(sLineStr->data)[sLineStr->len - 1] = '\0';
		(sLineStr->len)--;


		
		if (strequals(sEmptStr, sLineStr))
		{
			freeString(sLineStr);
			continue;
		}

			
		if (strequals(sHelpStr, toUpper(sLineStr)) || ((sline[0] == '!' && sline[1] == 'h' || sline[1] == 'H') && sline[2] == '\0'))
		{
			printf(helpString);
		}

		//If user types '!q', or '!quit' (case insensitive), quit the program with EXIT_SUCCESS
		if (strequals(sQuitStr, toUpper(sLineStr)) || ((sline[0] == '!' && sline[1] == 'q' || sline[1] == 'Q') && sline[2] == '\0'))
		{
			freeString(sLineStr);
			break;
		}

		if (sLineStr->len > 2)
		{
			if (strequals(sStatStr, sLineStr))
			{
				printf("Len: %d\n",  ll->iLen);
			}
			else
			{
				switch ((sLineStr->data)[0])
				{
					case 'b':
					{
						string sSubstring = (sLineStr->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						printf("Storing '%s' in linkedlist!\n", sKey->data);
						LLappend(ll, sKey);
						break;
					}
					case 'f':
					{
						string sSubstring = (sLineStr->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						printf("Storing '%s' in linkedlist!\n", sKey->data);
						LLprepend(ll, sKey);
						break;
					}
					case 'r':
					{
						string sSubstring = (sLineStr->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						printf("Removing '%s' from linkedlist!\n", sSubstring);
						String ** items = LLgetAll(ll);
						String *  found = false;
						for (int loop = 0; loop < ll->iLen; loop++)
						{
							String ** s = items + loop;
							String * current = *(items + loop);
							if (strequals(current, sKey))
							{
								found = current;
								break;
							}
						}

						bool rem = LLremoveItemAddr(ll, found);

						if (rem)
						{
							printf("'%s' was removed!\n", sKey->data);
						}
						else
						{
							printf("'%s' was not removed :(\n", sKey->data);
						}
						break;
					}
					case 'R':
					{
						string sSubstring = (sLineStr->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						int ind = atoi(sSubstring);
						printf("Removing index '%d' from linkedlist!\n", ind);
						

						bool rem = LLremoveItemIndex(ll, ind);

						if (rem)
						{
							printf("'%s' was removed!\n", sKey->data);
						}
						else
						{
							printf("'%s' was not removed :(\n", sKey->data);
						}
						break;
					}
					case 's':
					{
						string sSubstring = (sLineStr->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						printf("Searching '%s' in linkedlist!\n", sSubstring);
						String ** items = LLgetAll(ll);
						bool found = false;
						for (int loop = 0; loop < ll->iLen; loop++)
						{
							String ** s = items + loop;
							String * current = *(items + loop);
							if (strequals(current, sKey))
							{
								found = true;
								break;
							}
						}
						if (found)
						{
							printf("'%s' was found!\n", sKey->data);
						}
						else
						{
							printf("'%s' was not found :(\n", sKey->data);
						}
						break;
					}
					case 'S':
					{
						string sSubstring = (sLineStr->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						int ind = atoi(sSubstring);
						printf("Finding index '%d' from linkedlist!\n", ind);
						String ** items = LLgetAll(ll);
						String * rem = LLgetItemIndex(ll, ind);
						if (rem)
						{
							printf("'%d' was found: %s!\n", ind, rem->data);
						}
						else
						{
							printf("'%d' was not found :(\n", ind);
						}
						break;
					}
					default:
					{
						printf("Invalid Command!\n");
						printf("Enter only 'i', 'g', or 's' followed by keyword argument.\n");
						break;
					}
				}

			}
		}
		else
		{
			printf("Invalid Command!\n");
			printf("Enter only 'i', 'g', or 's' followed by keyword argument.\n");
		}


		free(sLineStr);
	}
	freeLinkedList(ll);
}
void alTest()
{
	string sline;
	String * sLineStr;
	const String * sQuitStr = getStringPtr("!QUIT");
	const String * sStatStr = getStringPtr("!STATS");
	const String * sHelpStr = getStringPtr("!HELP");
	const String * sEmptStr = getStringPtr("");
	ArrayList * al = getAL(4);

	
	const string helpString = "\nArrayList operations:\n   - 'b' --> insert a string into the [b]ack of the ArrayList\n   - 'f' --> insert a string into the [f]ront of the ArrayList\n   - 'r' --> [r]emove the first instance of a string from the ArrayList\n   - 'R' --> [R]emove an item at an index from the ArrayList\n   - 's' --> [s]earch for a string in the ArrayList\n   - 'S' --> [S]earch for a string at an index in the ArrayList\n\n[Type '!help' for help, and '!quit' to quit]\n\n";
	printf(helpString);



	while (true)
	{
		printf(">  ");
		sline = (string)calloc(sizeof(char), MAX_LINE);
		fgets(sline, MAX_LINE, stdin);



		//Converting to struct Str data-type
		sLineStr = getStringPtr(sline);

		//Removing endline character from the end on inputed String
		(sLineStr->data)[sLineStr->len - 1] = '\0';
		(sLineStr->len)--;

		
		if (strequals(sEmptStr, sLineStr))
		{
			freeString(sLineStr);
			continue;
		}

		//If user types '!q', or '!quit' (case insensitive), quit the program with EXIT_SUCCESS
		if (strequals(sQuitStr, toUpper(sLineStr)) || ((sline[0] == '!' && sline[1] == 'q' || sline[1] == 'Q') && sline[2] == '\0'))
		{
			freeString(sLineStr);
			break;
		}
			
		if (strequals(sHelpStr, toUpper(sLineStr)) || ((sline[0] == '!' && sline[1] == 'h' || sline[1] == 'H') && sline[2] == '\0'))
		{
			printf(helpString);
		}
		/*
a 1
a 2
a 3
a 4
a 5
a 6
p 7
p 8
		

a 4
a 3
a 2
a 1
r 1
!stats
		



a 1
a 2
a 1
a 1
a 3
a 3
a 2
!stats


a 1
a 2
a 1
a 2
p 2
p 1
a 1
a 3
t  

		*/
		if (sLineStr->len > 2)
		{
			if (strequals(sStatStr, sLineStr))
			{
				printf("\n");
				printf("Len: %d\n",  al->iLen);
				printf("ArrLen: %d\n", al->iArrLen);
				printf("OccuLen: %d\n", al->iOccuLen);
				printf("PreLen: %d\n", al->preList->iLen);
				printf("PostLen: %d\n", al->postList->iLen);
				printf("\n");
				printAL(al, (*stdPrint));
				printf("\n\n");
			}
			else
			{
				switch ((sLineStr->data)[0])
				{
					//a -> append
					case 'b':
					{
						string sSubstring = (sLineStr->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						printf("Appending '%s' to arraylist!\n", sKey->data);
						int i = atoi(sSubstring);
						ALappend(al, i);
						break;
					}
					//p -> prepend
					case 'f':
					{
						string sSubstring = (sLineStr->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						printf("Prepending '%s' to arraylist!\n", sKey->data);
						int i = atoi(sSubstring);
						ALprepend(al, i);
						break;
					}
					//r -> remove item by val
					case 'r':
					{
						string sSubstring = (sLineStr->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						printf("Removing '%s' from arraylist!\n", sSubstring);
						

						bool rem = ALremoveItemAddr(al, atoi(sSubstring));

						if (rem)
						{
							printf("'%s' was removed!\n", sKey->data);
						}
						else
						{
							printf("'%s' was not removed :(\n", sKey->data);
						}
						break;
					}
					//R -> remove item by index
					case 'R':
					{
						string sSubstring = (sLineStr->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						int ind = atoi(sSubstring);
						printf("Removing index '%d' from arraylist!\n", ind);
						

						bool rem = ALremoveItemIndex(al, ind);

						if (rem)
						{
							printf("'%s' was removed!\n", sKey->data);
						}
						else
						{
							printf("'%s' was not removed :(\n", sKey->data);
						}
						break;
					}
					//g -> get item by index
					case 's':
					{
						string sSubstring = (sLineStr->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						int ind = atoi(sSubstring);
						printf("Finding index '%d' from arraylist!\n", ind);
						
						int found = ALgetItemIndex(al, ind);

						if (found)
						{
							printf("'%d' was found: %d!\n", ind, found);
						}
						else
						{
							printf("'%d' was not found :(\n", ind);
						}
						break;
					}
					//s -> search item by value
					case 'S':
					{
						string sSubstring = (sLineStr->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						printf("Searching '%s' in arraylist!\n", sSubstring);

						int found = ALgetItemAddr(al, atoi(sSubstring));

						if (found)
						{
							printf("'%s' was found!\n", sKey->data);
						}
						else
						{
							printf("'%s' was not found :(\n", sKey->data);
						}
						break;
					}
					default:
					{
						printf("Invalid Command!\n");
						printf("Enter only 'i', 'g', or 's' followed by keyword argument.\n");
						break;
					}
				}

			}
		}
		else
		{
			printf("Invalid Command!\n");
			printf("Enter only 'i', 'g', or 's' followed by keyword argument.\n");
		}


		free(sLineStr);
	}
	
	freeHashTable(al);
}
void htTest()
{
	string sline;
	String * sLineStr;
	const String * sQuitStr = getStringPtr("!QUIT");
	const String * sStatStr = getStringPtr("!STATS");
	const String * sHelpStr = getStringPtr("!HELP");
	const String * sEmptStr = getStringPtr("");
	
	HashTable * ht = getHT(2);

	
	const string helpString = "\nHashTable operations:\n   - 'i' --> [i]nsert a string into the HashTable, with a random integer value\n   - 's' --> [s]earch for a key in the HashTable\n\n[Type '!help' for help, and '!quit' to quit]\n\n";
	printf(helpString);



	while (true)
	{
		printf(">  ");
		sline = (string)calloc(sizeof(char), MAX_LINE);
		fgets(sline, MAX_LINE, stdin);


		//Converting to struct Str data-type
		sLineStr = getStringPtr(sline);

		//Removing endline character from the end on inputed String
		(sLineStr->data)[sLineStr->len - 1] = '\0';
		(sLineStr->len)--;

		
		if (strequals(sEmptStr, sLineStr))
		{
			freeString(sLineStr);
			continue;
		}

		if (strequals(sHelpStr, toUpper(sLineStr)) || ((sline[0] == '!' && sline[1] == 'h' || sline[1] == 'H') && sline[2] == '\0'))
		{
			printf(helpString);
		}

		//If user types '!q', or '!quit' (case insensitive), quit the program with EXIT_SUCCESS
		if (strequals(sQuitStr, toUpper(sLineStr)) || ((sline[0] == '!' && sline[1] == 'q' || sline[1] == 'Q') && sline[2] == '\0'))
		{
			freeString(sLineStr);
			break;
		}

		if (strequals(sHelpStr, toUpper(sLineStr)) || ((sline[0] == '!' && sline[1] == 'h' || sline[1] == 'H') && sline[2] == '\0'))
		{
			printf(helpString);
		}

		if (sLineStr->len > 2)
		{
			if (strequals(sStatStr, sLineStr))
			{
				printf("Len: %d\n",  ht->len);
				printf("Size: %d\n", ht->size);
			}
			else
			{
				switch ((sLineStr->data)[0])
				{
					case 'i':
					{
						string sSubstring = (sLineStr->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						int iVal = rand();
						printf("Storing %d in hashtable!\n", iVal);
						HTinsert(ht, sKey, &iVal);
						break;
					}
					case 's':
					{
						string sSubstring = (sLineStr->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						printf("Searching '%s' in hashtable!\n", sSubstring);
						kvPair * kvp = HTgetItem(ht, sKey);
						if (kvp)
						{
							printf("Value: %d\n", *(int *)(kvp->data));
						}
						else
						{
							printf("'%s' not found!\n", sSubstring);
						}
						break;
					}
					default:
					{
						printf("Invalid Command!\n");
						printf("Enter only 'i', 'g', or 's' followed by keyword argument.\n");
						break;
					}
				}
			}
		}
		else
		{
			printf("Invalid Command!\n");
			printf("Enter only 'i', 'g', or 's' followed by keyword argument.\n");
		}


		freeString(sLineStr);
		free(sline);
	}
	
	freeHashTable(ht);
}
void reTest()
{
	string sline;
	String * sLineStr;
	const String * sQuitStr = getStringPtr("!QUIT");
	const String * sStatStr = getStringPtr("!STATS");
	const String * sHelpStr = getStringPtr("!HELP");
	const String * sEmptStr = getStringPtr("");


	HashTable * ht = getHT(2);

	string sRegex;
	string slineag;

	const string helpString = "\nRegex operations:\n   - 'nr' --> insert a [n]ew [r]egex into the regex database\n   - 'ts' --> [t]est a [s]tring against every string in the database\n\n[Type '!help' for help, and '!quit' to quit]\n\n";
	printf(helpString);


	while (true)
	{
		printf(">  ");
		sline = (string)calloc(sizeof(char), MAX_LINE);
		fgets(sline, MAX_LINE, stdin);



		//Converting to struct Str data-type
		sLineStr = getStringPtr(sline);

		//Removing endline character from the end on inputed String
		(sLineStr->data)[sLineStr->len - 1] = '\0';
		(sLineStr->len)--;


		if (strequals(sEmptStr, sLineStr))
		{
			freeString(sLineStr);
			continue;
		}
			
		if (strequals(sHelpStr, toUpper(sLineStr)) || ((sline[0] == '!' && sline[1] == 'h' || sline[1] == 'H') && sline[2] == '\0'))
		{
			printf(helpString);
		}

		//If user types '!q', or '!quit' (case insensitive)
		if (strequals(sQuitStr, toUpper(sLineStr)) || ((sline[0] == '!' && sline[1] == 'q' || sline[1] == 'Q') && sline[2] == '\0'))
		{
			freeString(sLineStr);
			break;
		}
		if (sLineStr->len > 2)
		{
			//Adding a regex
			if ((sLineStr->data)[0] == 'n' && (sLineStr->data)[1] == 'r' && (sLineStr->data)[2] == ' ')
			{
				//Regex * taggedRegex = getRegex(getStringPtr("nr \".+\" /.*/"));

				//if (!regexFullMatch(taggedRegex, sLineStr))
				{
					string srem = (sLineStr->data) + 3;
					slineag = (string) malloc(sizeof(char) * 5);
					sprintf_s(slineag, 5, "%d", ht->size + 1);
					sRegex = srem;
					printf("Adding:\n");
					printf("\tRegex: %s\n", sRegex);
					printf("\tTag: %s\n", slineag);

					String * sReg = getStringPtr(sRegex);
					String * sline = getStringPtr(slineag);

					Regex * reg = getRegex(sReg);

					if (reg == error)
					{
						printError(reg, -1, -1);
						printf("\n");
						continue;
					}
					HTinsert(ht, sline, reg);
				}

			}
			//Testing a regex
			else if ((sLineStr->data)[0] == 't' && (sLineStr->data)[1] == 's' && (sLineStr->data)[2] == ' ')
			{
				
				string srem = (sLineStr->data) + 3;
				String * sRem = getStringPtr(srem);

				kvPair ** table = HTgetAll(ht);
				int iTableSize  = ht->size;

				Regex * match;
				bool fm;

				//printf("here\n\n");
				LinkedList * fulls = getLL();

				for (int loop = 0; loop < iTableSize; loop++)
				{
					//void * k = (table[loop])->data;
					match = (Regex *)((table[loop])->data);
					fm = regexFullMatch(match, sRem);

					
					if (fm == error)
					{
						printError(fm, -1, -1);
						printf("\n");
						continue;
					}

					if (fm)
					{
						LLprepend(fulls, table[loop]);
					}
				}


				if (fulls->iLen)
				{
					void ** matches = LLgetAll(fulls);
					for (int loop = 0; loop < fulls->iLen; loop++)
					{
						printf("Match! with regex: %s\n", ((Regex *)((kvPair *)(matches[loop]))->data)->regex->data);
					}
				}
				else
				{
					printf("No matches :(\n");
				}
			}
			else
			{
				printf("Invalid2input!\n");
				continue;
			}
		}
		else
		{
			printf("Invalid1input!\n");
		}

		
		freeString(sLineStr);
	}
	
	freeHashTable(ht);

}



int indexConvert(int index, int len)
{
	//Converting integer index to an index that fits into the 
	//		a number that fits between 0 and @param len - 1
	if (index < 0)
	{
		index += len;
		if (index < 0)
			return indexConvert(index, len);
	}
	else
	{
		index %= len;
	}
	return index;
}

const string interpreter_stack = "CLI.c";

Error * error;
Error * propagateError(String * errMessage, String * addTrace, bool origin)
{
	if (error == NULL) 
	{
		//If the global error pointer does not exist, create it
		//		and initialize the data to NULL
		error = (Error *) malloc(sizeof(Error));
		if (error)
		{
			error->errMessage = NULL;
			error->trace      = NULL;
			error->origin     = NULL;
		}
	}
	if (origin)
	{
		//If this is the origin of the error, we need to first free the 
		//		old data before we set the new data
		if (error->errMessage) free(error->errMessage);
		if (error->trace) free(error->trace);
		if (error->origin) free(error->origin);

		//Setting origin call of the error
		error->origin = strCopy(addTrace);

		if (errMessage != NULL)
		{
			error->errMessage = errMessage;
		}

		if (addTrace == NULL)
		{
			//If there is no trace, the trace is simply "main"
			error->trace = "main";
		}
		else
		{
			// Otherwise, the trace is trace
			error->trace = addTrace;
		}
	}
	else
	{
		if (errMessage != NULL)
		{
			// If the error message is not NULL, we add add a newline and tab character to the 
			//		current error message, then add the new error message
			error->errMessage = strAddStringFreeBoth(error->errMessage, getStrKnownLen("\n\t", 2));
			error->errMessage = strAddStringFreeBoth(error->errMessage, addTrace);
		}
		if (addTrace != NULL)
		{
			// If the trace is not NULL, add newline and tab to current trace,
			//		then add the new stack strace
			error->trace = strAddStringFreeBoth(error->trace, getStrKnownLen("\n\t", 2));
			error->trace = strAddStringFreeBoth(error->trace, addTrace);
		}
	}

	return error;
}
Error * printError(Error * e, int line, int col)
{
	if (e == NULL) return propagateError(getStringPtr("NullPointerError!  Trying to read Length of null String!"), strAddStringFreeBoth(getStringPtr(interpreter_stack), getStringPtr("::printError")), true);

	if (line != -1 && col != -1)
	{
		printf("Error found on line: %d, character: %d!\n", line, col);
	}
	if (line != -1 && col == 1)
	{
		printf("Error found on line: %d!\n", line);
	}

	printf("ORIGIN: %s\n", e->origin->data);
	printf("ERROR: %s\n", e->errMessage->data);
	printf("TRACE: \n%s", e->trace->data);

}