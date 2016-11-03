/*
 * tokenizer.c
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**********TOKENIZER STRUCT - DEFINITION AND TYPEDEF**********/

struct TokenizerT_ {
	char * tokenstream;	//Used to store finalized token stream
	int * type;		//0 - Octal, 1 - Hex, 2 - decimal, 3 - float, 4 - zero

	char * tsptr;		//Used to find the location of the next token in the tokenstream - since tokenstream is malloced
	int * typeptr;		//Used to find the location of the next type in the int * type - since type is malloced

};

typedef struct TokenizerT_ TokenizerT;

/***************State Definitions***************/

int state0(char * pt, char *tknstr, int * sot){	//First char was a 0

	if( isdigit(*pt) != 0 && *pt < '8' ) { //Octal Digits pass -octal
		*tknstr = *pt;
		(*sot)++;
		return 1;
	}

	if( *pt == 'x' || *pt == 'X'){ //'X' & 'x' pass - hexadecimal
		*tknstr = *pt;
		(*sot)++;
		return 2;
	}

	if( *pt == '.'){	//'.' passes - float
		*tknstr = *pt;
		(*sot)++;
		return 3;
	}

	if( isspace(*pt) != 0 || *pt == '\0'){ //Token: "0" Type: zero
		//DONE
		return -1;
	}

	//Print out escape character and end token
	printf("Error [0x%02x] \n", *pt);
	return -2;
}

int state1(char * pt, char *tknstr, int * sot){	//Token should be octal - check for octal digits - COMPLETES OCTAL

	if( isdigit(*pt) != 0 && *pt < '8' ) { //Octal digits pass - Octal
		*tknstr = *pt;
		(*sot)++;
		return 1;
	}

	if( isspace(*pt) != 0 || *pt == '\0'){ //Token: "0..." Type: octal
		//DONE
		return -1;
	}

	//Print out escape character and end token
	printf("Error [0x%02x] \n", *pt);
	return -2;
}

int state2(char * pt, char *tknstr, int * sot){	//Token is hex ('x') - ONLY HEX DIGITS WILL PASS THIS STATE

	if( isxdigit(*pt) != 0){ //Hex digits will pass - Hexadecimal
		*tknstr = *pt;
		(*sot)++;
		return 4;
	}

	//Token cannot end here - Invalid token
	return 22;
}

int state3(char * pt, char *tknstr, int * sot) {	//Token should be float ('.') - ONLY DIGITS 1-9 WILL PASS THIS STATE

	if( isdigit(*pt) != 0 ){ //Digits will pass - Float
		*tknstr = *pt;
		(*sot)++;
		return 5;
	}

	//Token cannot end here - Invalid token
	return 23;
}

int state4(char * pt, char *tknstr, int * sot){	//Check for hex digits - COMPLETES HEX

	if( isxdigit(*pt) != 0 ){ //Hex digits will pass
		*tknstr = *pt;
		(*sot)++;
		return 4;
	}

	if( isspace(*pt) != 0 || *pt == '\0' ){// Token: "0x..." Type: Hexadecimal
		//DONE
		return -1;
	}

	//Print out escape character and end token
	printf("Error [0x%02x] \n", *pt);
	return -2;

}

int state5(char * pt, char *tknstr, int * sot){	//Check for float digits

	if( isdigit(*pt) != 0 ){ //Digits will pass - float
		*tknstr = *pt;
		(*sot)++;
		return 5;
	}

	if(*pt == 'e' || *pt == 'E'){ //'E' & 'e' will pass - float
		*tknstr = *pt;
		(*sot)++;
		return 7;
	}

	if( isspace(*pt) != 0 || *pt == '\0'){ //Token: "[...].[...]" Type: float
		//DONE
		return -1;
	}

	//Print out escape character and end token
	printf("Error [0x%02x] \n", *pt);
	return -2;

}

int state6(char * pt, char *tknstr, int * sot){	//First char is 1-9 - Could be an int - check for int digits

	if( isdigit(*pt) != 0 ){ //Only digits will pass - decimal
		*tknstr = *pt;
		(*sot)++;
		return 6;
	}

	if( *pt == '.'){ //'.' will pass - float
		*tknstr = *pt;
		(*sot)++;
		return 3;
	}

	if(*pt == 'e' || *pt == 'E'){ //'e' & 'E' will pass - float
		*tknstr = *pt;
		(*sot)++;
		return 7;
	}

	if( isspace(*pt) != 0 || *pt == '\0'){ //Token: "[Some collection of ints]" Type: Decimal
		//DONE
		return -1;
	}

	//Print out escape character and end token
	printf("Error [0x%02x] \n", *pt);
	return -2;
}

