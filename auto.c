/* 2007-04-25 - Nondeterministic Finite State Automata */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define N 20						/* Nbr of final/initial states for an automate */
#define buffer_ 128					/* Maximum size of the RE */
#define Epsilon '¤'					/* Epsilon character : HIDDEN character only used by the program */

#define OMANY	'+'					/* "One or Many" operator */
#define ZMANY 	'*'					/* "Zero or Many" operator */
#define OR 		','					/* OR operator : standard '|' doesn't work on every system */
#define AND 	'.'					/* AND operator : to make it easier, its use is forbidden here because 
										the program	automatically handles it */

/* State type definition 
	- index 		: is incremental
	- bool 			: determines whether it's a final state
	- transitions 	: stores the head of the transitions list
*/
typedef struct Sstate{
	int index;								
	bool final;								
	struct	Stransition *transitions;
}state;

/* Transition type definition
	- symbol : condition to move to the next state
	- nextS_ : state the transition goes to
	- nextT_ : following transition
*/
typedef struct Stransition{
	char	symbol;
	struct	Sstate *nextS_;
	struct	Stransition *nextT_;
}transition;

/* Automate type definition
	- initial 		: pointer to the initial state
	- finalList_	: list of final state
	- nbrFinal 		: index for finalList_
*/
typedef struct Sautomate{
	struct Sstate *initial;
	struct Sstate *finalList_[N];
	int nbrFinal;
}automate;

/* Agent type definition (list structure)
	- cursor	: position in the automate
	- next 		: next agent
*/
typedef struct Sagent{
	struct Sstate *cursor;
	struct Sagent *next;
}agent;

static int nbrStates = 0;							/* Nbr of current states */
static int nbrTrans = 0;                           	/* Nbrs of current transitions */
unsigned char option;								/* Option byte */
/*
Structure of the option byte
a7	a6	a5	a4	a3	a2	a1	a0
|	|	|	|	|	|	|	|
|	|	|	|	|	|	|	details of the transition
|	|	|	|	|	|	nbr of states
|	|	|	|	|	path of the automate
|	|	|	|	infos+
*/

state *createNewState(bool final)
{
	state *newState;
		
	newState = (state *)malloc( sizeof(state) );
	newState->index = nbrStates++;
	newState->final = final;
	newState->transitions = NULL;

	return newState;
}

transition * createNewTransition(char symb)
{
	transition *newTrans;
	
	newTrans = (transition *)malloc( sizeof(transition) );
	newTrans->symbol = symb;
	newTrans->nextS_ = NULL;
	newTrans->nextT_ = NULL;
	nbrTrans++;
	
	return newTrans;
}

/* 	Add new agent to the list.
	Agents explore path of the automate in order to match the RE
	Every time a path splits into 2 or more paths 1 or more agents
	are created and added to the list.
*/
void addAgent(state *s, agent **agency2)
{	
		agent *newAgent, *fatherAgent;
		bool hasABrother = false;
		
		newAgent = (agent *)malloc( sizeof(agent ) );
		/* If the list is not empty, the tail must point to the new agent */
		if (*agency2 != NULL)
		{
			/* Check whether the new agent doesn't belong to the list yet (has a brother) */
			fatherAgent = *agency2;
			
			while (fatherAgent != NULL)
			{
				if (fatherAgent->cursor->index == s->index)
					hasABrother = true;
				fatherAgent = fatherAgent->next;
			}

			if (!hasABrother)
			{
				/* agentSmith knows where to find the last agent (tail) */
				agent *agentSmith;
				agentSmith = (*agency2);
	
				while (agentSmith->next != NULL)
					agentSmith = agentSmith->next;

				agentSmith->next = newAgent;
			}
			/* If has a brother delete it */
			/*else
				free(newAgent);*/
		}	
		else
			*agency2 = newAgent;
		/* Set up the new agent */
		newAgent->cursor = s;
		newAgent->next = NULL;
}


/* Option "display transition" */
void displayTrans(transition *trans, state *start){if ((option & 1) == 1) printf("\nTransition created. %d---%c--->%d", start->index, trans->symbol, trans->nextS_->index);}
/* Option "display number of created states" */
void displayNbrStates(){if ((option & 2) == 2) printf("\nNumber of states : %d.\n", nbrStates);}
/* Option "display the path in the automate" */
void displayPath(int index){if ((option & 4) == 4)	printf("\n\tActual state : %d.", index);}
/* Option "infos+" */
void displayInfos(automate *auto1)
{
     int i = 0;
     if ((option & 8) == 8) 
     {
       puts("\nParameters of the automate");
       puts("====================");
       printf("Initial state : %d", auto1->initial->index);
       printf("\nFinal states :");
       
       for (i = 0; i < auto1->nbrFinal; i++)
               printf(" %d", auto1->finalList_[i]->index);
     }
}


