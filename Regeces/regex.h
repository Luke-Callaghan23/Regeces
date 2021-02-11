#ifndef REGEX_H
#define REGEX_H 0


#include "interpreter.h"
#include "string.h"
#include "hashtable.h"
#include "linkedlist.h"

#define State		struct _state
#define Regex		struct _regex
#define Match		struct _match
#define Machine		struct _machine
#define MLink	    struct _machine_link
#define MatchReport struct _match_report


// Types of machines
enum eMachTypes { STAR, PLUS, QUE, CURLY, SCOPE, SET, SCOPE_CLOSE, OR, TAG, OR_NODE=-3, SEQ=-2 };

struct _regex
{
	int			iStates;		// Running counter of the amount of states in this regex
	String    * regex;			// regex string
	State     * start;			// starting state
	MLink     * startMach;		// starting machine of the sequence
	HashTable * matches;		// (key: string, value: match array) hashtable
								// 		Whenever a string is tested against this regex, it is added to this hashtable
								// 		as a key, and its matches (if any) are added as the value.
								// 		Whenever a string is tested against a hashtable, we search here first
};


// A machine shell that holds links between a machine and
// 		links to other machine shells
// I could have put outer, inner, prev, and next fields in the
// 		machine struct, but there are already too many fields in 
// 		the machine struct, I didn't want to muddy the waters even more
// Stores only data about the position of this machine in a regex
struct _machine_link
{
	// Machine data
	Machine * data;				

	// Regex
	// Regex * parent;

	// Positional data of this machine in parent regex
	bool bIsPassable;
	int iStart;
	int iEnd;
	String * sSubstring;		// substring of regex string that this machine matches with
	
	String * tag;				// String tag of this machine
	bool bRepeatableTag;		// boolean determining whether or not this machine can
								//		produce multiple tagged matches

	// A bunch of links to other machine links
	MLink * outer;				// Outer machine used in sub machining like: 'a(a)a', 'a*', 'a|b' 
	MLink * inner;				// Inner machine used in sub machining like: 'a(a)a', 'a*', 'a|b' 
	MLink * prev;			    // Previous machine in a sequence of machines
	MLink * next;				// Next machine in sequence of machine
};

struct _machine
{
	State  ** aAccepting;		// accepting states of this machine
	State  ** aBaseStates;		// accepting states of previous machine
	String ** aBaseLinks;		// links that lead from previous machine to the beginning states of this machine
	int iAcceptingLen;			// length of accepting states
	int iBaseLen;				// length of base states
	int iBaseLinksLen;			// length of the array that holds the data for "base" transitions

	LinkedList * llAllStates;   // linked list of all states contained in this machine

	int machineType;
};

struct _match
{
	int iBeg;					// beginning index of match in inputted string (inlcusive)
	int iEnd;					// ending index of match in inputted string (exclusive)
	MLink  * mMatchMach;		// Machine that this match is tied to
	String * sSubstring;		// substring of input that this match is tied to
	Regex  * rRegex;
};


struct _match_report
{
	Regex  * rPattern;			   // Regex pattern that this MatchReport was generated by
	String * strTestString;		   // Inputted String that generated this MatchReport
	LinkedList * llMatches;		   // LinkedList of matches in this Match Report
	LinkedList * llTaggedMatches;
	HashTable  * htTaggedMatches;
};

struct _state
{
	int iStateId;				// State id for this state ... states start at zero and go to n
	bool accept;				// boolean indicating whether or not this state is an accepting state or not
	MLink * parent;				// parent machine that this state is contained within
	HashTable * links;			// (key: string (character), val: State *) hashtable
								// 		keys are characters, value is the next state of the machine that the key links to
};

Regex * getRegex(String *);
bool regexFullMatch(Regex *, String *);
void freeRegex(Regex *);

LinkedList * regexSplitAtMatches(Regex *, String *);
MatchReport * regexGenerateMatchReport(Regex *, String *);
LinkedList * regexSplitAtMatches(Regex *, String *);
HashTable * regexGetTaggedSubstrings(Regex *, String *);


Error * printLLMatch(void *);
void printMatch(void *);



#endif 