int state7(char * pt, char *tknstr, int * sot){	//Token is a float (with an e,E)
	if(*pt == '+' || *pt == '-'){ //'+' & '-' will pass - float
		*tknstr = *pt;
		(*sot)++;
		return 8;
	}

	if( isdigit(*pt) != 0 ){ //Digits will pass - float
		*tknstr = *pt;
		(*sot)++;
		return 9;
	}

	//Token cannot end here - Invalid token
	return 27;

}

int state8(char * pt, char *tknstr, int * sot){	//Token is a float (with a +/-) - ONLY DIGITS 1-9 WILL PASS THIS STEP

	if( isdigit(*pt) != 0 ){ //Digits will pass - float
		*tknstr = *pt;
		(*sot)++;
		return 9;
	}

	//Token cannot end here - Invalid token
	return 28;

}

int state9(char * pt, char *tknstr, int * sot){	//Check for remaining float digits

	if( isdigit(*pt) != 0 ){ //Digits will pass - float
		*tknstr = *pt;
		(*sot)++;
		return 9;
	}

	if( isspace(*pt) != 0 || *pt == '\0'){ //Token: "[Ints]('.' || 'e')[Ints]('e')[Ints]" Type: float
		//DONE
		return -1;
	}

	//Print out escape character and end token
	printf("Error [0x%02x] \n", *pt);
	return -2;
}

/**********OTHER HELPER FUNCTIONS**********/

//Skips Excess spaces and prints out escape chars - returns the pointer to the next location
char * killSpace(char * ts){
	while( isdigit(*ts) == 0 ){
		if( *ts == '\0') //End of token stream
			return ts;

		if( isspace(*ts) == 0 ){
			//Print out escape character
			printf("Error [0x%02x] \n", *ts);
		}

		ts++;
	}

	return ts;
}

//Adds the string to the token and returns the pointer to the next location
char * addToToken(char * token, char * ptr){
	while( *ptr != '\0'){
		*token = *ptr;
		token++;
		ptr++;
	}

	return token;
}

//Moves cursor back over a token to rewrite or erase it
char * removeLastToken(char * ptr, int * sot, char * front){
	char * newPos = ptr;
	newPos--;
	(*sot)--;

	while(*(newPos-1) != ' ' && newPos != front){ //Finds location of first char in current token
		newPos--;
		(*sot)--;
	}

	char * rdr = newPos; //Creates a new pointer to traverse removed token
	while(rdr != ptr){
		printf("Error [0x%02x] \n", *rdr); //Prints out each removed char in L -> R order
		rdr++;
	}

	//Defaults end to null byte
	*(newPos+1) = '\0';

	return newPos;
}

/*
 * TKCreate creates a new TokenizerT object for a given token stream
 * (given as a string).
 *
 * TKCreate should copy the arguments so that it is not dependent on
 * them staying immutable after returning.  (In the future, this may change
 * to increase efficiency.)
 *
 * If the function succeeds, it returns a non-NULL TokenizerT.
 * Else it returns NULL.
 *
 * You need to fill in this function as part of your implementation.
 */