/*	Creates the automate of a regular expression.
	The function is recursive and splits the RE until it becomes an axiom
	From there it will create the basic automate of the axiom and build it up
	-> returns the created automate 
*/
automate * createAuto(char *expR)
{
	/* If the RE is an axiom directly create the automte */
	if (strlen(expR) == 1)
	{
		automate *autoAxiome;
		state *stateIni, *stateFinal;
		transition *newTrans;
	
		/* Creates the elementary automate of the axiom (automate, 
			initial/final state, transitions, ...)*/		
		stateIni = createNewState(false);		
		stateFinal = createNewState(true);	
		newTrans = createNewTransition(expR[0]);
		newTrans->nextS_ = stateFinal;
		stateIni->transitions = newTrans;

		autoAxiome = (automate *)malloc(sizeof(automate));
		autoAxiome->initial = stateIni;
		autoAxiome->finalList_[0] = stateFinal;
		autoAxiome->nbrFinal = 1;
		/* If option enabled, display transition */
		displayTrans(newTrans, stateIni);
		
		return autoAxiome;										
	}
	/* If the RE ends with OMANY or ZMANY */
	else if ((expR[strlen(expR) - 1] == ZMANY) || (expR[strlen(expR) - 1] == OMANY))
	{
		transition *transTemp;
		automate *autoInf;
		int i;
		char c;

		/* Save the operator for treatment and create the inferior 
			automate */
		c = expR[strlen(expR) - 1];
		expR[strlen(expR) - 1] = '\0';
		autoInf = createAuto(expR);					

		/* 	For every final states, create an Epsilon transition to the initial state
		 	This allows the smart returns if an RE starts in the middle of anither one */
		for (i = 0; i < autoInf->nbrFinal; i++)
		{
			transition *newTrans;
			
			newTrans = createNewTransition(Epsilon);		
			transTemp = (autoInf->finalList_[i])->transitions;

			if (transTemp == NULL)
				(autoInf->finalList_[i])->transitions = newTrans;
			else
			{
				while (transTemp->nextT_ != NULL)
					transTemp = transTemp->nextT_;
				transTemp->nextT_ = newTrans;
			}
			/* Link the transition to the initial state */
			newTrans->nextS_ = autoInf->initial;
			/* If option enabled, display the created transition */
			displayTrans(newTrans, autoInf->finalList_[i]);
		}

		/* If ZMANY final state becomes initial state */
		if (c == ZMANY)
		{
			autoInf->initial->final = true;
			autoInf->finalList_[autoInf->nbrFinal] = autoInf->initial;		
			autoInf->nbrFinal++;
		}

		/* return the inferior level automate */
		return autoInf;
	}
	/* Otherwise, split the RE into 2 operands and 1 operator */
	else
	{
		automate *ope1, *ope2, *autoResult;
		
		char *string1, opr; /* *string2 */ /* Corresponding RE to ope1 and ope2 */
		int i;

		string1 = (char *)malloc(strlen(expR) * sizeof(char));

		/* Look for the operator */
		if (expR[0] == '(')
		{
			int openParenthesis = 1, closeParenthesis = 0;
			int position = 1; /* Position in the RE */

			/* While it's a n-level parenthesis move into expR */
			while (closeParenthesis != openParenthesis)
			{
				if (expR[position] == '(')
					openParenthesis++;
				if (expR[position] == ')')
					closeParenthesis++;
				position++;
			}

			/* 	Look if the expression is only in a n-level parenthesis
			 	If so, returns the automate inside the parentheses */
			if (position == strlen(expR))
			{
				expR[strlen(expR) - 1] = '\0';
				return createAuto(expR + 1);
			}
			/* Otherwise the operator is just right after the parenthesis */
			else
			{
				opr = expR[position];

				strcpy(string1, expR);
				/* The 1st operand stops where the parentheses are even */
				string1[position] = '\0';
                ope1 = createAuto(string1);
				/* 2nd operand is just the rest of the expression */
				ope2 = createAuto(expR + (position + 1));
			}
		}
		/* Otherwise it's "only" a word */
		else
		{
			strcpy(string1, expR);
			string1[1] = '\0';
			ope1 = createAuto(string1);
			opr = expR[1];
			ope2 = createAuto(expR + 2);
		}
		
		autoResult = (automate *)malloc(sizeof(automate));

		/* Do the ope1 opr ope2 operation and return the resulting automate */
		switch (opr)
		{
			/* New transitions / states for Or operator */
			state *stateIni;
			transition *t1, *t2;

			case AND:
				/* The initial state of the resulting automate is the ope1's one */
				autoResult->initial = ope1->initial;

				/* 	Link every final states of ope1 to ope2's initial state 
					Final states of ope1 are no more final states */
				for (i = 0; i < ope1->nbrFinal; i++)
				{
					transition *newTrans, *transTemp;
					
					newTrans = createNewTransition(Epsilon);
					transTemp = (ope1->finalList_[i])->transitions;

					if (transTemp == NULL)
						(ope1->finalList_[i])->transitions = newTrans;
					else
					{
						while (transTemp->nextT_ != NULL)
							transTemp = transTemp->nextT_;
						transTemp->nextT_ = newTrans;
					}
					newTrans->nextS_ = ope2->initial;
					ope1->finalList_[i]->final = false;
					/* If option enabled, display transitions */
					displayTrans(newTrans, ope1->finalList_[i]);
				}
				autoResult->nbrFinal = ope2->nbrFinal;

				for (i = 0; i < autoResult->nbrFinal; i++)
					autoResult->finalList_[i] = ope2->finalList_[i];

				return autoResult;
				break;

			case OR:
				/* 	Create 2 Epsilon transitions, one for each opeX. Then link 
					these transitions with the initial state of the automate */
				stateIni = createNewState(false);
				t1 = createNewTransition(Epsilon);
				t2 = createNewTransition(Epsilon);

				t1->nextS_ = ope1->initial;
				t2->nextS_ = ope2->initial;

				stateIni->transitions = t1;
				stateIni->transitions->nextT_ = t2;
				
				/* If option enabled display the transitions */
				displayTrans(t1, stateIni);
				displayTrans(t2, stateIni);

				autoResult->initial = stateIni;
				autoResult->nbrFinal = ope1->nbrFinal + ope2->nbrFinal;
				
				/* Combine both ope1 and ope2's final states */
				for(i = 0; i < ope1->nbrFinal; i++)
					autoResult->finalList_[i] = ope1->finalList_[i];
				for(i = 0; i < ope2->nbrFinal; i++)
					autoResult->finalList_[ope1->nbrFinal + i] = ope2->finalList_[i];

				return autoResult;
				break;
		}
	}
}


