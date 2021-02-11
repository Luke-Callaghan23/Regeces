#include "regex.h"
#include <stdlib.h>
#include <stdio.h>


const string regex_stack = "C.Interpreter.Regex.regex.c";



State * getState(bool, int, MLink *);
MLink * nextMachine     (Regex *, String *, int, State **, int, MLink *, MLink *, int, String *, bool);
MLink * sequenceMachine (Regex *, String *, int, State **, int, MLink *, MLink *);
MLink * starMachine     (String *, int, State **, int, MLink *, MLink *);
MLink * plusMachine     (String *, int, State **, int, MLink *, MLink *);
MLink * questionMachine (String *, int, State **, int, MLink *, MLink *);
MLink * curlyMachine    (Regex *, String *, int, State **, int, MLink *, MLink *);
MLink * setMachine      (Regex *, String *, int, State **, int, MLink *, MLink *);
LinkedList * getBaseLinks(MLink *);
LinkedList * getStates(MLink * machine);
bool addLink(State *, String *, State *);
String * strGetMatchGroupingElement(void *);
int iTagCompare(void *, void *);


void printMatch(void *);
bool bMatchEquals(void *, void *);
bool bTaggedMatch(void *, void *);

// Special shortcut characters
const int  iShortcutSize  = 6;
const char aShortcutChars[] = { 'd', 'D', 's', 'S', 'w', 'W' };
enum eShortcuts { d, D, s, S, w, W };

// Special border characters
const int  iBorderSize    = 9;
const char aBorderChars[] = { '*', '+', '?', '{', '(', '[', ')', '|', ';' };

