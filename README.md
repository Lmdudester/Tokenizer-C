# Tokenizer-C
## Description
        This program takes in a string as a command line argument and prints out the valid numerical tokens found 
    in the string along with their respective C types. Then it passes the string to TKCreate which reads 
    through the string character by character according to the finite state machine. As is reads the string 
    it places each valid character into a separate string with only valid tokens separated by one ‘ ‘ 
    character each and it determines what type of number the token is. Tokens can be ended by either white 
    space or an escape character. Escape characters are printed out as they are found while creating the 
    tokenstream. If it is found that the token is invalid, meaning the token could not be ended where an 
    escape character or a space was found, then the invalid token is printed out as escape characters in left 
    to right order. Then the pointer returns to the front of that token and overwrites it with the next one.
    As the program accesses each state the type of the current token is changed to reflect what the token 
    would be if it ended at that step. TKCreate returns the TokenizerT struct containing the finalized 
    tokenstream and array of ints representative of the type of each token in the string.  
    
        This struct is then passed to TKGetNextToken which reads through the tokenstream and creates a 
    malloced string of the type (as a string not as an int) and token for the next token in the tokenstream. 
    Two separate pointers are kept in the TokenizerT struct to track where the next token is in the 
    tokenstream and which position in the array of ints corresponds to the next token. The function returns 
    null if no tokens remain. The main method frees each string after use. This is repeated until every token 
    has been returned and printed out by the main.
    
    TKDestroy is then called to free all of the space malloced by TKCreate. It does and the program ends.
## Features:
    - Reallocs the finalized tokenstream and type array at the end of 
        TKCreate to ensure that dynamic memory usage is kept to a minimum
    - Original pointer to beginning of finalized tokenstream and int array is kept 
        so if you wanted to get all tokens in order again, before calling TKDestroy you could  
    - Stores types as integers to save space used until you get the token back 
        with TKGetNextToken