/* 	This browses every transition in order to find a match with the RE.
	If so it adds agent to the list (addAgent)
	Espilon transitions are considered to be direct link to the next "real transition"
*/
void exploreAuto(state *exploredState, agent **agency, agent **agency2, char symb)
{
	transition *transTemp;
	transTemp = exploredState->transitions;
	
	while (transTemp != NULL)
	{
		if (transTemp->symbol == symb)
			addAgent(transTemp->nextS_, agency2);

		else if (transTemp->symbol == Epsilon)
			exploreAuto(transTemp->nextS_, agency, agency2, symb);
		transTemp = transTemp->nextT_;
	}
}


/*  Main function for detecting RE (calls exploreAuto)
	Handle the agencies
	Agency is the current list of agents, agency2 is the new list.
	For every call agency is to be treated while agency2 is created for
	the next round
*/
void handleChar(char symb, agent **agency, agent **agency2)
{
	/* Add the initial state to the new agency */
	addAgent((*agency)->cursor, agency2);

	while ((*agency) != NULL)
	{
		agent *aSupp;
		aSupp = (*agency);
		(*agency) = (*agency)->next;

		exploreAuto(aSupp->cursor, agency, agency2, symb);
		free(aSupp);
	}
	(*agency) = (*agency2);
	(*agency2) = NULL;
}


/* 	Look if an agent managed to be in a final state which means the RE
	has been found
*/
bool isFound(agent**agency)
{
	agent *agentCarter; /* temporary agent */

	agentCarter = (*agency);

	while (agentCarter != NULL)
	{
		transition *transExplore;
		transExplore = agentCarter->cursor->transitions;
		/* If option enabled display path */
		displayPath(agentCarter->cursor->index);
		
		if (agentCarter->cursor->final == true)
			return true;
		/* Don't forget to go through Epsilon transition */
		while (transExplore != NULL)
		{
			if (transExplore->symbol = Epsilon)
				if (transExplore->nextS_->final == true)
					return true;
			transExplore = transExplore->nextT_;
		}

		agentCarter = agentCarter->next;
	}

	return false;
}


