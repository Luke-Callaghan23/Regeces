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
	
	
	//printf("Num arguments: %d\n", argc);

	//If testing LinkedList implementation
	//llTest();

	//If testinf ArrayList implementation
	//alTest();

	//If testing hash table implementation
	//htTest();

	//If testing regex implementation
	reTest();

	if (argc == 1)
	{




		//No command-line arguments
		//Run command interpreter in command line

		int iLineCtr;
		string sline;
		String * sLineStr;
		const String * sQuitStr = getStringPtr("!QUIT");



		for (iLineCtr = 0; true; iLineCtr++)
		{
			printf("%d : >  ", iLineCtr);							//line prompt
			sline = (string)calloc(sizeof(char), MAX_LINE);			//allocating space for next string and initializing to 0
			fgets(sline, MAX_LINE, stdin);							//reading stdin
			

			//Converting to struct Str data-type
			sLineStr = getStringPtr(sline);

			//Removing endline character from the end on inputed String
			(sLineStr->data)[sLineStr->len - 1] = '\0';
			(sLineStr->len)--;

			///////////////////////////////
			
			

			//If user types '!q', or '!quit' (case insensitive), quit the program with EXIT_SUCCESS
			if (strequals(sQuitStr, toUpper(sLineStr)) || ((sline[0] == '!' && sline[1] == 'q' || sline[1] == 'Q') && sline[2] == '\0'))
			{
				free(sLineStr);
				free(sline);
				break;
			}






			
			///////////////////////////////
			

			free(sLineStr);											//freeing line String
			free(sline);											//freeing line string
		}
	}
	else if(argc == 2)
	{
		//One command-line argument
		//Parse .in file
	}
	else
	{
		//More than one command-line arguments -> not valid
		printf("No defined actions for 2 or more command line arguments.\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
void htTest()
{
	string sT;
	String * sLine;
	const String * sQuit = getStringPtr("!Quit");
	const String * sStat = getStringPtr("!stats");

	HashTable * ht = getHT(2);


	while (true)
	{
		printf(">  ");
		sT = (string)calloc(sizeof(char), MAX_LINE);
		fgets(sT, MAX_LINE, stdin);



		//Converting to struct Str data-type
		sLine = getStringPtr(sT);

		//Removing endline character from the end on inputed String
		(sLine->data)[sLine->len - 1] = '\0';
		(sLine->len)--;

		//If user types '!q', or '!quit' (case insensitive), quit the program with EXIT_SUCCESS
		if (strequals(sQuit, toUpper(sLine)) || ((sT[0] == '!' && sT[1] == 'q' || sT[1] == 'Q') && sT[2] == '\0'))
		{
			free(sLine);
			free(sT);
			break;
		}

		if (sLine->len > 2)
		{
			if (strequals(sStat, sLine))
			{
				printf("Len: %d\n",  ht->len);
				printf("Size: %d\n", ht->size);
			}
			else
			{
				switch ((sLine->data)[0])
				{
					case 'i':
					{
						string sSubstring = (sLine->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						int iVal = rand();
						printf("Storing %d in hashtable!\n", iVal);
						HTinsert(ht, sKey, &iVal);
						break;
					}
					case 'g':
					{
						string sSubstring = (sLine->data) + 2;
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
					case 's':
					{
						string sSubstring = (sLine->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						printf("Searching '%s' in hashtable!\n", sSubstring);
						bool kvp = HTsearch(ht, sKey);
						if (kvp)
						{
							printf("'%s' was found!\n", sSubstring);
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


		free(sLine);
		free(sT);
	}
}
void reTest()
{
	string sT;
	String * sLine;
	const String * sQuit = getStringPtr("!Quit");
	const String * sStat = getStringPtr("!stats");

	HashTable * ht = getHT(2);

	string sRegex;
	string sTag;



	while (true)
	{
		printf(">  ");
		sT = (string)calloc(sizeof(char), MAX_LINE);
		fgets(sT, MAX_LINE, stdin);



		//Converting to struct Str data-type
		sLine = getStringPtr(sT);

		//Removing endline character from the end on inputed String
		(sLine->data)[sLine->len - 1] = '\0';
		(sLine->len)--;

		//If user types '!q', or '!quit' (case insensitive), quit the program with EXIT_SUCCESS
		if (strequals(sQuit, toUpper(sLine)) || ((sT[0] == '!' && sT[1] == 'q' || sT[1] == 'Q') && sT[2] == '\0'))
		{
			break;
		}

		//Input: nr "[tag]" /[regex]/
		//		create a new regex, tagged [tag]
		//		regex string is [regex]

		//Input: nr [regex]
		//		creates a new regex, tagged sequentially (so, "1", "2", "some manually created tag", "4", and so on)

		//Input: ts [string]
		//		tests test string between double quotes


		if (strequals(getStringPtr(""), sLine))
		{
			continue;
		}

		if (sLine->len > 2)
		{
			//Adding a regex
			if ((sLine->data)[0] == 'n' && (sLine->data)[1] == 'r' && (sLine->data)[2] == ' ')
			{
				//Regex * taggedRegex = getRegex(getStringPtr("nr \".+\" /.*/"));

				//if (!regexFullMatch(taggedRegex, sLine))
				{
					string srem = (sLine->data) + 3;
					sTag = (string) malloc(sizeof(char) * 5);
					sprintf_s(sTag, 5, "%d", ht->size + 1);
					sRegex = srem;
					printf("Adding:\n");
					printf("\tRegex: %s\n", sRegex);
					printf("\tTag: %s\n", sTag);

					String * sReg = getStringPtr(sRegex);
					String * sT = getStringPtr(sTag);

					Regex * reg = getRegex(sReg);

					if (reg == error)
					{
						printError(reg, -1, -1);
						printf("\n");
						continue;
					}
					HTinsert(ht, sT, reg);
				}

			}
			//Testing a regex
			else if ((sLine->data)[0] == 't' && (sLine->data)[1] == 's' && (sLine->data)[2] == ' ')
			{
				
				string srem = (sLine->data) + 3;
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
			else if ((sLine->data)[0] == 'g' && (sLine->data)[1] == ' ')
			{
				String * get = getStringPtr(sLine->data + 2);
				kvPair * ret = HTgetItem(ht, get);
				if (ret)
				{
					printf("Regex found\n\tData: \"%s\"!\n\n", (string)((String *)(ret->data))->data);
				}
				else
				{
					printf("Regex not found!\n");
				}
				continue;
			}
			else if ((sLine->data)[0] == 's' && (sLine->data)[1] == ' ')
			{
				String * get = getStringPtr(sLine->data + 2);
				bool ret = HTsearch(ht, get);
				if (ret)
				{
					printf("Regex found!\n\n");
				}
				else
				{
					printf("Regex not found!\n");
				}
				continue;
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
	}
}
void llTest()
{
	string sT;
	String * sLine;
	const String * sQuit = getStringPtr("!Quit");
	const String * sStat = getStringPtr("!stats");

	LinkedList * ll = getLL();

	while (true)
	{
		printf(">  ");
		sT = (string)calloc(sizeof(char), MAX_LINE);
		fgets(sT, MAX_LINE, stdin);



		//Converting to struct Str data-type
		sLine = getStringPtr(sT);

		//Removing endline character from the end on inputed String
		(sLine->data)[sLine->len - 1] = '\0';
		(sLine->len)--;

		//If user types '!q', or '!quit' (case insensitive), quit the program with EXIT_SUCCESS
		if (strequals(sQuit, toUpper(sLine)) || ((sT[0] == '!' && sT[1] == 'q' || sT[1] == 'Q') && sT[2] == '\0'))
		{
			free(sLine);
			free(sT);
			break;
		}

		if (sLine->len > 2)
		{
			if (strequals(sStat, sLine))
			{
				printf("Len: %d\n",  ll->iLen);
			}
			else
			{
				switch ((sLine->data)[0])
				{
					case 'i':
					{
						string sSubstring = (sLine->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						printf("Storing '%s' in linkedlist!\n", sKey->data);
						LLappend(ll, sKey);
						break;
					}
					case 'b':
					{
						string sSubstring = (sLine->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						printf("Storing '%s' in linkedlist!\n", sKey->data);
						LLprepend(ll, sKey);
						break;
					}
					case 'r':
					{
						string sSubstring = (sLine->data) + 2;
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
					case 'e':
					{
						string sSubstring = (sLine->data) + 2;
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
					case 'g':
					{
						string sSubstring = (sLine->data) + 2;
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
					case 'a':
					{
						string sSubstring = (sLine->data) + 2;
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


		//free(sLine);
		//free(sT);
	}
}
void alTest()
{
	string sT;
	String * sLine;
	const String * sQuit = getStringPtr("!Quit");
	const String * sStat = getStringPtr("!stats");

	ArrayList * al = getAL(4);

	while (true)
	{
		printf(">  ");
		sT = (string)calloc(sizeof(char), MAX_LINE);
		fgets(sT, MAX_LINE, stdin);



		//Converting to struct Str data-type
		sLine = getStringPtr(sT);

		//Removing endline character from the end on inputed String
		(sLine->data)[sLine->len - 1] = '\0';
		(sLine->len)--;

		//If user types '!q', or '!quit' (case insensitive), quit the program with EXIT_SUCCESS
		if (strequals(sQuit, toUpper(sLine)) || ((sT[0] == '!' && sT[1] == 'q' || sT[1] == 'Q') && sT[2] == '\0'))
		{
			free(sLine);
			free(sT);
			break;
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
		if (sLine->len > 2)
		{
			if (strequals(sStat, sLine))
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
				switch ((sLine->data)[0])
				{
					//a -> append
					case 'a':
					{
						string sSubstring = (sLine->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						printf("Appending '%s' to arraylist!\n", sKey->data);
						int i = atoi(sSubstring);
						ALappend(al, i);
						break;
					}
					//p -> prepend
					case 'p':
					{
						string sSubstring = (sLine->data) + 2;
						String * sKey = getStringPtr(sSubstring);
						printf("Prepending '%s' to arraylist!\n", sKey->data);
						int i = atoi(sSubstring);
						ALprepend(al, i);
						break;
					}
					//r -> remove item by val
					case 'r':
					{
						string sSubstring = (sLine->data) + 2;
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
					//e -> remove item by index
					case 'e':
					{
						string sSubstring = (sLine->data) + 2;
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
					case 'g':
					{
						string sSubstring = (sLine->data) + 2;
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
					case 's':
					{
						string sSubstring = (sLine->data) + 2;
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
					//s -> search item by value
					case 't':
					{
						ArrayList * set = ALsetify(al, (*stdEquals));

						printf("Set: \n");
						printAL(set, (*stdPrint));
						printf("\n");

						freeAL(set);
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


		//free(sLine);
		//free(sT);
	}
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

const string interpreter_stack = "C.Interpreter.interpreter.c";

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