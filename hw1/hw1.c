#include <stdio.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

typedef enum {
    S_STRT,
    S_LPAR,
    S_RPAR,
    S_MUL,
    S_DIV,
    S_PLUS,
    S_SUB,
    S_INT,
    S_VAR1,
    S_EQ,
    S_SMCL,
    S_DEAD,
    S_VAR2
} State;

//the index of a token always matches a state:
//tokens[0] is an empty string because S_STRT doesn't recognize any token,
//tokens[i] is LPAR because S_LPAR recognizes the "(", and so on
char tokens[13][9] = {
    "",
    "LPAR",
    "RPAR",
    "MUL",
    "DIV",
    "PLUS",
    "SUB",
    "INT",
    "VAR",
    "EQ",
    "SEMICOLON",
    "",
    ""
};

void printTokens(State old, State new, char c);
int closeTokenBracket = FALSE;

int main(int argc, char** argv)
{
    //the input string comes from the standard input and is limited to 1024 characters,
    //though the parser should theoretically work for any size
    printf("Input: ");
    char* inputString = (char*)calloc(1024, sizeof(char));
    fgets(inputString, 1024, stdin);

    //table with 128 entries, so that it can encompass all characters in unextended ASCII
    //the meaning is table[currentChar][currentState] = nextState
    State** table = (State**)calloc(128, sizeof(State*));

    //some rows will be the same for different characters (ex. 1, 2, 3, etc), so we can
    //initialize only a single one of each and then pass the pointer to the different characters's entries.
    //this means that there are only really 12 rows in memory instead of 128

    //current states:    S_STRT  S_LPAR S_RPAR S_MUL  S_DIV  S_PLUS S_SUB  S_INT  S_VAR1 S_EQ   S_SMCL S_DEAD S_VAR2
    State skip[13] =    {S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,S_DEAD,S_STRT};
    State lpar[13] =    {S_LPAR,S_LPAR,S_LPAR,S_LPAR,S_LPAR,S_LPAR,S_LPAR,S_LPAR,S_LPAR,S_LPAR,S_LPAR,S_DEAD,S_LPAR};
    State rpar[13] =    {S_RPAR,S_RPAR,S_RPAR,S_RPAR,S_RPAR,S_RPAR,S_RPAR,S_RPAR,S_RPAR,S_RPAR,S_RPAR,S_DEAD,S_RPAR};
    State mul[13] =     {S_MUL, S_MUL, S_MUL, S_MUL, S_MUL, S_MUL, S_MUL, S_MUL, S_MUL, S_MUL, S_MUL, S_DEAD, S_MUL};
    State divs[13] =    {S_DIV, S_DIV, S_DIV, S_DIV, S_DIV, S_DIV, S_DIV, S_DIV, S_DIV, S_DIV, S_DIV, S_DEAD, S_DIV};
    State plus[13] =    {S_PLUS,S_PLUS,S_PLUS,S_PLUS,S_PLUS,S_PLUS,S_PLUS,S_PLUS,S_PLUS,S_PLUS,S_PLUS,S_DEAD,S_PLUS};
    State sub[13] =     {S_SUB, S_SUB, S_SUB, S_SUB, S_SUB, S_SUB, S_SUB, S_SUB, S_SUB, S_SUB, S_SUB ,S_DEAD, S_SUB};
    State digit[13] =   {S_INT, S_INT, S_INT, S_INT, S_INT, S_INT, S_INT, S_INT, S_VAR2, S_INT, S_INT, S_DEAD,S_VAR2};
    State letter[13] =  {S_VAR1,S_VAR1,S_VAR1,S_VAR1,S_VAR1,S_VAR1,S_VAR1,S_DEAD,S_VAR2,S_VAR1,S_VAR1,S_DEAD,S_VAR2};
    State eq[13] =      {S_EQ,  S_EQ,  S_EQ,  S_EQ,  S_EQ,  S_EQ,  S_EQ,  S_EQ,  S_EQ,  S_EQ,  S_EQ,  S_DEAD,  S_EQ};
    State smcl[13] =    {S_SMCL,S_SMCL,S_SMCL,S_SMCL,S_SMCL,S_SMCL,S_SMCL,S_SMCL,S_SMCL,S_SMCL,S_SMCL,S_DEAD,S_SMCL};
    State others[13] =  {S_DEAD,S_DEAD,S_DEAD,S_DEAD,S_DEAD,S_DEAD,S_DEAD,S_DEAD,S_DEAD,S_DEAD,S_DEAD,S_DEAD,S_DEAD};

    table[' '] = skip;
    table['\n'] = skip;
    table['\r'] = skip;
    table['\t'] = skip;
    table['('] = lpar;
    table[')'] = rpar;
    table['*'] = mul;
    table['\\'] = divs;
    table['+'] = plus;
    table['-'] = sub;
    table[';'] = smcl;
    table['='] = eq;

    int i;
    for (i = '0'; i <= '9'; i++) table[i] = digit;
    for (i = 'a'; i <= 'z'; i++) table[i] = letter;
    for (i = 'A'; i <= 'Z'; i++) table[i] = letter;
    for (i = 0; i < 128; i++)
    {   //fill in all the remaining characters entries with a dead state row
        if (table[i] == NULL)
            table[i] = others;
    }

    State currentState = S_STRT;
    int index = 0;
    char c = inputString[index];

    printf("\n");
    while (c != '\0')
    {
        State newState = table[(int)c][currentState];
        printTokens(currentState, newState, c);
        currentState = newState;
        index++;
        c = inputString[index];
    }
    printf("%s", closeTokenBracket ? ")" : "");
    printf("\n\n%s\n", currentState == S_DEAD ? "Invalid string" : "Valid string");

    return 0;
}

//the tokens to be printed are decided by the transition
//between an old and a new state caused by a character,
//and extra work needs to be done for the situations
//in which the content matters (INT and VAR)
void printTokens(State old, State new, char c)
{
    if (new == S_VAR1 && old != S_VAR1)
    {
        closeTokenBracket = TRUE;
        printf("VAR(%c", c);
        return;
    }
    if (new == S_VAR2)
    {
        printf("%c", c);
        return;
    }
    if (old == S_VAR2 && new != S_VAR2)
    {
        closeTokenBracket = FALSE;
        printf(") ");
    }
    if (new == S_INT && old != S_INT)
    {
        closeTokenBracket = TRUE;
        printf("INT(%c", c);
        return;
    }
    if (old == S_INT && new == S_INT)
    {
        printf("%c", c);
        return;
    }
    if (old == S_INT && new != S_INT)
    {
        closeTokenBracket = FALSE;
        printf(") ");
    }
    if (new != S_INT && new != S_VAR1 && new != S_VAR2 &&
        new != S_DEAD && new != S_STRT)
        printf("%s ", tokens[new]);
}