/* Handles a RE without parentheses */
char * handleBasicRE(char *string)
{
	/* Axiom or Axiom + operator*/
	if ((strlen(string)== 1) || ( (strlen(string) == 2) && (string[1]== ZMANY || string[1]== OMANY)))
	{
		char *renvoi;
		renvoi = (char *)malloc(7 * sizeof(char));

		if (strlen(string) == 2)
		{
			renvoi[0] = '(';
			renvoi[1] = string[0];
			renvoi[2] = string[1];
			renvoi[3] = ')';
			renvoi[4] = '\0';
		}
		/* If only Axiom */
		else
		{
			renvoi[0] = string[0];
			renvoi[1] = '\0';
		}
		return renvoi;
	}
	/* Otherwise split string into ope1 opr ope2 */
	else
	{
		char *ope2;
		char *opr;
		char *result;
		ope2 = (char *)malloc((strlen(string)-1) * sizeof(char));
		opr = (char *)malloc(2 * sizeof(char));
		result = (char *)malloc(buffer_*sizeof(char));
		result[0] = '\0';
		
		if ((string[1] == ZMANY) || (string[1] == OMANY))
		{
			/* If charachter without operator => automatically add AND operator */
			if ((string[2]<=122) && (string[2]>=97))
			{
				strcpy(opr, ".");
				strcpy(ope2, (string + 2));
			}
			else
			{
				strcpy(opr, ",");
				strcpy(ope2, (string + 3));
			}
			
			string[2] = '\0';
		}
		else
		{
			/* If charachter without operator => automatically add AND operator */
			if ((string[1]<=122) && (string[1]>=97))
			{
				strcpy(opr, ".");
				strcpy(ope2, (string + 1));
			}
			else
			{
				strcpy(opr, ",");
				strcpy(ope2, (string + 2));
			}

			string[1] = '\0';
		}
		/* Create the RE according to : (ope1 opr ope2) */
		strcat(result, "(");
		strcat(result, handleBasicRE(string));
		strcat(result, opr);
		strcat(result, handleBasicRE(ope2));
		strcat(result, ")");

		return result;
	}
}


/* Function that converts a RE given by the user into a program-readable RE */
char * convertER(char * string)
{
	char *ope2, *ope1;
	char *opr;
	char *result;
	ope1 = (char *)malloc(strlen(string) * sizeof(char));
	opr = (char *)malloc(2 * sizeof(char));
	result = (char *)malloc(buffer_*sizeof(char));
	ope1[0] = '\0';
	result[0] = '\0';
	
	if (string[0] == '(')
	{
		int openParenthesis = 1, closeParenthesis = 0, positionC = 1;

		while (openParenthesis != closeParenthesis)
		{
			
			if (string[positionC] == '(')
				openParenthesis++;
			
			if (string[positionC] == ')')
				closeParenthesis++;

			positionC++;
		}
		/* If reached the end of the parentheses, return what's inside */
		if (positionC == strlen(string))
		{
			string[positionC -1] = '\0';
			return convertER(string+1);
		}
		/* If there's an operator and wh've reached the end */
		else if (( (string[positionC] == ZMANY) || (string[positionC] == OMANY) ) && (positionC+1 == strlen(string)))
		{
			char *result;
			char c;
			c = string[positionC];
			result = (char *)malloc(buffer_ * sizeof(char));
			result[0] = '\0';

			strcat(result, "(");
			string[positionC-1] = '\0';
			strcat(result, convertER(string+1));
			if (c == ZMANY)
				strcat(result, "*");
			else
				strcat(result, "+");
			strcat(result, ")");
			return result;
		}
		else if ((string[positionC] == ZMANY) || (string[positionC] == OMANY))
		{
			/* Look for the operator and build ope2 */
			if (string[positionC+1] == OR)
			{
				opr[0] = string[positionC + 1];
				ope2 = string + positionC + 2;
			}
			else
			{
				opr[0] = AND;
				ope2 = string + positionC + 1;
			}
			opr[1] = '\0';

			/* Build ope1 (EXPR*) */
			strcpy(ope1, string);
			ope1[positionC + 1] = '\0';
		}
		/* Otherwise the operator is right next to it */
		else
		{
			if (string[positionC] == OR)
			{
				opr[0] = string[positionC];
				ope2 = string + positionC + 1;
			}
			else
			{
				opr[0] = AND;
				ope2 = string + positionC;
			}
			opr[1] = '\0';

			/* Removes the parenthesis */
			string[positionC - 1] = '\0';

			ope1 = string + 1;
		}
	}
	/* Otherwise it's a basic character */
	else
	{
		int positionC = 0;
		bool isParentez = false;

		while ((string[positionC] != '(') && (positionC != strlen(string)))
			positionC++;
		
		/* If no parentheses directly handles as a basic RE */
		if (positionC == strlen(string))
			return handleBasicRE(string);
		else
		{
			/* Look if the operator is AND or OR */
			if (string[positionC - 1] == OR)
			{
				opr[0] = OR;
				string[positionC - 1] = '\0';
			}
			else
			{
				opr[0] = AND;
				string[positionC] = '\0';
			}
			ope2 = string + positionC;
			opr[1] = '\0';
			ope1 = string;
		}

	}
	/* Create the RE as in : (ope1 opr ope2) */
	strcat(result, "(");
	strcat(result, convertER(ope1));
	strcat(result, opr);
	strcat(result, convertER(ope2));
	strcat(result, ")");

	return result;
}


