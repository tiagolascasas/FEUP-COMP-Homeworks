//Compile with, for example, gcc hw3.c -o hw3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

char** scanTokens(char* input);
void addToken(char** tokens, int* i, State old, State new, char c);
char verifyGrammar(char** tokens);
void prodS(char** tokens);
void prodT(char** tokens);
void prodE(char** tokens);
void prodRhs(char** tokens);
void prodRA(char** tokens);
void prodRB(char** tokens);
void prodRC(char** tokens);
void prodRD(char** tokens);
void prodRPLUS(char** tokens);
void prodRSUB(char** tokens);
void prodRDIV(char** tokens);
void prodRMUL(char** tokens);

int closeTokenBracket = FALSE;

//the index of a token always matches a state:
//tokens[0] is an empty string because S_STRT doesn't recognize any token,
//tokens[i] is LPAR because S_LPAR recognizes the "(", and so on
char tokensList[13][9] = {
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

int main(int argc, char** argv)
{
    char* inputString;
    if (argc == 2)
    {
        FILE* fptr;
        long fileLength;
        fptr = fopen(argv[1], "rb");
        fseek(fptr, 0, SEEK_END);
        fileLength = ftell(fptr);
        rewind(fptr);
        inputString = (char*)calloc(fileLength + 1, sizeof(char));
        fread(inputString, fileLength, 1, fptr);
        fclose(fptr);
    }
    else
    {
        printf("Input: ");
        char* inputString = (char*)calloc(1024, sizeof(char));
        fgets(inputString, 1024, stdin);
    }

    char** tokens = scanTokens(inputString);

    printf("\nLexical analysis: string is %s\n", tokens == NULL ? "invalid" : "valid");
    if (tokens == NULL)
        return -1;


    for (int x = 0; x < 128; x++)
    {
        printf("%s ", tokens[x]);
    }

    char valid = verifyGrammar(tokens);
    printf("\nSyntactic analysis: string is %s\n", valid ? "valid" : "invalid");

    return 0;
}

char** scanTokens(char* inputString)
{
    //table with 128 entries, so that it can encompass all characters in unextended ASCII
    //the meaning is table[currentChar][currentState] = nextState
    State** table = (State**)calloc(128, sizeof(State*));

    char** tokens = (char**)calloc(1024, sizeof(char*));

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
    int tokensIndex = 0;

    while (c != '\0')
    {
        State newState = table[(int)c][currentState];
        addToken(tokens, &tokensIndex, currentState, newState, c);
        currentState = newState;
        index++;
        c = inputString[index];
    }

    return currentState == S_DEAD ? NULL : tokens;
}

void addToken(char** tokens, int* i, State old, State new, char c)
{
    if (new == S_VAR1 && old != S_VAR1)
    {
        closeTokenBracket = TRUE;
        tokens[*i] = (char*)calloc(258, sizeof(char));
        sprintf(tokens[*i], "VAR(%c", c);
        printf(tokens[*i]);printf("\n");
        return;
    }
    if (new == S_VAR2)
    {
        tokens[*i][strlen(tokens[*i])] = c;
        printf(tokens[*i]);printf("\n");
        return;
    }
    if (old == S_VAR2 && new != S_VAR2)
    {
        closeTokenBracket = FALSE;
        tokens[*i][strlen(tokens[*i])] = ')';
        printf(tokens[*i]);printf("\n");
        (*i)++;
    }
    if (new == S_INT && old != S_INT)
    {
        closeTokenBracket = TRUE;
        tokens[*i] = (char*)calloc(258, sizeof(char));
        sprintf(tokens[*i], "INT(%c", c);
        printf(tokens[*i]);printf("\n");
        return;
    }
    if (old == S_INT && new == S_INT)
    {
        tokens[*i][strlen(tokens[*i])] = c;
        printf(tokens[*i]);printf("\n");
        return;
    }
    if (old == S_INT && new != S_INT)
    {
        closeTokenBracket = FALSE;
        tokens[*i][strlen(tokens[*i])] = ')';
        printf(tokens[*i]);printf("\n");
        (*i)++;
    }
    if (new != S_INT && new != S_VAR1 && new != S_VAR2 &&
        new != S_DEAD && new != S_STRT)
    {
        tokens[*i] = tokensList[new];
        printf(tokens[*i]);printf("\n");
        (*i)++;
    }
}

/*
Grammar:
S -> VAR EQ T
T -> INT SMICOLON S | E
E -> Rhs SMICOLON

Rhs -> RA RPLUS     //assure operator priority: +- have less, while () have more
RA -> RB RSUB
RB -> RC RDIV
RC -> RD RMUL
RD -> VAR | LPAR Rhs RPAR

RPLUS -> PLUS Rhs | ε
RSUB -> SUB RA | ε
RDIV -> DIV RB | ε
RMUL -> MUL RC | ε
*/
char isValid = TRUE;
int idx;

char equals(char* str1, char* str2)
{
    int n = strlen(str1);
    return strncmp(str1, str2, n - 1) == 0 ? TRUE : FALSE;
}

char verifyGrammar(char** tokens)
{
    idx = 0;
    prodS(tokens);
    return isValid;
}

void prodS(char** tokens)
{
    if (equals("VAR", tokens[idx]) && equals("EQ", tokens[idx+1]))
    {
        printf("%s EQ ", tokens[idx]);
        idx += 2;
        prodT(tokens);
    }
    else
        isValid = FALSE;
}

void prodT(char** tokens)
{
    if(equals("INT", tokens[idx]) && equals("SMICOLON", tokens[idx+1]))
    {
        printf("%s SMICOLON ", tokens[idx]);
        idx += 2;
        prodS(tokens);
    }
    else
        prodE(tokens);
}

void prodE(char** tokens)
{
    prodRhs(tokens);
    if (equals("SMICOLON", tokens[idx]))
    {
        printf("SMICOLON ");
        idx++;
    }
    else
        isValid = FALSE;
}

void prodRhs(char** tokens)
{
    prodRA(tokens);
    prodRPLUS(tokens);
}

void prodRA(char** tokens)
{
    prodRB(tokens);
    prodRSUB(tokens);
}

void prodRB(char** tokens)
{
    prodRC(tokens);
    prodRDIV(tokens);
}

void prodRC(char** tokens)
{
    prodRD(tokens);
    prodRMUL(tokens);
}

void prodRD(char** tokens)
{
    if (equals("VAR", tokens[idx]))
    {
        printf("%s ", tokens[idx]);
        idx++;
    }
    else if (equals("LPAR", tokens[idx]))
    {
        printf("LPAR ");
        idx++;
        prodRhs(tokens);
        if (equals("RPAR", tokens[idx]))
        {
            printf("RPAR ");
            idx++;
        }
        else
            isValid = FALSE;
    }
    else
        isValid = FALSE;
}

void prodRPLUS(char** tokens)
{
    if (equals("PLUS", tokens[idx]))
    {
        printf("PLUS ");
        idx++;
        prodRhs(tokens);
    }
}

void prodRSUB(char** tokens)
{
    if (equals("SUB", tokens[idx]))
    {
        printf("SUB ");
        idx++;
        prodRhs(tokens);
    }
}

void prodRDIV(char** tokens)
{
    if (equals("DIV", tokens[idx]))
    {
        printf("DIV ");
        idx++;
        prodRhs(tokens);
    }
}

void prodRMUL(char** tokens)
{
    if (equals("MUL", tokens[idx]))
    {
        printf("MUL ");
        idx++;
        prodRhs(tokens);
    }
}
