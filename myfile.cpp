/*
 * Names and IDs:
 * Student 1: Ahmed Mohamed Alhabibi  ID: 20226007
 * Student 2: Mariam Ahmed Nassar   ID: 20226128
 * Student 3: Sarah Mohamed Gamal  ID: 20226045
 * Student 4: Ahmed Sameh  ID: 20226002
 *
 * Extended BNF Grammar for Group Expressions (James Hein, pp. 459-460):
 *
 *   <expression> ::= <term> { "." <term> }
 *   <term>        ::= <factor> [ "^-1" ]
 *   <factor>      ::= letter | "(" <expression> ")"
 *
 * Notes:
 *   - "." represents the group binary product operator (left-associative)
 *   - "^-1" represents the group inverse (postfix unary operator)
 *   - This parser treats every single letter (a-z or A-Z) as a normal leaf symbol
 *   - Parentheses are used for grouping only and do not appear in the tree
 */

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
using namespace std;

/*===========================================================================*/
/* Parse Tree Node                                                            */
/*===========================================================================*/

#define MAX_CHILDREN 2
#define MAX_LABEL 16
#define MAX_INPUT 512

struct Node
{
    Node *child[MAX_CHILDREN];
    char label[MAX_LABEL];

    Node()
    {
        int i;
        for (i = 0; i < MAX_CHILDREN; i++)
            child[i] = 0;
        label[0] = 0;
    }
};

/*===========================================================================*/
/* Global Input State                                                         */
/*===========================================================================*/

char g_input[MAX_INPUT];
int g_pos;

char peekChar()
{
    return g_input[g_pos];
}

void consumeChar()
{
    g_pos++;
}

/*===========================================================================*/
/* Parser (Recursive Descent)                                                 */
/*===========================================================================*/

Node *parseExpression();

/* <factor> ::= letter | "(" <expression> ")" */
Node *parseFactor()
{
    char ch = peekChar();
    if (ch == '(')
    {
        consumeChar();
        Node *tree = parseExpression();
        consumeChar();
        return tree;
    }
    Node *tree = new Node;
    tree->label[0] = ch;
    tree->label[1] = 0; // terminate C-style string
    consumeChar();
    return tree;
}

/* <term> ::= <factor> [ "^-1" ] */
Node *parseTerm()
{
    Node *tree = parseFactor();
    if (peekChar() == '^')
    {
        consumeChar();
        consumeChar();
        consumeChar();
        Node *inv = new Node;
        strcpy(inv->label, "inverse");
        inv->child[0] = tree;
        return inv;
    }
    return tree;
}

/* <expression> ::= <term> { "." <term> } */
Node *parseExpression()
{
    Node *tree = parseTerm();
    while (peekChar() == '.')
    {
        consumeChar();
        Node *right = parseTerm();
        Node *prod = new Node;
        strcpy(prod->label, "product");
        prod->child[0] = tree;
        prod->child[1] = right;
        tree = prod;
    }
    return tree;
}

/*===========================================================================*/
/* Tree Printing                                                              */
/*===========================================================================*/

void printTree(Node *node, int depth)
{
    int i;
    if (depth > 0)
    {
        for (i = 0; i < (depth - 1) * 3; i++)
            printf(" ");
        printf("|--");
    }
    printf("%s\n", node->label);
    for (i = 0; i < MAX_CHILDREN; i++)
    {
        if (node->child[i])
            printTree(node->child[i], depth + 1);
    }
}

/*===========================================================================*/
/* Tree Cleanup                                                               */
/*===========================================================================*/

void destroyTree(Node *node)
{
    int i;
    for (i = 0; i < MAX_CHILDREN; i++)
    {
        if (node->child[i])
            destroyTree(node->child[i]);
    }
    delete node;
}

/*===========================================================================*/
/* Run a Single Test Case                                                     */
/*===========================================================================*/

void runTest(const char *expr)
{
    printf("Input: %s\n", expr);
    strcpy(g_input, expr);
    g_pos = 0;
    Node *tree = parseExpression();
    printTree(tree, 0);
    printf("\n");
    destroyTree(tree);
}

/*===========================================================================*/
/* Main                                                                       */
/*===========================================================================*/

int main()
{
    /*
     * Test Cases (30 total)
     * Cases 1-8:   directly from James Hein pp. 459-460
     * Cases 9-30:  complex nested variations
     */
    const char *tests[30] =
        {
            /* 1 */ "e.x",
            /* 2 */ "x^-1.x",
            /* 3 */ "(x.y).z",
            /* 4 */ "(x.e).(x.y)",
            /* 5 */ "x.x^-1",
            /* 6 */ "y^-1.(y.z)",
            /* 7 */ "(x.y)^-1",
            /* 8 */ "((x.y^-1).z)^-1",
            /* 9 */ "x.y.z",
            /* 10 */ "x^-1.y^-1",
            /* 11 */ "(x.y.z)^-1",
            /* 12 */ "x.(y.z^-1)",
            /* 13 */ "(x^-1.y)^-1",
            /* 14 */ "e.e.e",
            /* 15 */ "(x.y).(z.w)",
            /* 16 */ "((x.y).z).w",
            /* 17 */ "x.(y.(z.w))",
            /* 18 */ "(x.y^-1).(y.z^-1)",
            /* 19 */ "((x.y)^-1.z)^-1",
            /* 20 */ "(x.(y.z^-1)^-1)^-1",
            /* 21 */ "((a.b).c).((d.e).f)",
            /* 22 */ "(x.y.z^-1.w^-1)^-1",
            /* 23 */ "((x^-1)^-1)^-1",
            /* 24 */ "(e.e.x^-1.e).e",
            /* 25 */ "((a.b)^-1.(c.d)^-1)^-1",
            /* 26 */ "x.(x^-1.(x.(x^-1.y)))",
            /* 27 */ "(((x.y).z).w).v^-1",
            /* 28 */ "(x^-1.y.x)^-1",
            /* 29 */ "((x.y)^-1.e.z.e)^-1",
            /* 30 */ "(x.(y.(z.(w.v)^-1)^-1)^-1)^-1"};

    int i;
    printf("=== Predefined Test Cases ===\n\n");
    for (i = 0; i < 30; i++)
    {
        printf("Test %d:\n", i + 1);
        runTest(tests[i]);
    }

    while (true)
    {
        printf("Expression: ");
        scanf("%s", g_input);
        if (g_input[0] == 'q' && g_input[1] == 0)
            break;
        g_pos = 0;
        Node *tree = parseExpression();
        printTree(tree, 0);
        printf("\n");
        destroyTree(tree);
    }

    return 0;
}