Regex * getRegex(String * regex)
{
	const string call = "::getRegex";

	Regex * reg = (Regex *)malloc(sizeof(Regex));
	if (reg)
	{
		// Initializing default values
		reg->regex   = getStringPtr(regex->data);
		if (reg->regex == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		reg->iStates = 0;
		reg->matches = getHT(2);
		// printf("%d", error);
		if (reg->matches == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		reg->start   = getState(true, reg->iStates, NULL);
		if (reg->start == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		reg->startMach = NULL;

		// Wrapping reg->start into a 1-long array containing only reg->start
		State ** aStart = (State **)malloc(sizeof(State *));
		if (aStart)
		{
			aStart[0] = reg->start;

			// Calling next machine on our new regex, and
			MLink * mlMach = nextMachine(reg, regex, 0, aStart, 1, NULL, NULL, 0, NULL, false);
			if (mlMach == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		}
		else
		{
			return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
		}
	}
	else
	{
		return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
	}
	return reg;
}

State * getState(bool accept, int id, MLink * parent)
{
	const string call = "::getState";


	if (parent == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	State * ret = (State *)malloc(sizeof(State));
	if (ret)
	{
		ret->accept   = accept;
		ret->iStateId = id;
		ret->parent   = parent;
		ret->links    = getHT(2);
		if (ret->links == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	}
	else
	{
		return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
	}
	return ret;

}

int charInArray(char target, char* arr, int iLen)
{
	int ret = NOT_FOUND;
	for (int loop = 0; loop < iLen; loop++)
	{
		if (arr[loop] == target)
		{
			return loop;
		}
	}
	return ret;
}

LinkedList * getBaseLinks(MLink * machine)
{
	
	const string call = "::getBaseLinks";

	if (machine == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	if (!machine) return propagateError(getStringPtr("NullPointerError!  Trying to get base links of a NULL machine!"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);

	LinkedList * ret = getLL();
	if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	LinkedList * temp;
	// If machine is a scope machine,
	// 		we find the base links by traversing through a linear linked list
	// 		advancing by the pts's next and adding the base links as long as the next machine is
	// 		A: in scope, and B: the previous machine is passable
	if (machine->data->machineType == SCOPE || machine->data->machineType == OR_NODE)
	{
		MLink * ptr = machine->inner;
		while (ptr && ptr->outer)
		{
			temp = LLify(ptr->data->aBaseLinks, ptr->data->iBaseLinksLen);
			if (temp == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			LinkedList * ll = LLaddLL(ret, temp);
			if (ll == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			free(temp);
			if (!ptr->bIsPassable)
			{
				break;
			}
			ptr = ptr->next;
		}
	}
	else if (machine->data->machineType == OR)
	{
		MLink * mlOrNodePtr = machine->inner;
		while (mlOrNodePtr)
		{
			temp = LLify(mlOrNodePtr->inner->data->aBaseLinks, mlOrNodePtr->inner->data->iBaseLinksLen);
			if (temp == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			LinkedList * ll = LLaddLL(ret, temp);
			if (ll == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			free(temp);
			mlOrNodePtr = mlOrNodePtr->next;
		}
	}
	// If the current machine is passable, we traverse through a linear linked list
	// 		advancing by the ptr's next and adding base links as long as the the previous
	// 		machine was passable
	else if (machine->bIsPassable)
	{
		MLink * ptr = machine;
		while (ptr)
		{
			temp = LLify(ptr->data->aBaseLinks, ptr->data->iBaseLinksLen);
			if (temp == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			LinkedList * ll = LLaddLL(ret, temp);
			if (ll == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			free(temp);
			if (!ptr->bIsPassable)
			{
				break;
			}
			ptr = ptr->next;
		}
	}
	// Else, the base links simply the LinkedList form of the machine's base links
	else
	{
		free(ret);
		ret = LLify(machine->data->aBaseLinks, machine->data->iBaseLinksLen);
		if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	}
	LinkedList * set = LLsetify(ret, strequals);
	if (set == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	freeLL(ret);
	return set;
}

LinkedList * getStates(MLink * machine)
{

	const string call = "::getStates";

	if (machine == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	if (!machine || !machine->data) return propagateError(getStringPtr("NullPointerError!  Trying to get states of a NULL machine!"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
	if (machine->data == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


	if (machine->data->machineType == SCOPE || machine->data->machineType == OR_NODE)
	{
		LinkedList * ret = getLL();
		if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		MLink * ptr = machine->inner;
		while (ptr)
		{
			ret = LLaddLL(ret, ptr->data->llAllStates);
			if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			ptr = ptr->next;
		}
		return ret;
	}
	else if (machine->data->machineType == OR)
	{
		LinkedList * ret = getLL();
		if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		MLink * mlOrNodePtr = machine->inner;
		while (mlOrNodePtr)
		{
			LinkedList * ll = LLaddLL(ret, mlOrNodePtr->data->llAllStates);
			if (ll == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			mlOrNodePtr = mlOrNodePtr->next;
		}
		return ret;
	}
	else
	{
		return machine->data->llAllStates;
	}
}

bool getPassability(MLink * machine)
{

	const string call = "::getPassability";


	if (machine == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	if (!machine || !machine->data) return propagateError(getStringPtr("NullPointerError!  Trying to get passability of a NULL machine!"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
	if (machine->data == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	if (machine->data->machineType == SCOPE || machine->data->machineType == OR_NODE)
	{
		MLink * mlMachPtr = machine->inner;
		while (mlMachPtr)
		{
			if (!mlMachPtr->bIsPassable) return false;
			mlMachPtr = mlMachPtr->next;
		}
		return true;
	}
	else if (machine->data->machineType == OR)
	{
		MLink * orNodePtr = machine->inner;
		while (orNodePtr)
		{
			bool bPassable = getPassability(orNodePtr);
			if (bPassable == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			if (bPassable) return true;
			orNodePtr = orNodePtr->next;
		}
		return false;
	}
	else
	{
		return machine->bIsPassable;
	}
}

enum eDirections { INNER, OUTER, NEXT, PREV, NONE=-1, EXIT=-2 };

MLink * nextMachine(Regex * rRegex, String * strRegex, int iRegexStart, State ** aPrevAccept, int iPrevAcceptLen, MLink * mlNextMach, MLink * mlPrevMach, int iNextDir, String * strIncomingTag, bool bRepeatingTag)
{
	const string call = "::nextMachine";
	if (rRegex == error ||
		strRegex == error ||
		aPrevAccept == error ||
		iPrevAcceptLen == error ||
		mlNextMach == error ||
		mlPrevMach == error ||
		strIncomingTag == error)
			return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


	if (iRegexStart >= strRegex->len)
	{
		return mlPrevMach;
	}
	char cur = (strRegex->data)[iRegexStart];
	int iBorderIndex = charInArray(cur, aBorderChars, iBorderSize);

	MLink * next = NULL;
	MLink * mlCur;
	if (!mlNextMach)
	{
		mlCur = (MLink *)malloc(sizeof(MLink));
		if (mlCur)
		{
			mlCur->iStart = iRegexStart;

			mlCur->sSubstring = NULL;
			mlCur->bIsPassable = false;
			mlCur->data  = NULL;

			mlCur->outer = (mlPrevMach) ? mlPrevMach->outer : NULL;
			mlCur->inner = NULL;
			mlCur->prev  = mlPrevMach;
			mlCur->next  = NULL;
			mlCur->tag   = strIncomingTag;
			mlCur->bRepeatableTag = bRepeatingTag;
			if (mlPrevMach) mlPrevMach->next = mlCur;
		}
		else
		{
			return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
		}
	}
	else
	{
		mlCur = mlNextMach;
		mlNextMach->tag = strIncomingTag;
		mlNextMach->bRepeatableTag = bRepeatingTag;
	}

	if (iBorderIndex == NOT_FOUND || (iRegexStart == 0 && (iBorderIndex <= CURLY || iBorderIndex == OR)))
	{	
		if (cur == '\\' && charInArray(strRegex->data[iRegexStart + 1], aShortcutChars, iShortcutSize) != NOT_FOUND)
		{
			string setRegex = (string)malloc(sizeof(char) * 5);
			if (!setRegex) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);

			setRegex[0] = '[';
			setRegex[1] = '\\';
			setRegex[2] = strRegex->data[iRegexStart+1];
			setRegex[3] = ']';
			setRegex[4] = '\0';

			String * strSetReg = getStrKnownLen(setRegex, 5);
			if (strSetReg == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			next = setMachine(rRegex, strSetReg, 0, aPrevAccept, iPrevAcceptLen, mlCur, NULL);
			if (next == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			mlCur->iStart = iRegexStart;
			mlCur->iEnd   = mlCur->iStart + 2;
			mlCur->sSubstring  = substring(strRegex, mlCur->iStart, mlCur->iEnd);
			mlCur->bIsPassable = false;
			if (mlCur->sSubstring == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

		}
		else
		{
			// Next machine is a sequence machine
			next = sequenceMachine(rRegex, strRegex, iRegexStart, aPrevAccept, iPrevAcceptLen, mlCur, mlPrevMach);
			if (next == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		}

	}
	else if (iBorderIndex <= CURLY)
	{
		// Structure of a "repitiion machine"
		// 		  (prev macines) --(next)--> REP --(next)--> (rest of the regex machines)
		// 									  |
		// 									  |
		// 								   (inner)
		// 									  |
		// 									  |----> (previous machine)
		// 
		// 	-	When a repition machine is made, the previous machine is removed from the normal sequence of 
		// 			machines and inserted as the submachine of the repition machine
		// 	-	In the case of a curly machine '{}' which can have a certain number of repetitions
		// 			the previous machine is removed from the normal sequence of machines and it is
		// 			re-added some amount of times



		// next machine is a length modification machine
		// 		i.e. *, +, ?, {
		switch (aBorderChars[iBorderIndex])
		{
			case '*':
				next = starMachine(strRegex, iRegexStart, aPrevAccept, iPrevAcceptLen, mlCur, mlPrevMach);
				break;
		
			case '+':
				next = plusMachine(strRegex, iRegexStart, aPrevAccept, iPrevAcceptLen, mlCur, mlPrevMach);
				break;
		
			case '?':
				next = questionMachine(strRegex, iRegexStart, aPrevAccept, iPrevAcceptLen, mlCur, mlPrevMach);
				break;

			case '{':
				next = curlyMachine(rRegex, strRegex, iRegexStart, aPrevAccept, iPrevAcceptLen, mlCur, mlPrevMach);
				break;
		}
		if (next == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

		if (rRegex->startMach == mlPrevMach || !rRegex->startMach)
		{
			rRegex->startMach = next;
		}
		iNextDir = NOT_FOUND;
	}
	else if (iBorderIndex == OR)
	{
		// Structure of an OR machine
		// 		(prev) --(next)--> OR --(next)--> (rest of regex machines)
		// 					  		|
		// 					  	 (inner)
		// 					  		|
		// 					  		|----> OR_NODE 1 --(next)--> OR_NODE 2 --(next)--> ... --(next)--> OR_NODE N
		// 					  				  |                     |									  |
		// 					  			   (inner)				 (inner)							   (inner)
		// 					  				  |						|									  |
		// 					  				  |--> (machine (s))	|-->(machine (s))					  |-->(machine (s))
		// 
		// 
		// 	-	Like a SCOPE machine, the OR machine uses an outer, OR, machine which points to 
		// 			(essentially) a LinkedList of other machines
		// 			-	In the case of the OR machine, the "inner" MLink pointer points not to
		// 					an actual machine with data in it, but an OR_NODE which has an "inner"
		// 					MLink pointer to essentially the same thing as a SCOPE machine
		// 					-	That is, a sequence of machines, connected by a "next" MLink pointer
		// 	-	The behavoir of an OR machine is a sequence of scope machines (OR_NODEs) which are all
		// 			optional branches in the state machine
		// 	-	Because '|' has such high precedence, there are only two ways an OR machine can be expressed:
		// 				-	some machine|some other machine
		// 				-	(some machine|some other machine)
		// 			-	In the first case, we create an OR_NODE to surround the machines in 'some machine'
		// 			-	In the second case, however, we simply use the existing SCOPE machine the is the 
		// 					outer machine of '(some machine'

		// If the next character after the current '|' OR character, is another '|' or character, or a character for one of the repetition machines
		// 		('*', '+', '?', '}')
		// 		then, instead of crashine and saying that is a non-logical sequence, we force the next machine to be a sequence machine
		// 		and the '|', '+', '*', '?', '}' will be treated as literal characters
		bool bForceSequenceMachine = (strRegex->data[iRegexStart + 1] == OR || charInArray(strRegex->data[iRegexStart], aBorderChars, iBorderSize) <= CURLY);


		if (iRegexStart == strRegex->len - 1)
		{
			string s1 = (string) calloc(sizeof(char), 350);
			sprintf_s(s1, "Regex error!\n\tCannot end a regex with the OR machine character '|'.  Did you mean '\|' for the literal character '|'?\n\t\tRegex: %s\n\t\t", strRegex->data);
			string s2 = (string) calloc(sizeof(char), iRegexStart + 8);
			for (int iLoop = 0; iLoop < iRegexStart + 7; iLoop++) s2[iLoop] = ' ';
			string s3 = "^ Location of opening '|' machine character character.";
			String * s2s3 = strAddStringFreeBoth(getStringPtr(s2), getStringPtr(s3));
			free(s2);
			 
			String * err = strAddStringFreeBoth(getStringPtr(s1), s2s3);
			free(s1);
			return propagateError(err, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
		}
			

		if (mlPrevMach && ((mlPrevMach->outer && mlPrevMach->outer->data->machineType == SCOPE) || !mlPrevMach->outer))
		{
			int mod = 1;
			// If there is no outer maching for the previous OR NODE:
			if (!mlPrevMach->outer)
			{
				// If this is the case, we need to create a OR_NODE ourselves:
				mod = 0;
				MLink * mlHead = rRegex->startMach;
				mlPrevMach->outer = (MLink *)malloc(sizeof(MLink));
				if (!mlPrevMach->outer) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);

				mlPrevMach->outer->tag    = mlPrevMach->tag;
				mlPrevMach->bRepeatableTag = mlPrevMach->bRepeatableTag;
				mlPrevMach->outer->inner  = mlHead;
				mlPrevMach->outer->outer  = NULL;
				mlPrevMach->outer->next   = NULL;
				mlPrevMach->outer->prev   = NULL;
				mlPrevMach->outer->iStart = mlPrevMach->iStart;

				mlPrevMach->outer->data = (Machine *)malloc(sizeof(Machine));
				if (!mlPrevMach->data) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);

				MLink * mlPrevMachsPtr = mlHead;
				while (mlPrevMachsPtr)
				{
					mlPrevMachsPtr->outer = mlPrevMach->outer;
					mlPrevMachsPtr = mlPrevMachsPtr->next;
				}



			}


			MLink * mlOr = (MLink *)malloc(sizeof(MLink));
			if (!mlOr) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
			mlPrevMach->next = NULL;

			if (rRegex->startMach == mlPrevMach || rRegex->startMach == mlPrevMach->outer)
			{
				rRegex->startMach = mlOr;
			}


			// If the '|' OR divider is found INSIDE of a scope, we then
			// 		simply change the machine type of that SCOPE machine
			// 		to a OR_NODE Machine
			// (REMEMBER: OR machine is a sequence of optional OR_NODES, so the SCOPE machine is
			// 		essentially transformed into just one OR_NODE branch)
			MLink * scope = mlPrevMach->outer;
			scope->data->machineType = OR_NODE;

			// Rename scope to a more apropriate var:
			MLink * mlPreviousORNode = scope;

			// We then take this OR_NODE branch and we make it the inner machine of the mlOr
			// 		we created earlier
			mlOr->tag   = scope->tag;
			mlOr->bRepeatableTag = scope->bRepeatableTag;
			mlOr->inner = mlPreviousORNode;
			mlOr->outer = mlPreviousORNode->outer;
			if (mlPreviousORNode->outer && mlPreviousORNode->outer->inner == mlPreviousORNode) mlPreviousORNode->outer->inner = mlOr;
			mlPreviousORNode->outer = mlOr;

			// Then take the next/prev links from the (previously) SCOPE machine
			// 		and apply them to the outer OR machine
			mlOr->prev = mlPreviousORNode->prev;
			mlOr->next = mlPreviousORNode->next;
			if (mlPreviousORNode->next) mlPreviousORNode->next->prev = mlOr;
			if (mlPreviousORNode->prev) mlPreviousORNode->prev->next = mlOr;

			// Then, now that the (previously) SCOPE machine is finished, we need to
			// 		fill in the remaining data
			mlPreviousORNode->iEnd = iRegexStart;
			mlPreviousORNode->sSubstring = substring(strRegex, mlPreviousORNode->iStart + mod, mlPreviousORNode->iEnd);

			if (mlPreviousORNode->sSubstring == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			// OR_NODE data:
			Machine * mPreviousORNodeData = mlPreviousORNode->data;
			mPreviousORNodeData->aAccepting    = mlPrevMach->data->aAccepting;
			mPreviousORNodeData->iAcceptingLen = mlPrevMach->data->iAcceptingLen;
			mPreviousORNodeData->llAllStates   = getStates(mlPreviousORNode);

			if (mPreviousORNodeData->llAllStates == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			LinkedList * llBaseLinks = getBaseLinks(mlPreviousORNode);

			if (llBaseLinks == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			mPreviousORNodeData->iBaseLinksLen = llBaseLinks->iLen;
			mPreviousORNodeData->aBaseLinks    = LLgetAll(llBaseLinks);

			if (mPreviousORNodeData->aBaseLinks == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			mPreviousORNodeData->aBaseStates   = mlPreviousORNode->inner->data->aBaseStates;
			mPreviousORNodeData->iBaseLen      = mlPreviousORNode->inner->data->iBaseLen;
			mlPreviousORNode->bIsPassable = getPassability(mlPreviousORNode);
			
			if (mlPreviousORNode->bIsPassable == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


			mlPreviousORNode->tag = NULL;

			// Fill in some OR machine data
			mlOr->bIsPassable = false;
			mlOr->iStart = mlPreviousORNode->iStart;
			Machine * mORMachineData = (Machine *)malloc(sizeof(Machine));
			if (!mORMachineData) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);

			// Set data machine
			mlOr->data = mORMachineData;

			// mOrData->aBaseLinks    = LLgetAll(llBaseLinks);
			// mOrData->iBaseLinksLen = llBaseLinks->iLen;
			mORMachineData->aBaseStates = mlPreviousORNode->inner->data->aBaseStates;
			mORMachineData->iBaseLen    = mlPreviousORNode->inner->data->iBaseLen;
			mORMachineData->machineType = OR;
			mORMachineData->aAccepting  = NULL;
			mORMachineData->iAcceptingLen = 0;
			mORMachineData->aBaseLinks  = NULL;
			mORMachineData->iBaseLinksLen = 0;
			mORMachineData->llAllStates = NULL;


			mlCur->next = NULL;
			mlCur->prev = mlPreviousORNode;
			mlCur->outer = mlOr;
			mlCur->inner = NULL;
			mlCur->iStart = iRegexStart + 1;
			mlCur->iEnd   = -1;
			mlCur->sSubstring = NULL;
			mlCur->bIsPassable = false;
			mlCur->data = (Machine *)malloc(sizeof(Machine));
			if (!mlCur->data) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);

			mlCur->data->machineType = OR_NODE;

			mlPreviousORNode->next = mlCur;

			MLink * mlNextInnerNode = (MLink *)malloc(sizeof(MLink));
			if (!mlNextInnerNode) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);

			mlNextInnerNode->tag    = NULL;
			mlNextInnerNode->bRepeatableTag = false;
			mlNextInnerNode->outer  = mlCur;
			mlNextInnerNode->inner  = NULL;
			mlNextInnerNode->next   = NULL;
			mlNextInnerNode->prev   = NULL;
			mlNextInnerNode->iStart = iRegexStart + 1;
			mlNextInnerNode->iEnd   = -1;
			mlNextInnerNode->sSubstring = NULL;
			mlNextInnerNode->bIsPassable = false;
			mlCur->inner = mlNextInnerNode;


			// Structure:
			// 		mlOr		  --> or machine
			// 		mlOrData	  --> or machine's data machine
			// 		mlPrevMachine --> or node	(or machine's first or node... mlCur's prev)
			// 		mlCur		  --> or node	(or machine's next or node... mlPrevMachine's next)
			// 		mOND		  --> or node 2's data machine
			// 		mlNM		  --> mlCur's inner machine	(the machine that will be written-to during the next nextMachine call)
			// 


			next = bForceSequenceMachine ? 
				sequenceMachine(rRegex, strRegex, iRegexStart + 1, mlPreviousORNode->data->aBaseStates, mlPreviousORNode->data->iBaseLen, mlNextInnerNode, mlPrevMach, EXIT):
				nextMachine(rRegex, strRegex, iRegexStart + 1, mlPreviousORNode->data->aBaseStates, mlPreviousORNode->data->iBaseLen, mlNextInnerNode, mlPrevMach, EXIT, NULL, false);
			
			if (next == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


			// Finishing off current OR_NODE before finishing
			// 		OR machine 
			mlCur->tag  = mlNextInnerNode->tag;
			mlCur->bRepeatableTag = mlNextInnerNode->tag;
			mlCur->iStart = mlNextInnerNode->iStart;
			mlCur->iEnd = mlNextInnerNode->iEnd;
			mlCur->sSubstring = substring(strRegex, mlCur->iStart, mlCur->iEnd);

			if (mlCur->sSubstring == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			mlCur->data->aAccepting    = mlNextInnerNode->data->aAccepting;
			mlCur->data->iAcceptingLen = mlNextInnerNode->data->iAcceptingLen;
			mlCur->data->aBaseStates   = mlOr->data->aBaseStates;
			mlCur->data->iBaseLen      = mlOr->data->iBaseLen;
			mlCur->data->aBaseLinks    = mlNextInnerNode->data->aBaseLinks;
			mlCur->data->iBaseLinksLen = mlNextInnerNode->data->iBaseLinksLen;
			mlCur->data->llAllStates   = mlNextInnerNode->data->llAllStates;
			
			LinkedList * llAllAccept = getLL();
			LinkedList * llAllStates = getLL();
			LinkedList * llAllLinks  = getLL();

			if (llAllAccept == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			if (llAllStates == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			if (llAllLinks  == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);



			LinkedList * temp;

			LinkedList * errChecker;

			MLink * mlLastMach = NULL;
			MLink * mlONPtr = mlOr->inner;
			while (mlONPtr)
			{
				errChecker = LLaddLL(llAllAccept, temp = LLify(mlONPtr->data->aAccepting, mlONPtr->data->iAcceptingLen));
				if (errChecker == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				free(temp);
				errChecker = LLaddLL(llAllLinks, temp = LLify(mlONPtr->data->aBaseLinks, mlONPtr->data->iBaseLinksLen));
				if (errChecker == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				free(temp);
				errChecker = LLaddLL(llAllStates, temp = LLcopy(mlONPtr->data->llAllStates));
				if (errChecker == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				free(temp);

				mlLastMach = mlONPtr;
				mlONPtr = mlONPtr->next;
			}



			// Filling in remaining OR machine data
			mORMachineData->aAccepting = LLgetAll(llAllAccept);
			if (mORMachineData->aAccepting == error) propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			mORMachineData->iAcceptingLen = llAllAccept->iLen;
			Error * e = freeLL(llAllAccept);
			if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			mORMachineData->aBaseLinks = LLgetAll(llAllLinks);
			if (mORMachineData->aBaseLinks == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			mORMachineData->iBaseLinksLen = llAllLinks->iLen;
			e = freeLL(llAllLinks);
			if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			mORMachineData->llAllStates   = llAllStates;
			mlOr->iEnd             = next->iEnd + mod;
			mlOr->sSubstring       = substring(strRegex, mlOr->iStart, mlOr->iEnd);
			mlOr->bIsPassable      = getPassability(mlOr);
			e = freeLL(llBaseLinks);
			if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			if (mlOr->sSubstring == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			if (mlOr->bIsPassable == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			if (mlOr->bIsPassable)
			{
				for (int iBaseResetLoop = 0; iBaseResetLoop < mlCur->data->iBaseLen; iBaseResetLoop++)
				{
					mlCur->data->aBaseStates[iBaseResetLoop]->accept = true;
				}
			}

			next = mlOr;
			iNextDir = NOT_FOUND;
		}
		else if (mlPrevMach && mlPrevMach->outer && mlPrevMach->outer->data->machineType == OR_NODE)
		{
			// At this point, the prev machine is the previous machine to be built
			// 		and the outer is the previous ornode
			MLink * mlPrevON = mlPrevMach->outer;
			MLink * mlCurONInner = (MLink *)malloc(sizeof(MLink));
			if (!mlCurONInner) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);

			// Fill in previous OR NODE machine:
			mlCurONInner->tag     = NULL;
			mlCurONInner->bRepeatableTag = false;
			mlPrevON->iEnd        = iRegexStart;
			mlPrevON->sSubstring  = substring(strRegex, mlPrevON->iStart, mlPrevON->iEnd);
			mlPrevON->bIsPassable = getPassability(mlPrevON);
			if (mlPrevON->sSubstring  == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			if (mlPrevON->bIsPassable == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			
			Machine * prevOnData  = mlPrevON->data;
			prevOnData->aAccepting    = mlPrevMach->data->aAccepting;
			prevOnData->iAcceptingLen = mlPrevMach->data->iAcceptingLen;
			prevOnData->aBaseLinks    = mlPrevMach->data->aBaseLinks;
			prevOnData->iBaseLinksLen = mlPrevMach->data->iBaseLinksLen;
			prevOnData->aBaseStates   = mlPrevMach->data->aBaseStates;
			prevOnData->iBaseLen      = mlPrevMach->data->iBaseLen;
			prevOnData->llAllStates   = LLcopy(mlPrevMach->data->llAllStates);
			if (prevOnData->llAllStates == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			prevOnData->machineType   = OR_NODE;



			// mlCur --> current OR_NODE machine
			mlCur->prev  = mlPrevON;
			mlPrevON->next = mlCur;
			mlCur->next  = NULL;
			mlCur->outer = mlPrevON->outer;  // Making outer connection to OR machine
			mlCur->inner = mlCurONInner;
			mlCur->iEnd  = 0;
			mlCur->iStart++;
			mlCur->bIsPassable = false;
			mlCur->sSubstring  = NULL;
			mlCur->data  = (Machine *)malloc(sizeof(Machine));
			if (!mlCur->data) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
			
			mlCur->data->machineType = OR_NODE;

			// Head inner machine for the current
			// 		OR_NODE machine
			mlCurONInner->outer  = mlCur;
			mlCurONInner->inner  = NULL;
			mlCurONInner->prev   = NULL;
			mlCurONInner->next   = NULL;
			mlCurONInner->iStart = iRegexStart + 1;
			mlCurONInner->iEnd   = -1;
			mlCurONInner->data   = NULL;
			mlCurONInner->sSubstring = NULL;
			mlCurONInner->bIsPassable = false;

			next = bForceSequenceMachine ? 
				sequenceMachine(rRegex, strRegex, iRegexStart + 1, mlPrevON->outer->inner->data->aBaseStates, mlPrevON->outer->inner->data->iBaseLen, mlCurONInner, mlPrevMach, EXIT):
				nextMachine(rRegex, strRegex, iRegexStart + 1, mlPrevON->outer->inner->data->aBaseStates, mlPrevON->outer->inner->data->iBaseLen, mlCurONInner, mlPrevMach, EXIT, NULL, false);
			if (next == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


			Machine * mlCurData = mlCur->data;
			// Finishing off current OR_NODE before finishing
			// 		OR machine data

			mlCur->tag = mlCurONInner->tag;
			mlCur->bRepeatableTag = mlCurONInner->bRepeatableTag;
			mlCur->iStart = mlCurONInner->iStart;
			mlCur->iEnd   = mlCurONInner->iEnd;
			mlCur->sSubstring = substring(strRegex, mlCur->iStart, mlCur->iEnd);
			if (mlCur->sSubstring == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


			mlCur->bIsPassable       = getPassability(mlCur);
			if (mlCur->bIsPassable == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			mlCurData->aBaseStates   = mlPrevON->outer->data->aBaseStates;
			mlCurData->iBaseLen      = mlPrevON->outer->data->iBaseLen;
			mlCurData->aBaseLinks    = mlCurONInner->data->aBaseLinks;
			mlCurData->iBaseLinksLen = mlCurONInner->data->iBaseLinksLen;
			
			MLink * findLatestMachine = mlCur->inner;
			MLink * mLatest = NULL;
			while (findLatestMachine)
			{
				mLatest = findLatestMachine;
				findLatestMachine = findLatestMachine->next;
			}

			mlCurData->aAccepting    = mlCurONInner->data->aAccepting;
			mlCurData->iAcceptingLen = mlCurONInner->data->iAcceptingLen;
			mlCurData->llAllStates   = getStates(mlCurONInner);
			if (mlCurData->llAllStates == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			mlCur->iEnd              = mLatest->iEnd;

			return mlCur;
		}
	}
	else if (iBorderIndex == SCOPE)
	{
		// Structure of a SCOPE machine:
		// 		(prev machines) --(next)--> SCOPE --(next)--> (rest of machines in the regex)
		// 		   					  		  |
		// 		   					  	   (inner)
		// 		   					  		  |
		// 		   					  		  |----> (machine1) --(next)--> (machine2) --(next)--> ... --(next)--> (machine n)
		// 
		// 
		// 	-	A SCOPE machine is a way to easily encapsulate many machines into
		// 			one entity in a machine sequence... as we can see on the highest level,
		// 			previous machines point to a SCOPE which points to the rest of the machines
		// 			-	This way we can organize all machines in a sequence together into one entity
		// 					-	So, if the next machine happens to be a repetition machine,
		// 							we can easily repeate the SCOPE, without even having to glance 
		// 							at the machines secluded inside the SCOPE
		// 	-	The machine works by having the outer, SCOPE, machine have an inner MLink pointer
		// 			to the first machine in the sequence of machines encapsulated in the SCOPE
		// 			the first machine, as well as all of the "next" machines in the SCOPE sequence
		// 			all have "outer" MLink pointers to the SCOPE machine
		// 			-	In this way, the SCOPE machine essentially acts as a machine which has a ptr
		// 					to a LinkedList of inner machines

		// From here on:
		// 		mlCur --> the machine that will be used as the "scope machine"
		// 					this is the outer machine that acts as a grouping of everything in the 
		// 					parentheses
		// 		next  --> the inner machine inside of mlCur


		// First, we venture forward in the string to determine if the scope
		// 		has a closing ')' character
		// If not, we must raise an error
		bool bEndFound = false;
		int iParens  = 1;
		int iParenLoop = iRegexStart + 1;
		for (; iParenLoop < strRegex->len; iParenLoop++)
		{
			if (strRegex->data[iParenLoop] == '(')
			{
				iParens++;
			}
			else if (strRegex->data[iParenLoop] == ')')
			{
				iParens--;
				if (iParens == 0)
				{
					bEndFound = true;
					break;
				}
			}
		}


		// If no closing semicolon is found, or the closing 
		// 		semicolong was the last character, return an error
		if (!bEndFound)
		{
			string s1 = (string) calloc(sizeof(char), 350);
			sprintf_s(s1, "Regex error!\n\tOpening scope character '(' found, with no closing pair.  Did you mean '\\(' to for the literal sequence character '('?\n\t\tRegex: %s\n\t\t", strRegex->data);
			string s2 = (string) calloc(sizeof(char), iRegexStart + 8);
			for (int iLoop = 0; iLoop < iRegexStart + 7; iLoop++) s2[iLoop] = ' ';
			string s3 = "^ Location of opening '(' scope character.";
			String * s2s3 = strAddStringFreeBoth(getStringPtr(s2), getStringPtr(s3));
			free(s2);
			String * err = strAddStringFreeBoth(getStringPtr(s1), s2s3);
			free(s1);
			return propagateError(err, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
		}

		// Declaring machine links
		next = (MLink *)malloc(sizeof(MLink));
		if (!next) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);


		next->tag = NULL;
		next->bRepeatableTag = false;

		// Declaring machine
		mlCur->data = (Machine *)malloc(sizeof(Machine));
		if (!mlCur->data) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);


		mlCur->data->machineType = SCOPE;
		
		mlCur->iStart = iRegexStart;
		mlCur->iEnd   = 0;
		mlCur->data->aAccepting  = NULL;
		mlCur->data->aBaseStates = NULL;
		mlCur->data->aBaseLinks  = NULL;
		mlCur->data->iAcceptingLen = 0;
		mlCur->data->iBaseLen      = 0;
		mlCur->data->iBaseLinksLen = 0;
		mlCur->data->llAllStates   = NULL;


		mlCur->inner = next;

		mlCur->sSubstring  = NULL;
		mlCur->bIsPassable = false;
		
		// Resetting links of "current machine" so that it can can be used as the inner machine for the current scope
		// 		being created
		next->outer = mlCur;
		next->inner = NULL;
		next->prev  = NULL;
		next->next  = NULL;

		// let\s+;varname;(\w+)\s*(;?|=\s*;expression;(\w+))


		if (!rRegex->startMach) rRegex->startMach = mlCur;

		// next, the inner machine (think, all the states that make up the regex inside a set of parentheses),
		// 		gets created
		next = nextMachine(rRegex, strRegex, iRegexStart + 1, aPrevAccept, iPrevAcceptLen, next, mlPrevMach, EXIT, NULL, false);
		if (next == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		

		if(next && (next->outer && next->outer->data->machineType == OR) || next->data->machineType == OR)
			return next;

		if(next && next->data && next->data->machineType == OR) return true;

		mlCur->data->aAccepting    = next->data->aAccepting;
		mlCur->data->iAcceptingLen = next->data->iAcceptingLen;
		LinkedList * llBaseLinks   = getBaseLinks(mlCur);
		if (llBaseLinks == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		mlCur->data->aBaseLinks    = LLgetAll(llBaseLinks);
		if (mlCur->data->aBaseLinks == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		mlCur->data->iBaseLinksLen = llBaseLinks->iLen;
		mlCur->data->llAllStates   = getStates(mlCur);
		if (mlCur->data->llAllStates == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		mlCur->data->aBaseStates   = mlCur->inner->data->aBaseStates;
		mlCur->data->iBaseLen      = mlCur->inner->data->iBaseLen;

		next->iEnd = next->iEnd;
		mlCur->iEnd = next->iEnd + 1;
		mlCur->sSubstring = substring(strRegex, mlCur->iStart, mlCur->iEnd);
		if (mlCur->sSubstring == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		mlCur->bIsPassable = getPassability(mlCur);
		if (mlCur->bIsPassable == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		next = mlCur;
	}
	else if (iBorderIndex == SCOPE_CLOSE)
	{
		if (mlPrevMach && mlPrevMach->data->machineType == SCOPE)
		{
			// Break the current scope
			


			// Store scope machine
			MLink * scope = mlPrevMach;
			scope->iEnd = iRegexStart;
			scope->data->aAccepting    = mlPrevMach->data->aAccepting;
			scope->data->iAcceptingLen = mlPrevMach->data->iAcceptingLen;
			LinkedList * llBaseLinks   = getBaseLinks(scope);
			if (llBaseLinks == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			scope->data->aBaseLinks    = LLgetAll(llBaseLinks);
			if (scope->data->aBaseLinks == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			scope->data->iBaseLinksLen = llBaseLinks->iLen;
			Error * e = freeLL(llBaseLinks);
			if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			scope->data->llAllStates   = getStates(scope);
			if (scope->data->llAllStates == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			scope->data->aBaseStates   = scope->inner->data->aBaseStates;
			scope->data->iBaseLen      = scope->inner->data->iBaseLen;
			scope->bIsPassable         = getPassability(scope);
			scope->sSubstring = substring(strRegex, scope->iStart, scope->iEnd);
			if (scope->bIsPassable == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			if (scope->sSubstring  == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			
			MLink * ret = (mlPrevMach->iEnd <= strRegex->len) ? 
				nextMachine(rRegex, strRegex, iRegexStart + 1, scope->data->aAccepting, scope->data->iAcceptingLen, NULL, scope, NEXT, NULL, false) : NULL;
			if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			return ret;
		}
		else if (mlPrevMach && mlPrevMach->outer && mlPrevMach->outer->data->machineType == OR)
		{
			if (mlPrevMach == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			return mlPrevMach;
		}
		else if (mlPrevMach && mlPrevMach->outer && mlPrevMach->outer->data->machineType == OR_NODE)
		{
			if (mlPrevMach == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			return mlPrevMach;
		}
		else 
		{
			// Else, next machine is a sequence machine in we are not preaking out of a scope
			next = sequenceMachine(rRegex, strRegex, iRegexStart, aPrevAccept, iPrevAcceptLen, mlCur, mlPrevMach, NULL);
			if (next == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		}
	}
	else if (iBorderIndex == SET)
	{
		// Structure of SET machine:
		// 		(prev machines) --(next)--> (SET Machine -- a singular state) --(next)--> (rest of machines in regex) 
		// 
		// 	-	a set machine is a singular state with many links going into it
		// 		-	It appears just like any given state in a SEQ machine, but unlike states
		// 				in a SEQ machine which only (normally) have one incoming link
		// 				the SET machine's state (normally) has many links that lead to it

		// Next machine is a char set machine
		next = setMachine(rRegex, strRegex, iRegexStart, aPrevAccept, iPrevAcceptLen, mlCur, mlPrevMach);
		if (next == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	}
	else if (iBorderIndex == TAG)
	{
		// When a semicolon is encountered:

		string aTagBuffer = (string)calloc(sizeof(char), strRegex->len - iRegexStart);
		if (!aTagBuffer) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);


		// By default, we start adding tag characters into the buffer
		//		starting at the first character after the opening ';'
		//		tag character
		// And, the tag is not repeatable
		int offset = 1;
		bool bRepeating = false;

		// If the first character after the opening ';' tag character is '*',
		//		however, we start reading the tag String starting after the 
		//		'*' with repeating tag set to true
		if (iRegexStart + 1 < strRegex->len && strRegex->data[iRegexStart + offset] == '*')
		{
			offset = 2;
			bRepeating = true;
		}

		// Loop to add characters between the two ';' character to a string 
		//		buffer
		bool bEndFound = false;
		int iTagLen  = 0;
		int iTagLoop = iRegexStart + 1;
		for (iTagLoop = iRegexStart + offset; iTagLoop < strRegex->len; iTagLoop++)
		{
			if (strRegex->data[iTagLoop] == ';')
			{
				bEndFound = true;
				break;
			}
			aTagBuffer[iTagLen++] = strRegex->data[iTagLoop];
		}

		// If no closing semicolon is found, or the closing 
		// 		semicolong was the last character, return an error
		if (!bEndFound)
		{
			string s1 = (string) calloc(sizeof(char), 350);
			sprintf_s(s1, "Regex error!\n\tOpening tag character ';' found, with no closing pair.  Did you mean '\\;' to for the literal sequence character ';'?\n\t\tRegex: %s\n\t\t", strRegex->data);
			string s2 = (string) calloc(sizeof(char), iRegexStart + 8);
			for (int iLoop = 0; iLoop < iRegexStart + 7; iLoop++) s2[iLoop] = ' ';
			string s3 = "^ Location of opening ';' tag character.";
			String * s2s3 = strAddStringFreeBoth(getStringPtr(s2), getStringPtr(s3));
			free(s2);
			 
			String * err = strAddStringFreeBoth(getStringPtr(s1), s2s3);
			free(s1);
			return propagateError(err, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
		}

		// Copy data into new, smaller buffer
		string aRealBuffer = (string) calloc(sizeof(char), (iTagLen + 1 - bRepeating));
		if (!aTagBuffer) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);

		// Copying characters from large character buffer into
		//		the correctly sized buffer
		int iCopyLoop;
		for (iCopyLoop = 0; iCopyLoop < iTagLen; iCopyLoop++)
		{
			aRealBuffer[iCopyLoop] = aTagBuffer[iCopyLoop];
		}
		aRealBuffer[iCopyLoop] = '\0';

		// Free old buffer
		free(aTagBuffer);

		mlCur->iStart = iTagLoop + 1;

		// Then create the next machine with the tag
		MLink * ret = nextMachine(rRegex, strRegex, iTagLoop + 1, aPrevAccept, iPrevAcceptLen, mlCur, mlPrevMach, iNextDir, getStrKnownLen(aRealBuffer, iTagLen), bRepeating);
		if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		return ret;
	}

	// If error, propagate
	if (!next || next == error)
	{
		// Error handling goes here
		return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	}

	// A lot of conditional stuff for making a SCOPE machine work correctly,
	//		the gist of it is that if "next direction" that we are building a machine is
	//		EXIT, and the "next" machine constructed in this function is an inner machine
	//		of a SCOPE machine, then we exist here instead of proceeding to the
	//		last line in this function which calls "nextMachine" again recursively
	if (iNextDir == EXIT && (next->iEnd < strRegex->len && strRegex->data[next->iEnd] == ')') 
		|| (next->iEnd < strRegex->len && strRegex->data[next->iEnd] == ')' 
				&& next->outer 
				&& next->outer->data 
				&& next->outer->data->machineType == SCOPE))
	{
		return next;
	}


	// Setting the startMach property of the regex struct only if this is the first machine to be 
	//		constructed in the regex
	if (!rRegex->startMach) rRegex->startMach = next;


	MLink * ret = (next->iEnd < strRegex->len) ? 
		nextMachine(rRegex, strRegex, next->iEnd, next->data->aAccepting, next->data->iAcceptingLen, NULL, next, NEXT, NULL, false) : 
		next;
	if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	return ret;
}

MLink * sequenceMachine(Regex * rRegex, String * strRegex, int iRegexStart, State ** aPrevAccept, int iPrevAcceptLen, MLink * mlCurrentMach, MLink * mlPrevMach)
{
	const string call = "::sequenceMachine";
	const String * universal = getStringPtr("un");
	if (universal == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	// Setting base facts about any sequence machine
	mlCurrentMach->iStart      = iRegexStart;
	mlCurrentMach->sSubstring  = NULL;
	mlCurrentMach->bIsPassable = false;

	// Initializing Machine data for this sequence machine
	Machine * seq = (Machine *)malloc(sizeof(Machine));
	if (seq)
	{
		seq->iBaseLen      = iPrevAcceptLen;
		seq->aAccepting    = NULL;
		seq->aBaseLinks    = NULL;
		seq->machineType   = SEQ;
		seq->aBaseStates   = aPrevAccept;
		seq->llAllStates   = getLL();
		seq->iBaseLinksLen = 0;
		seq->iAcceptingLen = 0;

		if (seq->llAllStates == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	}
	else
	{
		return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
	}
	mlCurrentMach->data = seq;

	
	int iBorderIndex;


	// Getting the remaining regex and its remaining length
	int iremlen = strRegex->len  - iRegexStart;
	string srem = strRegex->data + iRegexStart;


	// Loop variable that, on each iteration, will hold the 'prev' states
	// 		in the sequence
	// Initialized to the accepting states of the previous machine
	State ** aPrevStates = aPrevAccept;
	int iPrevLen = iPrevAcceptLen;
	

	// Loop variables that will hold the previous previous states
	// 		and previous previous length of those states
	// Initialized to NULL
	// Needed for cases where we need to retain the previous character and turn it into a machine
	// 		in cases such as: 'a*', 'a?', 'a+', 'a{n, m}'
	State * aPrevPrevStates = NULL;
	int iPrevPrevLen = 0;

	// Also needed is the specific link that leads from the prevprev states
	// 		and the prev states
	String * sPrevLink = NULL;

	// Flag for ignoring the effects of the next character
	bool bIgnoreNextChar = false;

	// outer, outer loop variable
	int outerouter;
	for (outerouter = 0; outerouter < iremlen; outerouter++)
	{
		// Localizing current character in sequence
		char cur = srem[outerouter];

		// Checking if the current character is a border character
		iBorderIndex = charInArray(cur, aBorderChars, iBorderSize);

		// If the border character was not found, or we're told to ignore the effects of the next character: (MCB)
		if (iBorderIndex == NOT_FOUND || bIgnoreNextChar)
		{
			// If the current character is '\' and we're not begin told to ignore the next character,
			if (cur == '\\' && !bIgnoreNextChar)
			{
				if (outerouter + 1 < iremlen)
				{
					// Check if the next character in the sequence is one of the shortcut characters
					int iShortCut = charInArray(srem[outerouter + 1], aShortcutChars, iShortcutSize);
					if (iShortCut == NOT_FOUND)
					{
						// If not, 
						// Set the ignor flag, and continue, so it doesn't get reset at the end of the loop
						bIgnoreNextChar = true;
						continue;
					}
					else
					{
						// Add a char set machine here
						// 		or something
						mlCurrentMach->iEnd = iRegexStart + outerouter;
						mlCurrentMach->sSubstring = substring(strRegex, iRegexStart, mlCurrentMach->iEnd);
						mlCurrentMach->data->aAccepting = aPrevStates;
						mlCurrentMach->data->iAcceptingLen = iPrevLen;
							
						if (mlCurrentMach->sSubstring == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


						if (!rRegex->startMach) rRegex->startMach = mlCurrentMach;

						MLink * nm = nextMachine(rRegex, strRegex, iRegexStart + outerouter, aPrevStates, iPrevLen, NULL, mlCurrentMach, NEXT, NULL, false);

						if (nm == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

						return nm;
					}
				}
			}
			else
			{
				// Add n to one
			NTOONEADD:
				if(1);

				// Variable that holds the transition character (String *) from the previous state(s) to this current one
				String * sLink = NULL;
				if (cur != '.' || bIgnoreNextChar)
				{
					// Translate current character into a String *
					// 		by making an array and promptly flattening it
					char l0 = cur;
					char l1 = '\0';

					string slink = (string)malloc(sizeof(char) * 2);
					if (slink)
					{
						slink[0] = l0;
						slink[1] = l1;

						sLink = getStringPtr(slink);

						if (sLink == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
					}
					else
					{
						return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
					}
				}
				else
				{
					// When the current character is a period '.', add the universal link
					sLink = universal;
				}

				// Initializing next state in the sequence
				State * sNextState = getState(true, ++rRegex->iStates, mlCurrentMach);
				if (sNextState == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


				// Add state to machine's list of states
				Error * e = LLappend(seq->llAllStates, sNextState);
				if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


				// Loop vars for tidying up the code a bit
				State * sPrevState = NULL;

				// Loop applying link to all items in the previous states array
				for (int inner = 0; inner < iPrevLen; inner++)
				{

					// Set loop vars
					sPrevState = aPrevStates[inner];

					// Set previous' accepting to false
					sPrevState->accept = false;

					// Adding a link, using the current character as the key,
					// 		from the current previous state, to the next state
					bool bAdded = addLink(sPrevState, sLink, sNextState);
					if (bAdded == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				}

				// Reseting previous previous states
				aPrevPrevStates = aPrevStates;
				iPrevPrevLen    = iPrevLen;

				// Freeing old previous link
				if(sPrevLink && sPrevLink->len == 1 && sPrevLink != seq->aBaseLinks[0])
				{
					Error * e = freeStr(sPrevLink);
					if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				}
				// Setting the previous link
				sPrevLink = sLink;


				// Reseting previous states
				aPrevStates = (State **)malloc(sizeof(State *));
				if (aPrevStates)
				{
					aPrevStates[0] = sNextState;
					iPrevLen = 1;
				}
				else
				{
					return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
				}
					 
				// Set the base links of this sequence machine 
				// 		only on the first iteration of this loop
				if (outerouter == 0 || (outerouter == 1 && bIgnoreNextChar))
				{
					seq->aBaseLinks = (String **)malloc(sizeof(String *));
					if (seq->aBaseLinks)
					{
						seq->aBaseLinks[0] = sLink;
						seq->iBaseLinksLen = 1;
					}
					else
					{
						return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
					}
				}
			}
		}
		// If the border character is scope or set, then simply create the next machine
		else if (iBorderIndex == SCOPE || iBorderIndex == SET || iBorderIndex == OR || iBorderIndex == TAG)
		{
			// Do some standard tidying up to make all the variables of this machine
			// 		correct
			mlCurrentMach->iEnd = outerouter + iRegexStart;
			mlCurrentMach->sSubstring = substring(strRegex, mlCurrentMach->iStart, mlCurrentMach->iEnd);

			if (mlCurrentMach->sSubstring == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			seq->aAccepting = aPrevStates;
			seq->iAcceptingLen = iPrevLen;

			if (!rRegex->startMach) rRegex->startMach = mlCurrentMach;

			// Then return the next machine
			MLink * ret = nextMachine(rRegex, strRegex, iRegexStart + outerouter, aPrevStates, iPrevLen, NULL, mlCurrentMach, NEXT, NULL, false);
			if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			return ret;
		}
		// If the border character is a closing scope:
		else if(iBorderIndex == SCOPE_CLOSE)
		{
			// Traverse upwards 
			MLink * outerPtr = mlCurrentMach->outer;
			while (outerPtr)
			{
				if (outerPtr->data->machineType == SCOPE || outerPtr->data->machineType == OR)
				{
					break;
				}
				outerPtr = outerPtr->outer;
			}

			// If this machine is inside of a scope, then we 
			// 		close that scope by returning (MCB)
			if (outerPtr && (outerPtr->data->machineType == SCOPE || outerPtr->data->machineType))
			{
				// Finish this machine
				mlCurrentMach->iEnd = outerouter + iRegexStart;
				mlCurrentMach->sSubstring = substring(strRegex, mlCurrentMach->iStart, mlCurrentMach->iEnd);

				if (mlCurrentMach->sSubstring == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

				seq->aAccepting = aPrevStates;
				seq->iAcceptingLen = iPrevLen;
				return mlCurrentMach;
			}
			// Else, if we are not inside of any scopes, we simply add the ')' character
			else
			{
				goto NTOONEADD;
			}
		}
		else if (iBorderIndex <= CURLY)
		{
			if (outerouter == 0)
			{
				goto NTOONEADD;
			}
			// If a previous machine exists, then call nextMachine using that previous machine
			if (mlPrevMach)
			{
				if (!rRegex->startMach) rRegex->startMach = mlCurrentMach;
				// call nextMachine using the prev machine 
				// 		because if the previous machine exists then outerouter loop
				// 		must be on it's first iteration
				MLink * ret = nextMachine(rRegex, strRegex, iRegexStart + outerouter, aPrevStates, iPrevLen, NULL, mlPrevMach, NONE, NULL, false);
				if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				return ret;
			}
			// If there is no previous machine, transform the previous character in the sequence
			else
			{
				if (outerouter == 1 || (outerouter == 2 && srem[0] == '\\'))
				{
					mlCurrentMach->iEnd = outerouter + iRegexStart;
					mlCurrentMach->sSubstring = substring(strRegex, mlCurrentMach->iStart, mlCurrentMach->iEnd);

					if (mlCurrentMach->sSubstring == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

					seq->aAccepting = aPrevStates;
					seq->iAcceptingLen = iPrevLen;
					MLink * ret = nextMachine(rRegex, strRegex, iRegexStart + outerouter, aPrevStates, iPrevLen, NULL, mlCurrentMach, NONE, NULL, false);
					if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
					return ret;
				}
				else
				{

					// Turning the previous state in this sequence into a machine of its own
					MLink * innerMach = (MLink *)malloc(sizeof(MLink));
					if (innerMach)
					{
						innerMach->data = (Machine *)malloc(sizeof(Machine));
						if (innerMach->data)
						{
							innerMach->iEnd   = outerouter + iRegexStart;
							innerMach->iStart = innerMach->iEnd - 1;
							innerMach->sSubstring = substring(strRegex, innerMach->iStart, innerMach->iEnd);

							if (innerMach->sSubstring == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

							innerMach->bIsPassable = false;

							innerMach->outer = mlCurrentMach->outer;
							innerMach->inner = NULL;
							innerMach->next  = NULL;
							innerMach->prev  = mlCurrentMach;

							innerMach->tag = NULL;
							innerMach->bRepeatableTag = false;

							mlCurrentMach->next = innerMach;
							mlCurrentMach->iEnd = iRegexStart + outerouter;

							innerMach->data->machineType = SEQ;
								

							aPrevStates[0]->parent = innerMach;


							innerMach->data->llAllStates = getLL();
							if (innerMach->data->llAllStates == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

							Error * e = LLappend(innerMach->data->llAllStates, aPrevStates[0]);
							if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

							innerMach->data->aAccepting = aPrevStates;
							innerMach->data->iAcceptingLen = iPrevLen;

							innerMach->data->aBaseStates = aPrevPrevStates;
							innerMach->data->iBaseLen    = iPrevPrevLen;
								
							innerMach->data->aBaseLinks = (String *)malloc(sizeof(String));
							if (innerMach->data->aBaseLinks)
							{
								innerMach->data->aBaseLinks[0] = sPrevLink;
								innerMach->data->iBaseLinksLen = 1;
							}
							else
							{
								return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
							}
								
						}
						else
						{
							return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
						}
					}
					else
					{
						return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
					}
					if (!rRegex->startMach) rRegex->startMach = innerMach;

					MLink * ret = nextMachine(rRegex, strRegex, iRegexStart + outerouter, aPrevStates, iPrevLen, NULL, innerMach, NONE, NULL, false);
					if (ret == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
					return ret;
				}

			}
		}
		mlPrevMach = NULL;
		bIgnoreNextChar = false;
	}

	mlCurrentMach->iEnd = outerouter + iRegexStart;
	mlCurrentMach->sSubstring = substring(strRegex, mlCurrentMach->iStart, mlCurrentMach->iEnd);

	if (mlCurrentMach->sSubstring == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	seq->aAccepting = aPrevStates;
	seq->iAcceptingLen = iPrevLen;


	if (mlCurrentMach == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	return mlCurrentMach;
}
MLink * setMachine(Regex * rRegex, String * strRegex, int iRegexStart, State ** aPrevAccept, int iPrevAcceptLen, MLink * mlCurrentMach, MLink * mlPrevMach)
{
	const string call = "::setMachine";
	const String * universal = getStringPtr("un");
	if (universal == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


	// Setting base facts about any sequence machine
	mlCurrentMach->iStart      = iRegexStart;
	mlCurrentMach->sSubstring  = NULL;
	mlCurrentMach->bIsPassable = false;

	// Initializing Machine data for this sequence machine
	Machine * set = (Machine *)malloc(sizeof(Machine));
	if (!set) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
	set->aAccepting    = NULL;
	set->aBaseLinks    = NULL;
	set->machineType   = SET;
	set->aBaseStates   = aPrevAccept;
	set->iBaseLen      = iPrevAcceptLen;
	set->llAllStates   = getLL();
	set->iBaseLinksLen = 0;
	set->iAcceptingLen = 0;

	if (set->llAllStates == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	mlCurrentMach->data = set;

	// If the first character of the 
	// 		char set machine is '^' then we know to invert all
	// 		links in the char set str
	bool bInvert = false;
	if (strRegex->data[iRegexStart + 1] == '^')
	{
		bInvert = true;
	}

	// Creating next state and adding it to the 
	// 		all states LL for this machine
	State * sNextState = getState(true, ++rRegex->iStates, mlCurrentMach);
	if (sNextState == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	
	// A set machine only has one new state, so set allStates to a 1-wide LL containing only
	// 		next state
	Error * e = LLappend(set->llAllStates, sNextState);
	if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	// And set aAccepting to a 1-wide array containing only 
	// 		next state, and set iAcceptingLen to 1
	set->aAccepting = (State **)malloc(sizeof(State *));
	if (!set->aAccepting) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
	set->aAccepting[0] = sNextState;
	set->iAcceptingLen = 1;

	// Creat linked list that will store all characters for 
	// 		used to link prev machine to next machine
	LinkedList * llAllLinks = getLL();
	if (llAllLinks == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


	// Flag for detecting if the last character
	// 		was backslash '\' which sometimes expresses that
	// 		we should ignore the effects of the next character
	// 		And sometimes expresses that we need to add a shortcut 
	// 		set: \d, \D, \w, \W, \s, \S
	bool bLastSlash = false;

	// Flag for detecting if the closing set character ']' was found
	// 		before the end of the regex
	bool bClosingFound = false;

	// Used for character ranges:
	char cBegChar = '\0', cEndChar = '\0';

	// This int will be used later to calculate mlCurrentMach->iEnd
	// 		so we initialize outside of the loop
	int iCharRetrievalLoop;
	for(iCharRetrievalLoop = iRegexStart + bInvert + 1; iCharRetrievalLoop < strRegex->len; iCharRetrievalLoop++)
	{
		if (strRegex->data[iCharRetrievalLoop] == '\\')
		{
			// If the last character was also a backslash,
			// 		then we add the bacslash character to the 
			// 		allLinks LL
			if (bLastSlash)
			{
				e = LLappend(llAllLinks, getStringFromChar('\\'));
				if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


				bLastSlash = false;
				continue;
			}

			// Else, we set bLasSlash to true and continue the loop
			bLastSlash = true;
			continue;
		}
		else if (bLastSlash && charInArray(strRegex->data[iCharRetrievalLoop], aShortcutChars, iShortcutSize) != NOT_FOUND)
		{
			// If character is lowercase:
			// 		shortcut is NOT inverted
			if (strRegex->data[iCharRetrievalLoop] >= 100 && strRegex->data[iCharRetrievalLoop] <= 119)
			{
				switch (strRegex->data[iCharRetrievalLoop])
				{
					case 'd':
					{
						// Beginning --> 0
						// Ending    --> 9
						cBegChar = '0';
						cEndChar = '9';
						// Adding all chars between
						for (char cLoop = cBegChar; cLoop <= cEndChar; cLoop++)
						{
							String * lnk = getStringFromChar(cLoop);
							if (lnk == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

							e = LLappend(llAllLinks, lnk);
							if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

						}
						break;
					}
					case 's':
					{
						int  iWhiteSpaceLen = 6;
						char aWhiteSpace[]  = { ' ', '\t', '\n', '\v', '\r', '\f' };
						char cCurrentAdd;
						for (int iWhiteSpace = 0; iWhiteSpace < iWhiteSpaceLen; iWhiteSpace++)
						{
							cCurrentAdd = aWhiteSpace[iWhiteSpace];
							String * lnk = getStringFromChar(cCurrentAdd);
							if (lnk == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

							e = LLappend(llAllLinks, lnk);
							if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
						}
						break;
					}
					case 'w':
					{
						// Setting up three ranges to add characters
						int  iArrLen = 3;
						char aBegs[] = { '0', 'a', 'A' };
						char aEnds[] = { '9', 'z', 'Z' };

						// For 0, 1, 2:
						for (int outer = 0; outer < iArrLen; outer++)
						{
							// Set beginning and ending ranges to add to accepted links
							cBegChar = aBegs[outer];
							cEndChar = aEnds[outer];

							// Do range-adding
							for (char cLoop = cBegChar; cLoop <= cEndChar; cLoop++)
							{
								String * lnk = getStringFromChar(cLoop);
								if (lnk == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

								e = LLappend(llAllLinks, lnk);
								if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
							}
						}
						break;
					}
				}
			}
			// Else, invert the shorcut:
			else
			{
				switch (strRegex->data[iCharRetrievalLoop])
				{
					case 'D':
					{
						// Adding \D shortcut
						int iInitialStart = 1;
						int iInitialEnd   = '0';

						int iSecondStart  = '9' + 1;
						int iSecondEnd    = 256;

						char cLoop;

						for (int iInitialAdd = iInitialStart; iInitialAdd < iInitialEnd; iInitialAdd++)
						{
							cLoop = iInitialAdd;

							String * lnk = getStringFromChar(cLoop);
							if (lnk == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

							e = LLappend(llAllLinks, lnk);
							if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
						}

						for (int iSecondAdd = iSecondStart; iSecondAdd < iSecondEnd; iSecondAdd++)
						{
							cLoop = iSecondAdd;
							String * lnk = getStringFromChar(cLoop);
							if (lnk == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

							e = LLappend(llAllLinks, lnk);
							if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
						}

						break;
					}
					case 'S':
					{
						// char aWhiteSpace[]  = { ' ', '\n', '\t', '\n', '\v', '\r', '\f' };

						char cLoop;
						for (int iAddNonWhiteSpace = 1; iAddNonWhiteSpace < 256; iAddNonWhiteSpace++)
						{
							cLoop = iAddNonWhiteSpace;
							if (cLoop != ' ' && cLoop != '\n' && cLoop != '\t' && cLoop != '\v' && cLoop != '\r' && cLoop != '\f')
							{
								String * lnk = getStringFromChar(cLoop);
								if (lnk == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

								e = LLappend(llAllLinks, lnk);
								if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
							}
						}

						break;
					}
					case 'W':
					{
						int  iSegsLen   = 4;
						int aBegSegs[] = { 1,   '9' + 1, 'Z' + 1, 'z' + 1 };
						int aEndSegs[] = { '0' - 1, 'A' - 1, 'a' - 1, 255 };


						char cLoop;

						for (int iOuterSegs = 0; iOuterSegs < iSegsLen; iOuterSegs++)
						{
							int cBeg = aBegSegs[iOuterSegs];
							int cEnd = aEndSegs[iOuterSegs];


							for (int iInitialAdd = cBeg; iInitialAdd <= cEnd; iInitialAdd++)
							{
								cLoop = iInitialAdd;
								
								String * lnk = getStringFromChar(cLoop);
								if (lnk == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

								e = LLappend(llAllLinks, lnk);
								if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
							}
						}


						break;
					}
				}
			}
		}
		else if (!bLastSlash && strRegex->data[iCharRetrievalLoop] == '-')
		{
			// If '-' is the first character in the sequence, then 
			// 		simply add '-' normally
			// If the current character is the last character in the sequence
			// 		simply add it normally, and the first statement after the loop
			// 		will handle the error
			if (iCharRetrievalLoop == iRegexStart + bInvert || iCharRetrievalLoop == strRegex->len - 1)
			{
				goto STD_ADDING;
			}
			
			// Setting range vars
			cBegChar = strRegex->data[iCharRetrievalLoop - 1];
			cEndChar = strRegex->data[iCharRetrievalLoop + 1];

			// If the character range is out of range,
			// 		add the '-' character normally
			// 		no use throwing an error
			// 		for something so small
			if (cBegChar >= cEndChar)
			{
				goto STD_ADDING;
			}


			// Adding each character in the range
			for (char cLoop = cBegChar; cLoop <= cEndChar; cLoop++)
			{
				String * lnk = getStringFromChar(cLoop);
				if (lnk == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

				e = LLappend(llAllLinks, lnk);
				if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			}


		}
		else if (!bLastSlash && strRegex->data[iCharRetrievalLoop] == '.')
		{
			e = LLappend(llAllLinks, universal);
			if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		}
		else if (!bLastSlash && strRegex->data[iCharRetrievalLoop] == ']')
		{
			// If ']' is the first character in the char set machine, then
			// 		simply add the character normally (empty char-set machines
			// 		not allowed)
			if (iCharRetrievalLoop == iRegexStart + bInvert)
			{
				goto STD_ADDING;
			}
			bClosingFound = true;
			break;
		}
		else
		{
		STD_ADDING:
			if (1);

			String * lnk = getStringFromChar(strRegex->data[iCharRetrievalLoop]);
			if (lnk == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			
			e = LLappend(llAllLinks, lnk);
			if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		}


		bLastSlash = false;

	}



	if (!bClosingFound)
	{
		string s1 = (string) calloc(sizeof(char), 350);
		sprintf_s(s1, "Regex error!\n\tOpening char-set character '[' found, with no closing pair.  Did you mean '\\[' to for the literal sequence character '['?\n\t\tRegex: %s\n\t\t", strRegex->data);
		string s2 = (string) calloc(sizeof(char), iRegexStart + 8);
		for (int iLoop = 0; iLoop < iRegexStart + 7; iLoop++) s2[iLoop] = ' ';
		string s3 = "^ Location of opening '[' char-set character.";
		String * s2s3 = strAddStringFreeBoth(getStringPtr(s2), getStringPtr(s3));
		free(s2);
		String * err = strAddStringFreeBoth(getStringPtr(s1), s2s3);
		free(s1);
		return propagateError(err, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
	}

	// Setify llAllLinks
	LinkedList * llSet = LLsetify(llAllLinks, (*strequals));
	if (llSet == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


	free(llAllLinks);
	llAllLinks = llSet;

	// Adding links
	if (!bInvert)
	{
		// If the links shouldn't be inverted:
		// 		first check if the size of the links is 1
		if (llAllLinks->iLen == 1)
		{
			// If only one link exists, then for each previously accepting state,
			// 		add the singular link going from there to the sNextState
			for (int iAddLinksLoop = 0; iAddLinksLoop < iPrevAcceptLen; iAddLinksLoop++)
			{
				aPrevAccept[iAddLinksLoop]->accept = false;
				e = addLink(aPrevAccept[iAddLinksLoop], (String *)(llAllLinks->front->data), sNextState);
				if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			}
		}
		else
		{
			// If more than one link exists
			// 		then for every link and every previously accepting state,
			// 		add a link between the previously accepting state and the sNextState
			LLNode * left  = llAllLinks->front;
			LLNode * right = llAllLinks->back;
			for (int iAllLinksLoop = 0; iAllLinksLoop <= llAllLinks->iLen / 2 && left && right; iAllLinksLoop++)
			{
				for (int iAddLinksLoop = 0; iAddLinksLoop < iPrevAcceptLen; iAddLinksLoop++)
				{
					aPrevAccept[iAddLinksLoop]->accept = false;
					// Adding links in both the left and right pointers
					e = addLink(aPrevAccept[iAddLinksLoop], (String *)(left->data),  sNextState);
					if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

					e = addLink(aPrevAccept[iAddLinksLoop], (String *)(right->data), sNextState);
					if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				}


				// Bad things happen if we left left and right pass each other
				// 		break loop
				if (right->prev == left)
				{
					break;
				}

				// Advance iteration vars
				left  = left->next;
				right = right->prev;
			}
		}
	}
	else
	{
		// Copy links
		LinkedList * llAllCopiedLinks = LLcopy(llAllLinks);
		if (llAllCopiedLinks == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

		// Free old links
		e = freeLL(llAllLinks);
		if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

		// Create a new list of links which we will now fill in
		llAllLinks = getLL();
		if (llAllLinks == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


		// If inverting the selection:
		// 		make a loop 1...255, and add every link that does not
		// 		exist in the llAllLinks LL (copied)
		for (int iAddingInverse = 1; iAddingInverse < 256; iAddingInverse++)
		{
			String * cCur = getStringFromChar((char)iAddingInverse);
			if (cCur == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			if (llAllCopiedLinks->iLen != 0)
			{
				String * found = (String *)LLgetItemByPredicate(llAllCopiedLinks, (*strequals), cCur);
				if (found == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

				if (found)
				{
					// If String link was found, remove the link from the LinkedList
					bool bRemoved = LLremoveItemAddr(llAllCopiedLinks, found);
					if (bRemoved == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
					
					//e = freeStr(cCur);
					//if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				}
				else
				{
					// If not found, simply add the normal way
					for (int iAddingLinksLoop = 0; iAddingLinksLoop < iPrevAcceptLen; iAddingLinksLoop++)
					{
						aPrevAccept[iAddingLinksLoop]->accept = false;
						e = addLink(aPrevAccept[iAddingLinksLoop], cCur, sNextState);
						if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
					}
					e = LLappend(llAllLinks, cCur);
					if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				}
			}
			else
			{
				// If we've eliminated all items in the copied List
				// 		then we don't have to perform cumbersome LL searches every iteration
				// 		simply add every new character as we come to it
				for (int iAddingLinksLoop = 0; iAddingLinksLoop < iPrevAcceptLen; iAddingLinksLoop++)
				{
					aPrevAccept[iAddingLinksLoop]->accept = false;
					e = addLink(aPrevAccept[iAddingLinksLoop], cCur, sNextState);
					if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				}
				e = LLappend(llAllLinks, cCur);
				if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			}
		}
	}

	// Base links is the LL of all links converted to a normal array
	set->aBaseLinks    = (State **)LLgetAll(llAllLinks);
	
	if (set->aBaseLinks == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	set->iBaseLinksLen = llAllLinks->iLen;

	mlCurrentMach->iEnd = iCharRetrievalLoop + 1;
	mlCurrentMach->sSubstring = substring(strRegex, mlCurrentMach->iStart, mlCurrentMach->iEnd);

	if (mlCurrentMach->sSubstring == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	return mlCurrentMach;
	
}
MLink * starMachine(String * strRegex, int iRegexStart, State ** aPrevAccept, int iPrevAcceptLen, MLink * mlCurrentMach, MLink * mlPrevMach)
{

	const string call = "::starMachine";

	int iNewAcceptLen   = (iPrevAcceptLen + mlPrevMach->data->iBaseLen);
	State ** aNewAccept = (State **)malloc(sizeof(State *) * iNewAcceptLen);
	if (!aNewAccept) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);

	// Localizing arrays
	State  ** aBegStates      = aPrevAccept;
	State  ** aPrevBaseStates = mlPrevMach->data->aBaseStates; 
	String ** aPrevBaseLinks  = mlPrevMach->data->aBaseLinks;

	// Initializing loop vars
	State  * sCurBeg;
	State  * sCurBase;
	String * sCurLink;
	State  * sCurStartState;

	LLNode * llStartListNode;

	// triple-nested loop that, for each beginning state, base state, and base link, 
	// 		adds a new link leading from the beginning state to the base state, by the base link
	for (int outer = 0; outer < iPrevAcceptLen; outer++)
	{
		sCurBeg = aBegStates[outer];
		sCurBeg->accept = true;    // ensuring that ending states of prev machine are accepting (I can't think of a situation where they wouldn't already be accepting, but better safe than sorry)
		aNewAccept[outer] = sCurBeg;
		for (int innerinner = 0; innerinner < mlPrevMach->data->iBaseLinksLen; innerinner++)
		{
			sCurLink = aPrevBaseLinks[innerinner];
			for (int inner = 0; inner < mlPrevMach->data->iBaseLen; inner++)
			{
				sCurBase = aPrevBaseStates[inner];
				sCurBase->accept = true;	// turn accepting status of current base state to accepting ('*' machine is defined as ZERO or more repitions)
				
				// Finding the list of starting states of prev machine by using the current link on the current base
				kvPair * kvpItem = HTgetItem(sCurBase->links, sCurLink);
				if (kvpItem == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				
				llStartListNode = ((LinkedList *) (kvpItem->data))->front;
				// llStartListNode = n->front;
				while (llStartListNode)
				{

					// Only add the link if the destination is a part of the current machine
					void * aGot = LLgetItemAddr(mlPrevMach->data->llAllStates, llStartListNode->data);
					if (aGot == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
					if (aGot)
					{
						// Foreach state in starting state list,
						// 		make a link going from the current accepting state to the 
						// 		current starting state, by the current link
						sCurStartState = llStartListNode->data;
						Error * e = addLink(sCurBeg, sCurLink, sCurStartState);
						if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
					}
					llStartListNode = llStartListNode->next;
				}
			}
		}
	}
	if (!mlCurrentMach) return NULL;

	// Creating list of accepting states for this machine
	for (int loop = iPrevAcceptLen; loop < iNewAcceptLen; loop++)
	{
		aNewAccept[loop] = aPrevBaseStates[loop - iPrevAcceptLen];
	}
	

	// Creating this star machine

	mlCurrentMach->data = (Machine *)malloc(sizeof(Machine));
	if (mlCurrentMach->data)
	{

		// Establishing inner/outer connections by:
		// 		connecting the current machine's inner to the previous machine,
		// 		connecting the current machine's outer to the previous machine's outer,
		// 		if the previous machine's outer exists, and its inner is equal to the previous machine, then reset it to the current machine
		// 				(we do it like this because all machines in a scope '()' machine's outer link are the scope, but the scope machine only holds one
		// 				link to the first machine in the scope)
		// 		connecting the previous mashine's outer to the current machine
		mlCurrentMach->inner = mlPrevMach;
		mlCurrentMach->outer = mlPrevMach->outer;
		if (mlPrevMach->outer && mlPrevMach->outer->inner == mlPrevMach) mlPrevMach->outer->inner = mlCurrentMach;
		mlPrevMach->outer = mlCurrentMach;

		mlCurrentMach->data->machineType = STAR;

		// Establishing next/prev connections by:
		// 		connecting current machine's next/prev to inner machine's next/prev
		// 		if inner machine's next/prev exist, set next's prev/prev's next to current machine
		mlCurrentMach->next = (mlPrevMach->next != mlCurrentMach) ? mlPrevMach->next : NULL;
		mlCurrentMach->prev = (mlPrevMach->prev != mlCurrentMach) ? mlPrevMach->prev : NULL;
		if (mlPrevMach->next && mlPrevMach->next != mlCurrentMach) mlPrevMach->next->prev = mlCurrentMach;
		if (mlPrevMach->prev && mlPrevMach->prev != mlCurrentMach) mlPrevMach->prev->next = mlCurrentMach;

		// Setting inner machine's next prev to NULL
		mlPrevMach->next = NULL;
		mlPrevMach->prev = NULL;
	
		// Setting further positional data:
		mlCurrentMach->iEnd   = iRegexStart + 1;
		mlCurrentMach->iStart = mlPrevMach->iStart;
		mlCurrentMach->sSubstring = substring(strRegex, mlCurrentMach->iStart, mlCurrentMach->iEnd);

		if (mlCurrentMach->sSubstring == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

		// star machines are passable
		mlCurrentMach->bIsPassable = true;
	
		// Setting machine data
		mlCurrentMach->data->llAllStates   = LLcopy(mlPrevMach->data->llAllStates);
		
		if (mlCurrentMach->data->llAllStates == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

		mlCurrentMach->data->aAccepting    = aNewAccept;
		mlCurrentMach->data->iAcceptingLen = iNewAcceptLen;
		mlCurrentMach->data->iBaseLen      = mlPrevMach->data->iBaseLen;
		mlCurrentMach->data->aBaseStates   = mlPrevMach->data->aBaseStates;
		mlCurrentMach->data->aBaseLinks    = mlPrevMach->data->aBaseLinks;
		mlCurrentMach->data->iBaseLinksLen = mlPrevMach->data->iBaseLinksLen;

	}
	else
	{
		return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
	}

	return mlCurrentMach;
}
MLink * plusMachine(String * strRegex, int iRegexStart, State ** aPrevAccept, int iPrevAcceptLen, MLink * mlCurrentMach, MLink * mlPrevMach)
{
	const string call = "::plusMachine";

	// Localizing arrays
	State  ** aBegStates      = aPrevAccept;
	State  ** aPrevBaseStates = mlPrevMach->data->aBaseStates; 
	String ** aPrevBaseLinks  = mlPrevMach->data->aBaseLinks;

	// Initializing loop vars
	State  * sCurBeg;
	State  * sCurBase;
	String * sCurLink;
	State  * sCurStartState;

	LLNode * llStartListNode;

	// triple-nested loop that, for each beginning state, base state, and base link, 
	// 		adds a new link leading from the beginning state to the base state, by the base link
	for (int outer = 0; outer < iPrevAcceptLen; outer++)
	{
		sCurBeg = aBegStates[outer];
		sCurBeg->accept = true;    // ensuring that ending states of prev machine are accepting (I can't think of a situation where they wouldn't already be accepting, but better safe than sorry)
		for (int innerinner = 0; innerinner < mlPrevMach->data->iBaseLinksLen; innerinner++)
		{
			sCurLink = aPrevBaseLinks[innerinner];
			for (int inner = 0; inner < mlPrevMach->data->iBaseLen; inner++)
			{
				sCurBase = aPrevBaseStates[inner];
				sCurBase->accept = false;    // ensuring that ending states of prev machine are not accepting (I can't think of a situation where they wouldn't already be not accepting, but better safe than sorry)
				
				// Finding the list of starting states of prev machine by using the current link on the current base
				kvPair * kvpGot = (HTgetItem(sCurBase->links, sCurLink)->data);
				if (kvpGot == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				
				llStartListNode = ((LinkedList *) kvpGot)->front;
				while (llStartListNode)
				{
					// Only add the link if the destination is a part of the current machine
					void * aGot = LLgetItemAddr(mlPrevMach->data->llAllStates, llStartListNode->data);
					if (aGot == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

					if (aGot)
					{
						// Foreach state in starting state list,
						// 		make a link going from the current accepting state to the 
						// 		current starting state, by the current link
						sCurStartState = llStartListNode->data;
						Error * e = addLink(sCurBeg, sCurLink, sCurStartState);
						if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
					}
					llStartListNode = llStartListNode->next;
				}

			}
		}
	}

	if (!mlCurrentMach) return NULL;

	// Creating this plus machine
	mlCurrentMach->data = (Machine *)malloc(sizeof(Machine));
	if (mlCurrentMach->data)
	{
		// Establishing inner/outer connections by:
		// 		connecting the current machine's inner to the previous machine,
		// 		connecting the current machine's outer to the previous machine's outer,
		// 		if the previous machine's outer exists, and its inner is equal to the previous machine, then reset it to the current machine
		// 				(we do it like this because all machines in a scope '()' machine's outer link are the scope, but the scope machine only holds one
		// 				link to the first machine in the scope)
		// 		connecting the previous mashine's outer to the current machine
		mlCurrentMach->inner = mlPrevMach;
		mlCurrentMach->outer = mlPrevMach->outer;
		if (mlPrevMach->outer && mlPrevMach->outer->inner == mlPrevMach) mlPrevMach->outer->inner = mlCurrentMach;
		mlPrevMach->outer = mlCurrentMach;

		mlCurrentMach->data->machineType = STAR;

		// Establishing next/prev connections by:
		// 		connecting current machine's next/prev to inner machine's next/prev
		// 		if inner machine's next/prev exist, set next's prev/prev's next to current machine
		mlCurrentMach->next = (mlPrevMach->next != mlCurrentMach) ? mlPrevMach->next : NULL;
		mlCurrentMach->prev = (mlPrevMach->prev != mlCurrentMach) ? mlPrevMach->prev : NULL;
		if (mlPrevMach->next && mlPrevMach->next != mlCurrentMach) mlPrevMach->next->prev = mlCurrentMach;
		if (mlPrevMach->prev && mlPrevMach->prev != mlCurrentMach) mlPrevMach->prev->next = mlCurrentMach;


		// Setting inner machine's next prev to NULL
		mlPrevMach->next = NULL;
		mlPrevMach->prev = NULL;

		// Setting further positional data:
		mlCurrentMach->iEnd   = iRegexStart + 1;
		mlCurrentMach->iStart = mlPrevMach->iStart;
		mlCurrentMach->sSubstring = substring(strRegex, mlCurrentMach->iStart, mlCurrentMach->iEnd);
		
		if (mlCurrentMach->sSubstring == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

		// plus machines are not passable
		mlCurrentMach->bIsPassable = false;


		// Setting machine data
		mlCurrentMach->data->machineType   = PLUS;
		mlCurrentMach->data->llAllStates   = LLcopy(mlPrevMach->data->llAllStates);

		if (mlCurrentMach->data->llAllStates == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

		mlCurrentMach->data->aAccepting    = aPrevAccept;
		mlCurrentMach->data->iAcceptingLen = iPrevAcceptLen;
		mlCurrentMach->data->iBaseLen      = mlPrevMach->data->iBaseLen;
		mlCurrentMach->data->aBaseStates   = mlPrevMach->data->aBaseStates;
		mlCurrentMach->data->aBaseLinks    = mlPrevMach->data->aBaseLinks;
		mlCurrentMach->data->iBaseLinksLen = mlPrevMach->data->iBaseLinksLen;
	}
	else
	{
		return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
	}

	return mlCurrentMach;
}
MLink * questionMachine(String * strRegex, int iRegexStart, State ** aPrevAccept, int iPrevAcceptLen, MLink * mlCurrentMach, MLink * mlPrevMach)
{

	const string call = "::questionMachine";

	// A question machine is the same as a star machine, except there are no links made between the end of the previous machine and
	// 		the beginning of the previous machine


	int iNewAcceptLen   = (iPrevAcceptLen + mlPrevMach->data->iBaseLen);
	State ** aNewAccept = (State **)malloc(sizeof(State *) * iNewAcceptLen);
	if (!aNewAccept) propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);

	// Localizing arrays
	State  ** aBegStates      = aPrevAccept;
	State  ** aPrevBaseStates = mlPrevMach->data->aBaseStates; 

	// Adding current accepting states to accepting states 
	// 		for this question machine
	for (int loop = 0; loop < iPrevAcceptLen; loop++)
	{
		aNewAccept[loop] = aBegStates[loop];
		aNewAccept[loop]->accept = true;
	}

	// Adding base states to list of accepting states
	// 		and turns base accepting-ness to true
	for (int loop = iPrevAcceptLen; loop < iNewAcceptLen; loop++)
	{
		aNewAccept[loop] = aPrevBaseStates[loop - iPrevAcceptLen];
		aNewAccept[loop]->accept = true;
	}
	
	// Creating this star machine

	mlCurrentMach->data = (Machine *)malloc(sizeof(Machine));
	if (mlCurrentMach->data)
	{

		// Establishing inner/outer connections by:
		// 		connecting the current machine's inner to the previous machine,
		// 		connecting the current machine's outer to the previous machine's outer,
		// 		if the previous machine's outer exists, and its inner is equal to the previous machine, then reset it to the current machine
		// 				(we do it like this because all machines in a scope '()' machine's outer link are the scope, but the scope machine only holds one
		// 				link to the first machine in the scope)
		// 		connecting the previous mashine's outer to the current machine
		mlCurrentMach->inner = mlPrevMach;
		mlCurrentMach->outer = mlPrevMach->outer;
		if (mlPrevMach->outer && mlPrevMach->outer->inner == mlPrevMach) mlPrevMach->outer->inner = mlCurrentMach;
		mlPrevMach->outer = mlCurrentMach;

		mlCurrentMach->data->machineType = QUE;

		// Establishing next/prev connections by:
		// 		connecting current machine's next/prev to inner machine's next/prev
		// 		if inner machine's next/prev exist, set next's prev/prev's next to current machine
		mlCurrentMach->next = (mlPrevMach->next != mlCurrentMach) ? mlPrevMach->next : NULL;
		mlCurrentMach->prev = (mlPrevMach->prev != mlCurrentMach) ? mlPrevMach->prev : NULL;
		if (mlPrevMach->next && mlPrevMach->next != mlCurrentMach) mlPrevMach->next->prev = mlCurrentMach;
		if (mlPrevMach->prev && mlPrevMach->prev != mlCurrentMach) mlPrevMach->prev->next = mlCurrentMach;

		// Setting inner machine's next prev to NULL
		mlPrevMach->next = NULL;
		mlPrevMach->prev = NULL;
	
		// Setting further positional data:
		mlCurrentMach->iEnd   = iRegexStart + 1;
		mlCurrentMach->iStart = mlPrevMach->iStart;
		mlCurrentMach->sSubstring = substring(strRegex, mlCurrentMach->iStart, mlCurrentMach->iEnd);

		if (mlCurrentMach->sSubstring == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

		// star machines are passable
		mlCurrentMach->bIsPassable = true;
	
		// Setting machine data
		mlCurrentMach->data->machineType   = PLUS;
		mlCurrentMach->data->llAllStates   = LLcopy(mlPrevMach->data->llAllStates);

		if (mlCurrentMach->data->llAllStates == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

		mlCurrentMach->data->aAccepting    = aNewAccept;
		mlCurrentMach->data->iAcceptingLen = iNewAcceptLen;
		mlCurrentMach->data->iBaseLen      = mlPrevMach->data->iBaseLen;
		mlCurrentMach->data->aBaseStates   = mlPrevMach->data->aBaseStates;
		mlCurrentMach->data->aBaseLinks    = mlPrevMach->data->aBaseLinks;
		mlCurrentMach->data->iBaseLinksLen = mlPrevMach->data->iBaseLinksLen;

	}
	else
	{
		return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
	}

	return mlCurrentMach;
}
MLink * curlyMachine(Regex * rRegex, String * strRegex, int iRegexStart, State ** aPrevAccept, int iPrevAcceptLen, MLink * mlCurrentMach, MLink * mlPrevMach)
{

	const string call = "::curlyMachine";

	// char array buffer that holds all characters between
	// 		opening and closing '{}' characters
	string sHoldingBuffer = (string)calloc(sizeof(char), strRegex->len - iRegexStart);
	if (!sHoldingBuffer) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);


	// Secondary holding buffer for storing secondary argument, if any exist
	string sHoldingBuffer2 = (string)calloc(sizeof(char), strRegex->len - iRegexStart);
	if (!sHoldingBuffer2) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);


	// Because there are two potential buffers to write to,
	// 		and those two buffers will be written to in the 
	// 		exact same way, we point and write to the first buffer
	// 		until the first comma is found
	// 		and the second buffer once the first comma is found
	string sTargetBuffer = sHoldingBuffer;
	int iTargetInsert = 0;

	// Flag for closing '}' character found
	// 		if never found, return error
	bool bCloseFound = false;

	// Flag for having found a comma in the bracketed section
	int iCommaEncountered = 0;

	// Loop variable --> held outside of the loop
	// 		because it is used outside of the loop later
	int iLoop;


	for (iLoop = iRegexStart + 1; iLoop < strRegex->len; iLoop++)
	{
		int iConvert = strRegex->data[iLoop];
		// Valid characters inside the '{}' curly machine:
		// 		0-9
		if ((iConvert - '0' >= 0 && iConvert - '0' <= 9))
		{
			sTargetBuffer[iTargetInsert++] = strRegex->data[iLoop];
		}
		else if (iConvert == ',')
		{
			// The curly machine is invalid if:
			// 		there is more than one comma in the 
			// 		'{}' brackets, or if the comma is the first
			// 		character in the '{}' brackets
			if (iCommaEncountered)
			{
				string s1 = (string) calloc(sizeof(char), 350);
				sprintf_s(s1, "Regex error!\n\tThere can only be one comma withing the curly '{}' brackets of a repetition machine to denote seperation between\n\tminimum and maximum values!\n\t\tRegex: %s\n\t\t", strRegex->data);
				string s2 = (string) calloc(sizeof(char), iRegexStart + 7 + iLoop);
				for (int inner = 0; inner < iRegexStart + iLoop + 6; inner++) s2[inner] = ' ';
				string s3 = "^ Location of rogue comma.";
				String * s2s3 = strAddStringFreeBoth(getStringPtr(s2), getStringPtr(s3));
				free(s2);
				 
				String * err = strAddStringFreeBoth(getStringPtr(s1), s2s3);
				free(s1);
				return propagateError(err, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);	
			}
			if (iLoop == iRegexStart + 1)
			{
				sHoldingBuffer[0] = "0";
				iTargetInsert++;
			}
			sHoldingBuffer[iTargetInsert] = '\0';
			// Swapping buffers
			sTargetBuffer = sHoldingBuffer2;
			iTargetInsert = 0;
			// Setting comma flag
			iCommaEncountered = iLoop;
		}
		else if (iConvert == '}')
		{
			// Breaking when closing '}' is found
			bCloseFound = true;
			sTargetBuffer[iTargetInsert] = '\0';
			break;
		}
		else if (iConvert == ' ');
		else if (iConvert == '\n');
		else if (iConvert == '\t');
		else
		{
			char cCharVers = (int) iConvert;
			// If current character is not 0-9, ',', or '}'
			// 		curly machine is invalid
			string s1 = (string) calloc(sizeof(char), 350);
			sprintf_s(s1, "Regex error!\n\tCharacters between the opening curly bracket '{' and closing curly bracket '}' in repetition machine,\n\tmust be either whitespace, 0-9, or a comma.\n\t'%c' is not a valid character!\n\t\tRegex: %s\n\t\t", cCharVers, strRegex->data);
			string s2 = (string) calloc(sizeof(char), iRegexStart + 7 + iLoop);
			for (int inner = 0; inner < iRegexStart + 6 + iLoop; inner++) s2[inner] = ' ';
			string s3 = "^ Location of rogue character.";
			String * s2s3 = strAddStringFreeBoth(getStringPtr(s2), getStringPtr(s3));
			free(s2);
			 
			String * err = strAddStringFreeBoth(getStringPtr(s1), s2s3);
			free(s1);
			return propagateError(err, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);	
		}
	}

	// If a closing bracket was not found,
	// 		return ERROR
	if (!bCloseFound)
	{
		string s1 = (string) calloc(sizeof(char), 350);
		sprintf_s(s1, "Regex error!\n\tOpening repetition machine character '{' found, with no closing pair.  Did you mean '\\{' to for the literal sequence character '{'?\n\t\tRegex: %s\n\t\t", strRegex->data);
		string s2 = (string) calloc(sizeof(char), iRegexStart + 8);
		for (int iLoop = 0; iLoop < iRegexStart + 7; iLoop++) s2[iLoop] = ' ';
		string s3 = "^ Location of opening '[' char-set character.";
		String * s2s3 = strAddStringFreeBoth(getStringPtr(s2), getStringPtr(s3));
		free(s2);
		 
		String * err = strAddStringFreeBoth(getStringPtr(s1), s2s3);
		free(s1);
		return propagateError(err, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);	
	}
	
	// Converting holding buffers into usable ints


	// minReps is easy because is program has gotten to this point
	// 		sHolding buffer is garuanteed to have a non-zero length
	// 		and only digits
	int iMinReps = atoi(sHoldingBuffer);
	
	// Max reps are harder because 
	// 		maxReps can be ommitted entirely
	// 		maxReps could be infinite
	// 		or maxReps could be an integer
	int iMaxReps;
	if (iCommaEncountered)
	{
		// If there is a comma in the curly brackets
		// 		check if the comma was the last character in the 
		// 		brackets
		if (iCommaEncountered != iLoop - 1)
		{
			// If it was not, then set maxReps to
			// 		the int version of the substring after the comma
			// Curly brackets must look like: {minReps, maxReps}
			iMaxReps = atoi(sHoldingBuffer2);
		}
		else
		{
			// Else, brackets look like: {minReps,}, which tells us that the curly machine
			// 		can repeate and infinite amount of times
			// 		set maxReps to -2, the special number that
			// 		will tell us that the sequence can repeat an infinite amount of times
			iMaxReps = -2;
		}
	}
	else
	{
		// If there was not comma, there are no max reps,
		// 		the machine repeats exactly the amount
		// 		of times that as the minReps
		// Curly bracket must look like: {minReps}
		iMaxReps = NOT_FOUND;
	}

	// Free buffers
	free(sHoldingBuffer);
	free(sHoldingBuffer2);

	// printf("holding1: %d\n", iMinReps);
	// printf("holding2: %d\n", iMaxReps);

	// printf("%s\n", mlPrevMach->sSubstring->data);

	// Storing the previously-accepting states of the 
	// 		machine
	State ** aPrevStates;
	int iPrevLen;

	// Next machine in the sequence
	MLink * mlNext = mlPrevMach;
	MLink * mlPrev = mlPrevMach;

	LinkedList * llAllStates = LLcopy(mlPrevMach->data->llAllStates);
	if (llAllStates == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	// In the special case where minReps is equal to 0,
	// 		we need to make this machine 
	// 		1) bPassable = true,
	// 		2) make sure all previous accepting states
	// 				remain accepting states
	if (iMinReps == 0)
	{
		iPrevLen = (iPrevAcceptLen + mlPrevMach->data->iBaseLen);
		aPrevStates = (State **)malloc(sizeof(State *) * iPrevLen);
		if (!aPrevStates) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);

		// Creating new array of 'prev' states
		// First by:
		// 		adding all items from the base states of the previous machine
		for (int loop = 0; loop < mlPrevMach->data->iBaseLen; loop++)
		{
			aPrevStates[loop] = mlPrevMach->data->aBaseStates[loop];
			// And setting their accepting-ness to true
			mlPrevMach->data->aBaseStates[loop]->accept = true;
		}
		// And then by:
		// 		adding all items from the beginning states of this machine
		for (int loop = 0; loop < iPrevAcceptLen; loop++)
		{
			aPrevStates[loop + mlPrevMach->data->iBaseLen] = aPrevAccept[loop];
		}

	}
	else
	{
		aPrevStates = aPrevAccept;
		iPrevLen = iPrevAcceptLen;


		for (int loop = 0; loop < iMinReps - 1; loop++)
		{
			
			mlNext = nextMachine(rRegex, mlPrevMach->sSubstring, 0, aPrevStates, iPrevLen, NULL, mlPrev, NEXT, NULL, false);
			if (mlNext == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		
			llAllStates = LLaddLL(llAllStates, mlNext->data->llAllStates);
			if (llAllStates) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			// Shifting "prev" machine forward
			mlPrev = mlNext;

			aPrevStates = mlNext->data->aAccepting;
			iPrevLen = mlNext->data->iAcceptingLen;


		}
	}
	LinkedList * llAllAccepting = LLify(aPrevStates, iPrevLen);
	
	// Setting iMax to the difference between
	// 		iMaxReps and iMinReps
	int iMax = iMaxReps;
	if (iMinReps == 0)
	{
		// Unless iMinReps == 0, in which case
		// 		iMax -> iMaxReps - 1
		iMax -= 1;
	}
	else
	{
		iMax -= iMinReps;
	}

	if (iMaxReps > 0)
	{
		// Same basic functionality for the maxReps as there are for the minReps, but
		// 		we also need to make sure every accepting item from the
		// 		old reps stays accepting after the new machine is added
		// Because a curly machine represents a minimum amount of repitions as well
		// 		as a maximum, with any number in between also working
		// We must also add all previously accepting states to the list of accepting states from above
		for (int outer = 0; outer < iMax; outer++)
		{
			// TODO:
			// 		make all machines no longer accept a regex as their 
			// 		first argument, but, instead, a String
			// TODO:
			// 		do this in the not-hacky way that I am currently doing it
			
			mlNext = nextMachine(rRegex, mlPrevMach->sSubstring, 0, aPrevStates, iPrevLen, NULL, mlPrev, NEXT, NULL, false);
			if (mlNext == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		
			llAllStates = LLaddLL(llAllStates, mlNext->data->llAllStates);
			if (llAllStates == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			// Shifting "prev" machine forward
			mlPrev = mlNext;

			for (int inner = 0; inner < iPrevLen; inner++)
			{
				// Turn accepting-ness to true
				aPrevStates[inner]->accept = true;

				// Append state to list of accepting states
				Error * e = LLappend(llAllAccepting, aPrevStates[inner]);
				if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			}

			
			// Reset prev states and len for next iteration
			aPrevStates = mlNext->data->aAccepting;
			iPrevLen = mlNext->data->iAcceptingLen;


		}
		// In the end, make sure to add previously accepting states to the 
		// 		accepting list, as well as setting their accepting-ness to true
		for (int inner = 0; inner < iPrevLen; inner++)
		{
			// Turn accepting-ness to true
			aPrevStates[inner]->accept = true;

			// Append state to list of accepting states
			Error * e = LLappend(llAllAccepting, aPrevStates[inner]);
			if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		}
	}
	else if (iMaxReps == -2)
	{
		// Here we emulate a star or plus machine, but only on the latest machine in the 
		// 		built by the iMinReps loop
		// Doing function pointer stuff because I can and I am frustrated :)
		// 		function-to-call ==> star machine if minReps == 0 (making it passable)
		// 						 ==> plus machine otherwise (making it not passable)
		MLink*(*func)(String *, int, State**, int, MLink*, MLink*)
			= (iMinReps == 0) ?
				(*starMachine):
				(*plusMachine);

		// Calling function (plus machine or star machine) with prev machine's substring
		// 		starting from 0 with prevAccepting, NULL current machine, and the latest
		// 		machine built in the minReps loop as args
		// When plus or star machine is called with a NULL current machine, it skips all 
		// 		operations pertaining to the current machine, and only makes the needed links
		Error * e  = func(mlPrevMach->sSubstring, 0, aPrevStates, iPrevLen, NULL, mlNext);
		if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	}
	else if (iMaxReps == NOT_FOUND);


	mlCurrentMach->data = (Machine *)malloc(sizeof(Machine));
	if (mlCurrentMach->data)
	{

		// Establishing inner/outer connections by:
		// 		connecting the current machine's inner to the previous machine,
		// 		connecting the current machine's outer to the previous machine's outer,
		// 		if the previous machine's outer exists, and its inner is equal to the previous machine, then reset it to the current machine
		// 				(we do it like this because all machines in a scope '()' machine's outer link are the scope, but the scope machine only holds one
		// 				link to the first machine in the scope)
		// 		connecting the previous mashine's outer to the current machine
		mlCurrentMach->inner = mlPrevMach;
		mlCurrentMach->outer = mlPrevMach->outer;
		if (mlPrevMach->outer && mlPrevMach->outer->inner == mlPrevMach) mlPrevMach->outer->inner = mlCurrentMach;
		mlPrevMach->outer = mlCurrentMach;

		mlCurrentMach->data->machineType = CURLY;

		// Establishing next/prev connections by:
		// 		connecting current machine's next/prev to inner machine's next/prev
		// 		if inner machine's next/prev exist, set next's prev/prev's next to current machine
		mlCurrentMach->next = (mlPrevMach->next != mlCurrentMach) ? mlPrevMach->next : NULL;
		mlCurrentMach->prev = (mlPrevMach->prev != mlCurrentMach) ? mlPrevMach->prev : NULL;
		if (mlPrevMach->next && mlPrevMach->next != mlCurrentMach) mlPrevMach->next->prev = mlCurrentMach;
		if (mlPrevMach->prev && mlPrevMach->prev != mlCurrentMach) mlPrevMach->prev->next = mlCurrentMach;

		// Setting inner machine's next prev to NULL
		mlPrevMach->next = NULL;
		mlPrevMach->prev = NULL;
	
		// Setting further positional data:
		mlCurrentMach->iEnd   = iRegexStart + (iLoop - iRegexStart) + 1;
		mlCurrentMach->iStart = mlPrevMach->iStart;
		mlCurrentMach->sSubstring = substring(strRegex, mlCurrentMach->iStart, mlCurrentMach->iEnd);
		// star machines are passable
		mlCurrentMach->bIsPassable = true;
	
		// Setting machine data
		mlCurrentMach->data->llAllStates   = llAllStates;
		mlCurrentMach->data->aAccepting    = LLgetAll(llAllAccepting);
		mlCurrentMach->data->iAcceptingLen = llAllAccepting->iLen;
		mlCurrentMach->data->iBaseLen      = mlPrevMach->data->iBaseLen;
		mlCurrentMach->data->aBaseStates   = mlPrevMach->data->aBaseStates;
		mlCurrentMach->data->aBaseLinks    = mlPrevMach->data->aBaseLinks;
		mlCurrentMach->data->iBaseLinksLen = mlPrevMach->data->iBaseLinksLen;

		if (mlCurrentMach->data->aAccepting) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	}
	else
	{
		return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
	}

	return mlCurrentMach;

}


// Adds a String link, "link", from State ptr "from" to State ptr "to".
bool addLink(State * from, String * link, State * to)
{
	const string call = "::addLink";


	if (to == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	if (!to) return propagateError(getStringPtr("NullPointerError!  Attempting to add a NULL State to a Regex!"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
	
	if (from == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	if (!from) return propagateError(getStringPtr("NullPointerError!  Attempting link from a NULL state!"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
	
	if (link == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	if (!link) return propagateError(getStringPtr("NullPointerError!  Attempting to use a NULL String as a transition!"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);

	// Loop vars
	HashTable * table = from->links;
	kvPair * found = NULL;
	

	// Search table for link
	if (table->size)
	{
		found = HTgetItem(from->links, link);
		if (found == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	}

	// If the transition was not previously in the state's links HT
	if (!found)
	{
		// If the link was not found (MCB):

		// Create the LinkedList that the will be used as the "value" in from's key-value pair hashtable
		LinkedList * in = getLL();
		if (in == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

		Error * e = LLappend(in, to);
		if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

		// Inserting LL into HT
		e = HTinsert(from->links, link, in);
		if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	}
	// If the transition *was* in the state's links HT
	else
	{
		// Simply add the target state to the LL that this key hashes to
		LinkedList * list = found->data; 
		void * aGot = LLgetItemAddr(list, to);
		if (aGot == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		if (!aGot)
		{
			Error * e = LLappend(list, to);
			if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		}
	}

	return true;
}

bool secretRegexMatchAll(State * stStartState, String * strTestString)
{

	const string call = "::secretRegexMatchAll";

	String * universal = getStringPtr("un");

	// Setting start point to start of regex (or subregex)
	State * curState = stStartState;

	LinkedList * transitions = NULL;
	kvPair * data = NULL;

	string charArr;
	String * link;
	for (int loop = 0; loop < strTestString->len; loop++)
	{
		charArr = (string)malloc(sizeof(char) * 2);
		if (charArr)
		{

			// Getting destinations of universal links
			data = HTgetItem(curState->links, universal);
			if (data == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			
			// Adding universal transitions into transitions list (if they exist)
			transitions = data ? data->data : NULL;

			// Turning current character in string into a String * pointer
			charArr[0] = (strTestString->data)[loop];
			charArr[1] = '\0';
			link = getStringPtr(charArr);
			if (link == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			
			// Getting destinations of currentCharacter link
			data = HTgetItem(curState->links, link);
			if (data == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			Error * e;

			// Adding non-universal transitions into transitions LL
			transitions = data ? 
				(transitions ? LLaddLL(
						(e = LLcopy(transitions)) == error ? 
							propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false) : 
							e, data->data
						) 
					: data->data
				) 
				: transitions;


			if (data == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			// If there is at least one valid transition from this state to the next:
			if (transitions)
			{
				if (transitions->iLen == 1)
				{
					// There is only one valid transition to the next (MCB), simply reset
					// 		curState to the next one determinsitically
					curState = LLgetItemIndex(transitions, 0);
					if (curState == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				}
				else
				{

					// If there are more than one valid transitions from the current state to the next
					// 		then we must recursively call this function on every possible 
					// 		transition going out from this state
					
					// Getting the remaining substring of the test string
					string srem = strTestString->data + loop + 1;
					String * sRem = getStringPtr(srem);
					if (sRem == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

					// Loop vars
					bool verified;
					LLNode * ptr = transitions->front;
					while (ptr)
					{
						// recursively calls this function on the next valid state in the transition
						// 		table
						verified = secretRegexMatchAll((State *) (ptr->data), sRem);
						if (verified == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false); 
						if (verified)
						{
							/*free(srem);
							free(sRem);*/
							free(charArr);
							free(link);

							// If any of the next states cause a full match return true
							return true;
						}
						ptr = ptr->next;
					}
					free(charArr);
					free(link);
					
					// Return false if none of the recursive calls returned true
					return false;
				}
			}
			else
			{
				free(charArr);
				free(link);
				return false;
			}

			free(charArr);
			free(link);
		}
		else
		{
			return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
		}
		transitions = NULL;
	}
	return curState && curState->accept;
}

bool regexFullMatch(Regex* reg, String* test)
{
	const string call = "::regexFullMatch";
	bool bMatched = secretRegexMatchAll(reg->start, test);
	if (bMatched == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	return bMatched;
}

/*

How to determine a Match:
	by Luke Callaghan


When a certain transition takes the State machine to a State that has an associated machine
		that is not equal to the current machine being matched:
		-	This is considered a new match.
				-	But, when a machine with an outer machine is matched we need to consider
						a few conditions about whether or not we should add the match to the
						match report:
						-	If the new machine's outer is different from the last machine's outer
								(and they are not equal because they are both NULL), we also 
								have to add the previous outer machine to the match report
						-	As well as, possibly, the next machine's outer 
				-	So, while the outer machines of the previous match and the current match are
						different, we must consider adding both the outer and the inner match to
						the match report



	Consider a Regex:

	Regex String: "a((b))c"

	regex structured like: SEQ "a" -> SCOPE "((b))" -> SEQ "c"
							 	        |
								        V
								      SCOPE "(b)"
								        |
										V
									   SEQ "b"


	-	When generateMatchReport detects that there is a difference in machines for SEQ "a" 
			and	SEQ "b", it will:
			1) create the match for SEQ "a", and, as SEQ "a" has no outer machine, it will leave 
					it at that
			2) Create a LinkedList of current potential Matches by creating a new Match for each 
					outer machine of SEQ "b", including SEQ "b"
					-	Modeled like `SEQ "b" -> SCOPE "(b)" -> SCOPE "((b))"
	-	When generateMatchRport later detects that there is a difference in machines for SEQ "b"
			and SEQ "c", it will:
			1) Create the match for SEQ "b" ... this will always happen when a difference in
					base-level machines is detected
			2) Then, remove the first item of the potential matches LinkedList and test the NEW
					first item, (now, SCOPE "(b)"), against the outer of the new machine, SEQ "c",
					which is NULL.  We add SEQ "c" to a NEW LinkedList of potential matches.
					-	While the head of the old LinkedList of potential matches is different 
							from the outer of the current machine, remove the head Match of old 
							LL, finish the match, add the match to the match report, and, if the 
							outer is not NULL add that to the new LL of matches
			3) Now that we have a LinkedList of remaining Matches from the old machine and
					new matches in the new machine, we prepend the new list in front of the old list
					and set that to the main match list
	-	But this leads to a problem, as the match report will include these matches:
			-	SEQ "a", SCOPE "((b))", SCOPE "(b)", SEQ "b", SEQ "c"
			-	SCOPE "((b))", SCOPE "(b)", and SEQ "b" are, for all intents and purposes,
					the same match. 
			-	If we wanted to split a test String "abc" by this match report, we'd get"
					"['a', 'b', 'b', 'b', 'c']" which is not correct
					-	So we need some way to determine what is the most significant machine 
							to be added to the match report
			-	Luckily for us, this is not TOO big of a change to what we previously had:
					-	Let's backtrack:
						-	When generateMatchRport later detects that there is a difference in machines for SEQ "b"
								and SEQ "c", it will:
								1) Create the match for SEQ "b" ... this will always happen when a difference in
										base-level machines is detected
								2) Then, remove the first item of the potential matches LinkedList and test the NEW
										first item, (now, SCOPE "(b)"), against the outer of the new machine, SEQ "c",
										which is NULL.  We add SEQ "c" to a NEW LinkedList of potential matches.
										-	While the head of the old LinkedList of potential matches is different 
												from the outer of the current machine:
													-	Previously, we would pop the head of the LinkedList, finish, and add
															the match to the MatchReport no matter what, but, again we must
															determine only the MOST SIGNIFICANT match
													-	So, instead of automatically adding the popped Match, we now must add
															all popped Matches to a THIRD! LinkedList of Matches:
															-	We now have a LinkedList of:
																	-	non-different Matches between previous machine
																			and next machine, llOldMatches
																	-	different Matches between previous machine and
																			next machine, llFinishedMatches
																	-	new matches to append to the front of non-different
																			matches, llNewMatches
															-	Instead of adding all the matches in llFinishedMatches, we need
																	to add only the significant ones:
										-	pseud:
												LinkedList * llFinishedMatches = NULL;
												LinkedList * llNewMatches = NULL;
												LinkedList * llOldMatches = ... ;
												State * newState = ... ;
												MLink * target = newState->parent;
												while (((Match *) llOldMatches->front->data)->mMatchMach != target)
												{
													if (!llNewMatches)
													{
														// llNewMatches and llFinishedMatches are always either simultaneously
														// 		NULL or not NULL; they are never out of sync
														// So only one test, for llNewMatches is necessary
														llNewMatches = getLL();
														llFinishedMatches = getLL();
													}

													// Pop from head of llOldMatches:
													Match * popped = LLgetItemIndex(llOldMatches, 0);
													LLremoveItemIndex(llOldMatches, 0);

													// Finish old match:
													popped->iEnd = [ current index in string ... either "iStartStr", or "loop" ];
													popper->sSubstring = substring(strTestString, popped->iBeg, popped->iEnd);

													// Add to llFinishedMatches:
													LLappend(llFinishedMatches, popped);

													if (target)
													{
														// If the target still exists, add it to llNewMatches:
														
														// Create a new match for this machine
														Match maNewMatch = {
															.iBeg = [ current index in string ... either "iStartStr", or "loop" ],
															.iEnd = -1,
															.sSubstring = NULL,
															.mMatchMach = target
														};

														// Append newly-constructed match to llNewMatches
														LLappend(llNewMatches, maNewMatch);

														// Advance target
														target = target->outer;
													}
												}
								3) Then, to find the significant matches, we go through llFinishedMatches to find only the 
										significant matches
										-	We do this by iteratively comparing matches in llFinishedMatches:
											-	On each iteration we compare some pointer element against another pointer,
													deeper in the list:
													-	Call them pointer p1, p2:
													-	p1 initialized to llFinishedMachines->head
													-	p2 initialized to llFinishedMachines->head->next;
													-	If p2 != NULL:
														-	If (p1.iEnd - p1.iBeg) < (p2.iEnd - p2.iBeg):
																-	Keep both p1 and p2 in the list
																-	temp = p2->next;
																-	p1 = p2;
																-	p2 = temp;
														-	If (p1.iEnd - p1.iBeg) == (p2.iEnd - p2.iBeg):
																-	Remove p1 from the list
																-	temp = p2->next;
																-	p1 = p2;
																-	p2 = temp;
														-	else:
																A case where p1.lenght > p2.length should never exist
										-	After all this, llFinishedMatches should be a list of all the significant 
												matches 
												-	We can now add these matches to the match report
								4) Now that we have a LinkedList of remaining Matches from the old machine and
										new matches in the new machine, we prepend the new list in front of the old list
										and set that to the main match list
	-	Now, we can ensure that when "abc" is split by MatchReport on Regex "a((b))c", the substrings we recieve
			will be "['a', 'b', 'c']"


			

	Consider a Regex:

	Regex String: "a((b)c)d"
	
	regex structured like: SEQ "a" -> SCOPE "((b)(c))" -> SEQ "d"
							 	        |
								        V
								      SCOPE "(b)" -> SCOPE "(c)"
								        |			   |
									   V			   V
									SEQ "b"		 SEQ "c"
*/






// MatchReport secretGenerateMatchReport(MatchReport * mrCurReport, LinkedList * llCurrentMatches, State * start, String * strTestString, int iStrStart)

Error * MRcheckAdding(Regex * rRegex, MatchReport * mrCurReport, LinkedList * llCurrentMatches, State * start, String * strTestString, int iStrStart)
{
	const string call = "::MRcheckAdding";


	// Function variables
	LinkedList * llFinMatches = getLL();	// Finished matches is the LL that will hold all the matches that were closed by this state
	LinkedList * llNewMatches = getLL();		// New matches is the LL that will hold all the new matches creates by this state
	LinkedList * llRemMatches = getLL();		// Remaining matches is the LL that will hold all the matches that were *not* closed by this state
	
	if (llFinMatches == error) {
		printf("0"); 
		return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	}
	if (llNewMatches == error) {
		printf("1"); 
		return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	}
	if (llRemMatches == error) {
		printf("2"); 
		return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	}
	
	
	if (!start)
	{
		// if Start is null, that means that we've reached the end of the 
		// 		test string.
		// So we finish all the matches and skip down to adding finished matches
		llFinMatches = LLcopy(llCurrentMatches);
		LLNode * ptr = llFinMatches->front;
		while (ptr)
		{
			// Finishing matches by setting iEnd and 
			// 		sSubstring
			Match * finished = (Match *) ptr->data;
			LLNode * next = ptr->next;

			finished->iEnd = iStrStart;
			if (finished->iEnd <= finished->iBeg)
			{
				
				bool bRemoved = LLremoveItemAddr(llFinMatches, finished);
				if (bRemoved == error)
				{
					return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				}
				bRemoved = LLremoveItemAddr(llCurrentMatches, finished);
				if (bRemoved == error)
				{
					return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				}

			}
			else
			{
				finished->sSubstring = substring(strTestString, finished->iBeg, finished->iEnd);
				if (finished->sSubstring == error) {
					printf("3"); 
					return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				}

			}

			ptr = next;
		}
		// Skipping to adding finished matches
		goto ADD_FIN;
	}

	bool bTargFound;
	Match * maCurMatch;
	MLink * mlCurMach;
	for (MLink * target = start->parent; target; target = target->outer)
	{
		// Traversing "outwards" in the new machine
		// 		(meaning through ptr = ptr->outer as the step in this loop)
		bTargFound = false;
		for(LLNode * llnCurrent = llCurrentMatches->front; llnCurrent; llnCurrent = llnCurrent->next)
		{
			// Traversing linearly through the linked list of current, unfinished matches
			maCurMatch = (Match *) llnCurrent->data;
			mlCurMach  = maCurMatch->mMatchMach;
			if (target == mlCurMach)
			{
				// If an outer machine of the current state is equal to a machine in the current machines
				// 		we add that machine to "remaining" matches
				bTargFound = true;
				Error * e = LLappend(llRemMatches, maCurMatch);
				if (e == error) {
					printf("4"); 
					return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				}
				break;
			}
		}

		if (!bTargFound)
		{
			// If the target was not found in the LL of current matches

			// Construct a new, unfinished, match
			Match * maNewMatch = (Match *)malloc(sizeof(Match));
			if (!maNewMatch) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
			maNewMatch->iBeg = iStrStart;
			maNewMatch->iEnd = -1;
			maNewMatch->sSubstring = NULL;
			maNewMatch->mMatchMach = target;
			maNewMatch->rRegex = rRegex;

			// And add it the llNewMatches LinkedList
			Error * e = LLappend(llNewMatches, maNewMatch);
			if (e == error) {
				printf("5"); 
				return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			}
		}
	}

	// The finished matches are all the matches in
	// 		llCurrentMatches that are *not* in llRemMatches
	llFinMatches = LLdifferenceAddr(llCurrentMatches, llRemMatches);
	if (llFinMatches == error) {
		printf("6"); 
		return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	}

	// Finishing off the finished matches in llFinishedMatches
	for (LLNode * llnPtr = llFinMatches->front; llnPtr; )
	{
		Match * maCur = llnPtr->data;
		LLNode * next = llnPtr->next;
		maCur->iEnd = iStrStart;
		if (maCur->iEnd <= maCur->iBeg)
		{

			bool bRemoved = LLremoveItemAddr(llFinMatches, maCur);
			if (bRemoved == error)
			{
				return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			}
			bRemoved = LLremoveItemAddr(llCurrentMatches, maCur);
			if (bRemoved == error)
			{
				return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			}
		}
		else
		{
			maCur->sSubstring = substring(strTestString, maCur->iBeg, maCur->iEnd);
			if (maCur->sSubstring == error) {
				printf("7"); 
				return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			}
		}
		llnPtr = next;
	}

	if (llNewMatches->iLen != 0)
	{
		// If new matches exist:

		// Combine the remaining matches with the new matches
		llNewMatches = LLaddLL(llNewMatches, llRemMatches);
		if (llNewMatches == error) {
			printf("8"); 
			return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		}

		// Reset some other stuff that makes this work more goodererer
		LinkedList * temp = LLcopy(llNewMatches);
		if (temp == error) {
			printf("9"); 
			return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		}
		// free(llOldMatches);
		// freeLL(llNewMatches);
		llCurrentMatches->front = temp->front;
		llCurrentMatches->back  = temp->back;
		llCurrentMatches->iLen  = temp->iLen;
		llNewMatches = NULL;
		llRemMatches = NULL;
	}
	else
	{
		free(llNewMatches);
	}
ADD_FIN:
	if (llFinMatches->iLen != 0)
	{
		// If some amount of matches were closed off by this state:
		// 		we search for only the significant matches 
		// A match is considered insignificant if:
		// 		the iEnd of the substring and length of substring
		// 		are equal to those of any of their parents
		// BUT, if a match's machine is tagged, then the match is always
		// 		considered significant

		// Loop vars
		LLNode * llnMatchPtr1 = (LLNode *) llFinMatches->front;
		LLNode * llnMatchPtr2 = llnMatchPtr1 ? (LLNode *) llFinMatches->front->next : NULL;
	

		while (llnMatchPtr2)
		{

			Match * maPtr1 = (Match *) llnMatchPtr1->data;
			Match * maPtr2 = (Match *) llnMatchPtr2->data;

			int iPtr1Len = maPtr1->iEnd - maPtr1->iBeg;
			int iPtr2Len = maPtr2->iEnd - maPtr2->iBeg;

			if (iPtr1Len == iPtr2Len &&				// substring length is the same
				maPtr1->iEnd == maPtr2->iEnd &&		// end at the same character
				((Match *) llnMatchPtr1->data)->mMatchMach->tag == NULL)	// NOT tagged
			{
				// Because llFinishedMatches will always be ordered
				// 		with inner machines first and outer machines after
				// 		then we remove maPtr1 from finished matches
				// 		because it is considered insignificant
				Match * temp = llnMatchPtr1->data;
				Error * e = LLremoveItemAddr(llFinMatches, maPtr1);
				if (e == error) {
					printf("10"); 
					return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				}
				e = LLremoveItemAddr(llCurrentMatches, maPtr1);
				if (e == error)
				{
					return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				}
				
				freeStr(temp->sSubstring);
				//printf("cuck!");
				free(temp);
			}
			llnMatchPtr1 = llnMatchPtr2;
			llnMatchPtr2 = llnMatchPtr2->next;


		}

		// Add the finished matches to the current match report
		Error * e = LLaddLL(mrCurReport->llMatches, llFinMatches);
		if (e == error) {
			printf("11"); 
			return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		}

	}
	else
	{
		free(llFinMatches);
	}
	return NULL;
}


MatchReport * secretGenerateMatchReport(Regex * rRegex, MatchReport * mrCurReport, LinkedList * llCurrentMatches, State * start, String * strTestString, int iStrStart)
{
	const string call = "::secretRegexGenerateMatchReport";
	String * universal = getStringPtr("un");


	if (start)
	{
		bool bCanMatch = secretRegexMatchAll(start, getStringPtr(strTestString->data + iStrStart));
		if (bCanMatch == error) propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		if (!bCanMatch) return mrCurReport;
	}

	
	// Checking and adding any new matches if necessary
	Error * e = MRcheckAdding(rRegex, mrCurReport, llCurrentMatches, start, strTestString, iStrStart);
	if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	// Setting start point to start of regex (or subregex)
	State * curState = start;

	// LinkedList that will hold all possible transitions from the current state
	// 		to some next state by the current character in the test string
	LinkedList * transitions = NULL;

	// kvPair that holds a LinkedList of transitions as its data
	// 		this kvPair is retrived, on each iteration, from the 
	// 		current state's link hash table
	kvPair * data = NULL;

	string charArr;
	String * link;
	int loop;
	for (loop = iStrStart; loop < strTestString->len; loop++)
	{
		charArr = (string)malloc(sizeof(char) * 2);
		if (charArr)
		{
			// Transitioning:
			// 		Val of state->links HT --> a LinkedList of possible destinations
			// 			from the current state to the next state, by the current transition
			// 		First, we collect the destination of the universal link, then the destination
			// 			of the current character and we combine the two LinkedLists
			// 		And, for every item in that LinkedList we call secretGeneratMatchReport
			// 			on the destination state and the remaining string


			// Getting destinations of universal links
			data = HTgetItem(curState->links, universal);
			if (data == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			
			// Adding universal transitions into transitions list (if they exist)
			transitions = data ? data->data : NULL;

			// Turning current character in string into a String * pointer
			charArr[0] = (strTestString->data)[loop];
			charArr[1] = '\0';
			link = getStringPtr(charArr);
			if (link == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			
			// Getting destinations of currentCharacter link
			data = HTgetItem(curState->links, link);
			if (data == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


			// Now that we have the potential destination of the universal '.' links, stored in LL 'transitions',
			//		and the destination of the current character read as a link, stored in kvp 'data', we have to combine
			//		and / or remove them so that ll 'transitions' hold *all* possible destinations on the graph:

			if (data)
			{
				// If the current transition character produced a non-universal link:
				if (transitions)
				{

					// If a universal link for this state exists, 
					// 		combine the universal (held in "transitions") and
					// 		the non-universal links (held in data->data)
					LinkedList * cp = LLcopy(transitions);
					if (cp == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
					transitions = LLaddLL(cp, data->data);
					if (transitions == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				}
				else
				{
					// If a universal link for this state DOES exist, set the 
					// 		transitions LinkedList to the LL of destinations for
					// 		the current character on the current state
					transitions = data->data;
				}
			}

			// If there is at least one valid transition from this state to the next:
			if (transitions)
			{
				if (transitions->iLen == 1)
				{
					// There is only one valid transition to the next (MCB), simply reset
					// 		curState to the next one determinsitically
					curState = LLgetItemIndex(transitions, 0);
					if (curState == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

					Error * e = MRcheckAdding(rRegex, mrCurReport, llCurrentMatches, curState, strTestString, loop);
					if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

				}
				else
				{

					// If there are more than one valid transitions from the current state to the next
					// 		then we must recursively call this function on every possible 
					// 		transition going out from this state
					
					// Getting the remaining substring of the test string
					string srem = strTestString->data + loop + 1;
					String * sRem = getStringPtr(srem);
					if (sRem == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

					// Loop vars
					LLNode * ptr = transitions->front;
					while (ptr)
					{
						// recursively calls this function on the next valid state in the transition
						// 		table
						State * stNextState = (State *) ptr->data;
						String * rem = getStringPtr(strTestString->data + loop);
						bool bMatches = secretRegexMatchAll(stNextState, rem);
						if (bMatches == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

						if (bMatches)
						{

							Error * e = MRcheckAdding(rRegex, mrCurReport, llCurrentMatches, stNextState, strTestString, loop);
							if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

							MatchReport * mr = secretGenerateMatchReport(rRegex, mrCurReport, llCurrentMatches, ((State *) (ptr->data)), strTestString, loop + 1);
							if (mr == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
						}
						ptr = ptr->next;

					}
					free(charArr);
					free(link);
					return mrCurReport;
				}
			}
			else
			{
				free(charArr);
				free(link);
				goto CHECK_LAST;
			}

			free(charArr);
			free(link);
		}
		else
		{
			return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
		}
		transitions = NULL;
	}
CHECK_LAST:
	if (llCurrentMatches->front)
	{
		MLink * mlTarg = ((Match *) llCurrentMatches->front->data)->mMatchMach;
		State ** aAccepting = mlTarg->data->aAccepting;
		for (int iAcceptSearch = 0; iAcceptSearch < mlTarg->data->iAcceptingLen; iAcceptSearch++)
		{
			if (curState == aAccepting[iAcceptSearch])
			{
				Error * e = MRcheckAdding(rRegex, mrCurReport, llCurrentMatches, NULL, strTestString, loop);
				if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			}
		}
	}

	return mrCurReport;
}

MatchReport * regexGenerateMatchReport(Regex* rRegex, String* strTestString)
{

	const string call = "::regexGenerateMatchReport";


	MatchReport * mr = (MatchReport *)malloc(sizeof(MatchReport));
	if (!mr) return propagateError(getStringPtr("Allocation error!  Are you out of memeory?"), strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), true);
	
	// Match report variables
	mr->rPattern = rRegex;
	mr->strTestString = strTestString;
	mr->llMatches = getLL();
	mr->llTaggedMatches = getLL();
	mr->htTaggedMatches = NULL;

	if (mr->llMatches == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	if (mr->llTaggedMatches == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	// Generating a new match report for every character in the 
	//		in the testing string
	for (int loop = 0; loop < strTestString->len; loop++)
	{
		mr = secretGenerateMatchReport(rRegex, mr, getLL(), rRegex->start, strTestString, loop);
		if (mr == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	}



	// Removing insignificant matches from the match report
	LinkedList * llSet = LLsetify(mr->llMatches, (*bMatchEquals));
	if (llSet == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


	//printLLMatch(mr->llMatches);

	// Freeing old matches
	Error * e = freeLL(mr->llMatches);
	if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	mr->llMatches = llSet;

	// Tagged matches are only the matches that have non-null tags,
	//		so we filter all those out and set llTaggedMatches to
	//		the new LL
	mr->llTaggedMatches = LLfilter(mr->llMatches, (*bTaggedMatch), NULL);
	if (mr->llTaggedMatches == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	//printLLMatch(mr->llTaggedMatches);

	if (mr->llTaggedMatches->iLen > 0)
	{
		// Group matches by their tags
		HashTable * groupings = HTgroupLLBy(mr->llTaggedMatches, (*strGetMatchGroupingElement));
		if (groupings == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

		

		for (LLNode * llnPtr = groupings->keys->front; llnPtr; llnPtr = llnPtr->next)
		{
			// For every tag:
			kvPair * pair = llnPtr->data;
			LinkedList * llCurTagged = (LinkedList *) pair->data;
		
			// Find the max tag
			Match * match = (Match *) LLmax(llCurTagged, iTagCompare);
			if (match == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			if (!match->mMatchMach->bRepeatableTag)
			{
				LinkedList * llMax = getLL();
				if (llMax == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

			
				Error * e = LLappend(llMax, match);
				if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

				pair->data = llMax;
			}
		}

		mr->htTaggedMatches = groupings;

	}
	else
	{
		HashTable * tagged = getHT(2);
		if (tagged == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		mr->htTaggedMatches = tagged;
	}

	// Return report
	return mr;
}

void printMatch(void * vmatch)
{
	Match * match = (Match *) vmatch;
	printf("(tag: '%s'; substring: '%s'; start: %d, end: %d)\n", (match->mMatchMach->tag) ? match->mMatchMach->tag->data : NULL, match->sSubstring->data, match->iBeg, match->iEnd);
}

bool bMatchEquals(void * match1, void * match2)
{
	Match * maMatch1 = match1;
	Match * maMatch2 = match2;

	
	// Match is insignificant if:
	return (
		(strequals(maMatch1->sSubstring, maMatch2->sSubstring)					// substrings are the same
			&& maMatch1->mMatchMach->tag == NULL								// And both tags are NULL
			&& maMatch2->mMatchMach->tag == NULL								// Both tags are NULL
		)	
		||																		// OR

		(strequals(maMatch1->mMatchMach->tag, maMatch2->mMatchMach->tag)		// Tags are the same 
			&& maMatch1->sSubstring->len <= maMatch2->sSubstring->len			// And one match is the substring of another
			&& isSubstring(maMatch1->sSubstring, maMatch2->sSubstring)			// one match is a substring of another
		)
	)
	&& !secretRegexMatchAll(maMatch1->rRegex->start, maMatch1->sSubstring)		// AND, the substring doesn't fully match the regex
	&& !secretRegexMatchAll(maMatch1->rRegex->start, maMatch2->sSubstring)		// AND, the substring doesn't fully match the regex
	;;

}


String * strGetMatchGroupingElement(void * m)
{
	Match * match = m;
	return match->mMatchMach->tag;
}

bool bTaggedMatch(void * ignored, void * maCurMatch)
{
	return ((Match *) maCurMatch)->mMatchMach->tag != NULL;
}

int iTagCompare(void * m1, void *m2)
{
	Match * match1 = m1;
	Match * match2 = m2;

	return match1->sSubstring->len - match2->sSubstring->len;
}

HashTable * regexGetTaggedSubstrings(Regex * rRegex, String * strTestString)
{
	const string call = "::regexGetTaggedSubstrings";

	MatchReport * mr = regexGenerateMatchReport(rRegex, strTestString);
	if (mr == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false); 




	if (mr->llTaggedMatches->iLen > 0)
	{
		// Create returned hashtable
		HashTable * htMatches = getHT(mr->llTaggedMatches->iLen);
		if (htMatches == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false); 

		LLNode * llnPtr = mr->llTaggedMatches->front;
		while (llnPtr)
		{
			// For all tagged matches:
			Match * maCurMatch = (Match *) llnPtr->data;

			// Search for tag of current tagged match in HT:
			String * tag = maCurMatch->mMatchMach->tag;
			kvPair * kvp = HTgetItem(htMatches, tag);
			if (kvp == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			
			if (kvp)
			{
				// If this tag already exists in the 
				// 		hash table, then we just add
				// 		the current match to the end
				// 		of the list
				LinkedList * llTagList = kvp->data;
				Error * e = LLappend(llTagList, maCurMatch);
				if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			}
			else
			{
				// If the tag does not exist in the LinkedList, then
				// 		we make a new LinkedList and insert it into
				// 		the ht
				LinkedList * llTagList = getLL();
				if (llTagList == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

				Error * e = LLappend(llTagList, maCurMatch);
				if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
				
				e = HTinsert(htMatches, tag, llTagList);
				if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
			}
			llnPtr = llnPtr->next;
		}
		return htMatches;
	}
	else
	{
		// If no tagged matches exist, then we
		// 		return an empty HT
		Error * e = getHT(2);
		if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	}
	
	/*const string call = "::regexGetTaggedSubstrings";

	MatchReport * mr = regexGenerateMatchReport(rRegex, strTestString);
	if (mr == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	return mr->htTaggedMatches;*/
}


LinkedList * regexSplitAtMatches(Regex * rRegex, String * strTestString)
{
	const string call = "::regexSplitAtMatches";


	MatchReport * mr = regexGenerateMatchReport(rRegex, strTestString);
	if (mr == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);


	LinkedList * splits = getLL();
	if (splits == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);

	LLNode * llnPtr = mr->llMatches->front;
	while (llnPtr)
	{
		Match * maCurMatch = (Match *) llnPtr->data;
		String * cSplit = substring(strTestString, maCurMatch->iBeg, maCurMatch->iEnd); 
		if (cSplit == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		Error * e = LLappend(splits, cSplit);
		if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
		llnPtr = llnPtr->next;
	}

	if (splits->iLen == 0)
	{
		Error * e = LLappend(splits, strTestString);
		if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	}
	return splits;
}

Error * printLLMatch(void* vll)
{
	const string call = "::printLLMatch";
	Error * e = printLL(vll, (*printMatch));
	if (e == error) return propagateError(NULL, strAddStringFreeBoth(getStringPtr(regex_stack), getStringPtr(call)), false);
	return NULL;
}

void freeRegex(Regex * regex)
{

}