TokenizerT *TKCreate( char * ts ) {
	//Skips Excess spaces and prints out escape chars - if it reaches the end no tokens exist
	ts = killSpace(ts);

	//Variable to track the next state the function should access
	int state;

	//Variables to measure how much space is taken up by the simplified token stream
	//Used to reallocate and take up less space at the end of the function
	int sizeOfTokens = 0;
	int *sot = &sizeOfTokens;
	int sizeOfTypes = 0;

	//Determmines how much space to allocate before finishing and realloc(ing)
	char * scanner = ts;
	int sizeofts = 1;
	while(*scanner != '\0'){
		sizeofts++;
		scanner++;
	}

	//Dynamically allocates space for the tokenizer
	TokenizerT *tokenizer = malloc(sizeof(TokenizerT));

	if(tokenizer == NULL){
		printf("Malloc failed for TokenizerT struct. Exiting...\n");
		exit(1);
	}

	//Allocates enough space for every char to be relevant - Will shrink size later
	(*tokenizer).tokenstream = malloc(sizeof(char)*sizeofts);
	(*tokenizer).type = malloc((sizeof(int)*(sizeofts/2)) + sizeof(int)); //Max possible types in tokenstream of n chars is (n/2)+1

	if( (*tokenizer).tokenstream == NULL ){
		printf("Malloc failed for tokenstream char* in the TokenizerT struct. Exiting...\n");
		exit(1);
	} else if( (*tokenizer).type == NULL){
		printf("Malloc failed for type int* in the TokenizerT struct. Exiting...\n");
		exit(1);
	}

	//Creates pointers that will be used to track the current token when creating token strings
	(*tokenizer).tsptr = (*tokenizer).tokenstream;
	(*tokenizer).typeptr = (*tokenizer).type;

	//Pointers to allow traversal & initalization of allocated space
	char * tknstrm = (*tokenizer).tokenstream;
	int * type = (*tokenizer).type;

	while(*ts != '\0'){
		*type = 2;	//Default type to integer (decimal)

		//Starting Cases - Either 0 or 6
		if(*ts == '0'){		//If its a zero, go to state 0
			state = 0;
			*type = 4;	//4 - zero; will change type if passed into any other state after state 0
			*tknstrm = *ts;
			sizeOfTokens++;

		} else if(*ts == '\0'){	//If its a null byte char, skip this step
			break;
		} else {		//Otherwise - must be 1-9 - go to state 6
			state = 6;
			*tknstrm = *ts;
			sizeOfTokens++;
		}


		while(state != -1) {	//Runs until ts points to a ' ' or a '\0'
			ts++;

			//Catches skipped characters
			//State is 10 more than actual state if last char was skipped - will return to same step
			//if(state <= 9)
				tknstrm++;
			//else
				//state -= 10;

			//TYPE: 0 - Octal, 1 - Hex, 2 - deicmal, 3 - float | Default to decimal
			switch(state){
				case 0: state = state0(ts,tknstrm,sot); break;			//First character is a zero
				case 1: state = state1(ts,tknstrm,sot); *type = 0; break;	//Token should be octal - Check for octal digits
				case 2: state = state2(ts,tknstrm,sot); *type = 1; break;	//Token is hex ('x')
				case 3: state = state3(ts,tknstrm,sot); *type = 3; break;	//Token should be float ('.')
				case 4: state = state4(ts,tknstrm,sot); break;			//Check for hex digits
				case 5: state = state5(ts,tknstrm,sot); break;			//Check for float digits
				case 6: state = state6(ts,tknstrm,sot); break;			//First char is 1-9 - Coud be an int - check for int digits
				case 7: state = state7(ts,tknstrm,sot); *type = 3; break;	//Token is a float (with an e,E)
				case 8: state = state8(ts,tknstrm,sot); break;			//Token is a float (with a +/-)
				case 9: state = state9(ts,tknstrm,sot); break;			//Check for remaining float digits
				default: state = -1; printf("An error has occured\n"); break;	//Should never reach this - just in case
			}

			//If token is invalid
			//State returned is 20 more than actual state
			if(state == -2){
				ts++;
				state = -1;
			} else if(state > 20){
				tknstrm = removeLastToken(tknstrm, sot, (*tokenizer).tokenstream);
				break;
			}

		}

		if(state < 20){
			sizeOfTypes++;

			//If token was not the last token, put a space in the stream
			//to seperate it from the next token
			if(*ts != '\0'){
				*tknstrm = ' ';
				tknstrm++;
				sizeOfTokens++;
				type++;
			}
		}

		//Skips Excess spaces and prints out escape chars
		ts = killSpace(ts);

	}

	//Puts a null byte character at the end of the string, finishing it
	tknstrm = '\0';
	sizeOfTokens++;

	//Reallocates just enough space for every char and int in the finalized stream
	(*tokenizer).tokenstream = realloc((*tokenizer).tokenstream, sizeOfTokens*sizeof(char));
	(*tokenizer).type = realloc((*tokenizer).type , sizeOfTypes*sizeof(int));

	if( (*tokenizer).tokenstream == NULL){
		printf("Realloc failed for tokenstream char* in the TokenizerT struct. Exiting...\n");
		exit(1);
	} else if( (*tokenizer).type == NULL && sizeOfTypes != 0){ //If sizeOfTypes is 0, there was nothing to store so realloc is ok to fail
		printf("Realloc failed for type int* in the TokenizerT struct. Exiting...\n");
		exit(1);
	}

	return tokenizer;
}