/* Check if the RE is valid */
bool isValidRe(char *word)
{
	int openParenthesis = 0, closeParenthesis = 0, positionM = 0;
	bool isForbiddenOpr = false;

	while (positionM != strlen(word))
	{
		/* If detect an open parenthesis */
		if (word[positionM] == '(')
			openParenthesis++;
		/* If detect a close parenthesis */
		if(word[positionM] == ')')
			closeParenthesis++;
		/* If detect the forbidden operator And' */
		if(word[positionM] == AND)
			isForbiddenOpr = true;
		positionM++;
	}

	/* If parentheses is wrong */
	if (openParenthesis != closeParenthesis)
	{
		printf("\nIncorrect parentheses");
		return true;
	}
	/* Or if forbidden AND operator exists */
	if (isForbiddenOpr)
	{
		printf("\nForbidden operator AND used");
		return true;
	}
	/* Otherwise it's OK */
	return false;
}

int main(int argc, char **argv)
{
	FILE *f;
	/* Character read in the file */
	char c;
	/* Escape character */
	char escape;
	/* Saving the position in the file */
	int position = 0;

	/* An option has been defined */
	if (argc == 4)
	{
		char c;
		int i = 0;

		while (i != strlen(argv[3]))
		{
			c = argv[3][i];
			switch (c)
			{
				/* Display transitions */
				case't':
					option = option | 1;
				break;
				/* Display the nbr of states */
				case'n':
					option = option | 2;
				break;
				/* Path in the automate */
				case'p':
					option = option | 4;
				break;
				/* Parameters of the automate */
				case'i':
					option = option | 8;
				break;
			}
			i++;
		}
	}

	/* The number of parameters is not valide */
	if (argc < 3)
	{
		puts("\nAutomate v2.5 Copyright (c)");
		puts("\nUsage : auto <word>|<RE> <text_file_path> [<option1><option2>...<optionN>]");
		puts("\n<option> : \n\t n - display the number of created states");
		puts("\t t - display created transitions");
		puts("\t p - display path in the automate");
		puts("\t i - display parameters of the automate");
		/*printf("\n\t v - Visual Mode");*/
		puts("\nrecognised operators :\n\t * - zero or many");
		puts("\t + - one or many");
		printf("\t %c - OR\n", OR);
		puts("\t . represents AND not handled here but not necessary");
    }
	/* The RE is wrong */
	else if (isValidRe(argv[1]))
		printf("\nThe regular expression is wrong");
	/* The file doesn't exist */
	else if ((f=fopen(argv[2], "r")) == NULL)
            printf("\nERROR, the file doesn't exist.");
	else
	{
		char * ER;
		agent *agency, *agency2; /* Heads of lists of agents */
		automate *autoResult;
		
		/* 	Convert the RE given by the user into a program-readable RE.
			Then create the resulting automate */
		ER = convertER(argv[1]);
		/*printf("\n\n\nexpr : %s \n\n", ER);*/
		autoResult = createAuto(ER);
        puts("");

        /* Options */
        displayInfos(autoResult);
		displayNbrStates();

		/* Set the agencies in order to explore from the initial state */
		agency = (agent *)malloc(sizeof(agent));
		agency->cursor = autoResult->initial;
		agency->next = NULL;
		agency2 = NULL;

		/* Read the file character by character */
		while ((fscanf(f, "%c", &c)) != EOF)
		{
			position++;
			handleChar(c, &agency, &agency2);
			/* If set display the corresponding path */
			if ((option & 4) == 4)
				printf("\n\nReading character %c :", c);
			if (isFound(&agency))
				printf("\nRegular expression found at : %d", position);
		}
		fclose(f);
	}
}