/*
 * TKDestroy destroys a TokenizerT object.  It should free all dynamically
 * allocated memory that is part of the object being destroyed.
 *
 * You need to fill in this function as part of your implementation.
 */

void TKDestroy( TokenizerT * tk ) {
	free((*tk).tokenstream);	//Frees the tokenstream pointer
	free((*tk).type);		//Frees the type pointer
	free(tk);			//Frees the struct itself
}

/*
 * TKGetNextToken returns the next token from the token stream as a
 * character string.  Space for the returned token should be dynamically
 * allocated.  The caller is responsible for freeing the space once it is
 * no longer needed.
 *
 * If the function succeeds, it returns a C string (delimited by '\0')
 * containing the token.  Else it returns 0.
 *
 * You need to fill in this function as part of your implementation.
 */

char *TKGetNextToken( TokenizerT * tk ) {
	//Make pointers to traverse streams without losing original pointers
	char * tsptr = (*tk).tsptr;
	int * typeptr = (*tk).typeptr;

	//If the string is empty just return null
	if(*tsptr == '\0'){
		return NULL;
	}

	//Counts how long the next token is to reduce memory allocation
	//Length will contain the number of chars in the token
	int length = 0;
	char * counter = tsptr;
	while(*counter != ' ' && *counter != '\0'){
		length++;
		counter++;
	}

	//Depending on type, chooses proper amount to malloc
	//This accounts for the space char between the token and the type,
	//Each char for the type and the null byte char at the end
	if(*typeptr == '1')
		length += 13;
	else if(*typeptr == '2')
		length += 8;
	else if(*typeptr == '4')
		length += 6;
	else
		length += 7;

	//Allocate just enough space for the token and its type in the string
	char * token = malloc(sizeof(char)*length);

	if(token == NULL){
		printf("Malloc failed for char * token in TKGetNextToken(). Exiting...\n");
		exit(1);
	}

	char * tknptr = token;

	//Determine and add the type to the output string
	if(*typeptr == 0) //Octal
		tknptr = addToToken(tknptr, "octal");

	else if(*typeptr == 1) //Hexadecimal
		tknptr = addToToken(tknptr, "hexadecimal");

	else if(*typeptr == 2) //Decimal
		tknptr = addToToken(tknptr, "decimal");

	else if(*typeptr == 3) //Float
		tknptr = addToToken(tknptr, "float");

	else //Zero
		tknptr = addToToken(tknptr, "zero");

	//Add the space to seperate the token and its type
	*tknptr = ' ';
	tknptr++;

	//Copy the next token to token using tknptr as the current location pointer
	while(*tsptr != ' ' && *tsptr != '\0'){
		*tknptr = *tsptr;
		tknptr++;
		tsptr++;
	}

	//If this was not the last token, move up the pointer so it is located
	//at the beginning of the next token
	if(*tsptr != '\0')
		tsptr++;

	//Moves the pointer for type to the next location (unless this is the last token)
	if(*tsptr != '\0')
		typeptr++;

	//Places null byte char at the end of the output string - now its finished
	*tknptr = '\0';

	//Corrects values for the current token in tokenizer struct
	(*tk).tsptr = tsptr;
	(*tk).typeptr = typeptr;

	return token;
}

/*
 * main will have a string argument (in argv[1]).
 * The string argument contains the tokens.
 * Print out the tokens in the second string in left-to-right order.
 * Each token should be printed on a separate line.
 */

int main(int argc, char **argv) {
	//Check to see if a command line argument exists
	if(argc < 2){
		printf("No Command Line Argument given. Exiting tokenizer.c...\n");
		return 1;
	} else if( argc > 2)
		printf("More than one argument given. Using only first argument...\n");

	//Create tokenizer object using first argument only
	TokenizerT *tokener;
	tokener = TKCreate(*(argv+1));

	printf("\n");

	//Print out all tokens
	while(0 == 0){
		char * token = TKGetNextToken(tokener);
		if(token == NULL)
			break;
		printf("%s\n",token);
		free(token);
	}

	//Free up space allocated by TKCreate
	TKDestroy(tokener);

	printf("\n");

	return 0;
}
