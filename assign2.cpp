/*
 * Names and IDs:
 * Student 1: Ahmed Mohamed Alhabibi  ID: 20226007
 * Student 2: Mariam Ahmed Nassar   ID: 20226128
 * Student 3: Sarah Mohamed Gamal  ID: 20226045
 * Student 4: Ahmed Sameh  ID: 20226002
 *
 * Extended BNF Grammar (James Hein, pp. 459-460):
 *   <expression> ::= <term> { "." <term> }
 *   <term>        ::= <factor> [ "^-1" ]
 *   <factor>      ::= letter | "(" <expression> ")"
 */

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
using namespace std;

#define MAX_CH 2
#define MAX_LB 16
#define MAX_IN 512

struct Node
{
    Node *child[MAX_CH];
    char label[MAX_LB];
    Node()
    {
        child[0] = 0;
        child[1] = 0;
        label[0] = 0;
    }
};

char g_input[MAX_IN];
int g_pos;

char peekChar() { return g_input[g_pos]; }
void consumeChar() { g_pos++; }

Node *parseExpression();

Node *parseFactor()
{
    char ch = peekChar();
    if (ch == '(')
    {
        consumeChar();
        Node *t = parseExpression();
        consumeChar();
        return t;
    }
    Node *t = new Node;
    t->label[0] = ch;
    t->label[1] = 0;
    consumeChar();
    return t;
}

Node *parseTerm()
{
    Node *t = parseFactor();

    while (g_input[g_pos] == '^' &&
           g_input[g_pos + 1] == '-' &&
           g_input[g_pos + 2] == '1')
    {
        consumeChar(); // ^
        consumeChar(); // -
        consumeChar(); // 1

        Node *inv = new Node;
        strcpy(inv->label, "inverse");
        inv->child[0] = t;
        t = inv;  //wraps around tree again
    }

    return t;
}

Node *parseExpression()
{
    Node *t = parseTerm();
    while (peekChar() == '.')
    {
        consumeChar();
        Node *r = parseTerm();
        Node *p = new Node;
        strcpy(p->label, "product");
        p->child[0] = t;
        p->child[1] = r;
        t = p;
    }
    return t;
}

void printTree(Node *n, int d)
{
    if (d > 0)
    {
        int i;
        for (i = 0; i < (d - 1) * 3; i++)
            printf(" ");
        printf("|--");
    }
    printf("%s\n", n->label);
    if (n->child[0]) printTree(n->child[0], d + 1);
    if (n->child[1]) printTree(n->child[1], d + 1);
}

void destroyTree(Node *n)
{
    if (n->child[0]) destroyTree(n->child[0]);
    if (n->child[1]) destroyTree(n->child[1]);
    delete n;
}

Node *copyTree(Node *n)
{
    if (n == 0) return 0;
    Node *c = new Node;
    strcpy(c->label, n->label);
    c->child[0] = copyTree(n->child[0]);
    c->child[1] = copyTree(n->child[1]);
    return c;
}

//is a variable like a,z,...
int isVar(Node *n)
{
    if (n == 0) return 0;
    char c = n->label[0];
    return n->label[1] == 0 &&
        ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

//is identity "e"
int isIdent(Node *n)
{
    return n != 0 && n->label[0] == 'e' && n->label[1] == 0;
}

//is inverse
int isInv(Node *n)
{
    return n != 0 && strcmp(n->label, "inverse") == 0;
}

int isProd(Node *n)
{
    return n != 0 && strcmp(n->label, "product") == 0;
}

//build product node shortcut
Node *mkProd(Node *l, Node *r)
{
    Node *p = new Node;
    strcpy(p->label, "product");
    p->child[0] = l;
    p->child[1] = r;
    return p;
}

//build inverse node shortcut
Node *mkInv(Node *c)
{
    Node *inv = new Node;
    strcpy(inv->label, "inverse");
    inv->child[0] = c;
    return inv;
}

//are the trees equal? recursive check of labels and structure
int treesEq(Node *a, Node *b)
{
    if (a == 0 && b == 0) return 1;
    if (a == 0 || b == 0) return 0;
    return strcmp(a->label, b->label) == 0 &&
        treesEq(a->child[0], b->child[0]) &&
        treesEq(a->child[1], b->child[1]);
}

//turns tree into expression string with parentheses if needed
void toExpr(Node *n, char *buf, int *pos)
{
    if (n == 0) return;
    if (isInv(n)) //for inverse, print child with parentheses if it's a product, then add ^-1
    {
        Node *ch = n->child[0];
        if (isProd(ch)) buf[(*pos)++] = '(';
        toExpr(ch, buf, pos);
        if (isProd(ch)) buf[(*pos)++] = ')';
        buf[(*pos)++] = '^';
        buf[(*pos)++] = '-';
        buf[(*pos)++] = '1';
        return;
    }
    if (isProd(n)) //for product, print left child, then dot, then right child with parentheses if it's a product
    {
        toExpr(n->child[0], buf, pos);
        buf[(*pos)++] = '.';
        if (isProd(n->child[1])) buf[(*pos)++] = '(';
        toExpr(n->child[1], buf, pos);
        if (isProd(n->child[1])) buf[(*pos)++] = ')';
        return;
    }
    int i = 0;
    while (n->label[i]) buf[(*pos)++] = n->label[i++]; // for variable or identity, just copy label
}

void printExpr(Node *n)
{
    char buf[MAX_IN * 4];
    int pos = 0;
    toExpr(n, buf, &pos);
    buf[pos] = 0;
    printf("%s\n", buf);
}

/* promote: copy fields of src into dst in-place, free src shell */
void promote(Node *dst, Node *src)
{
    strcpy(dst->label, src->label);
    dst->child[0] = src->child[0];
    dst->child[1] = src->child[1];
    src->child[0] = 0;
    src->child[1] = 0;
    delete src;
}

int applyOnce(Node *n)
{
    if (n == 0) return 0;

    /* R5: e^-1 -> e */
    if (isInv(n) && isIdent(n->child[0]))
    {
        destroyTree(n->child[0]);
        n->child[0] = 0;
        strcpy(n->label, "e");
        return 1;
    }

    /* R6: (x^-1)^-1 -> x */
    if (isInv(n) && isInv(n->child[0]))
    {
        Node *inner = n->child[0]->child[0];
        n->child[0]->child[0] = 0;
        delete n->child[0];
        n->child[0] = 0;
        promote(n, inner);
        return 1;
    }

    /* R10: (x.y)^-1 -> y^-1.x^-1 */
    if (isInv(n) && isProd(n->child[0]))
    {
        Node *prod = n->child[0];
        Node *x = prod->child[0];
        Node *y = prod->child[1];
        prod->child[0] = 0;
        prod->child[1] = 0;
        n->child[0] = 0;
        delete prod;
        strcpy(n->label, "product");
        n->child[0] = mkInv(y);
        n->child[1] = mkInv(x);
        return 1;
    }

    if (isProd(n))
    {
        Node *L = n->child[0];
        Node *R = n->child[1];

        /* R1: e.x -> x */
        if (isIdent(L))
        {
            n->child[1] = 0;
            destroyTree(L);
            n->child[0] = 0;
            promote(n, R);
            return 1;
        }

        /* R2: x.e -> x */
        if (isIdent(R))
        {
            n->child[0] = 0;
            destroyTree(R);
            n->child[1] = 0;
            promote(n, L);
            return 1;
        }

        /* R3: x^-1.x -> e */
        if (isInv(L) && treesEq(L->child[0], R))
        {
            destroyTree(L);
            destroyTree(R);
            n->child[0] = 0;
            n->child[1] = 0;
            strcpy(n->label, "e");
            return 1;
        }

        /* R4: x.x^-1 -> e */
        if (isInv(R) && treesEq(L, R->child[0]))
        {
            destroyTree(L);
            destroyTree(R);
            n->child[0] = 0;
            n->child[1] = 0;
            strcpy(n->label, "e");
            return 1;
        }

        /* R7: y^-1.(y.z) -> z */
        if (isInv(L) && isProd(R) && treesEq(L->child[0], R->child[0]))
        {
            Node *z = R->child[1];
            R->child[1] = 0;
            destroyTree(L);
            n->child[0] = 0;
            destroyTree(R->child[0]);
            R->child[0] = 0;
            delete R;
            n->child[1] = 0;
            promote(n, z);
            return 1;
        }

        /* R8: y.(y^-1.z) -> z */
        if (isProd(R) && isInv(R->child[0]) &&
            treesEq(L, R->child[0]->child[0]))
        {
            Node *z = R->child[1];
            R->child[1] = 0;
            destroyTree(L);
            n->child[0] = 0;
            destroyTree(R->child[0]);
            R->child[0] = 0;
            delete R;
            n->child[1] = 0;
            promote(n, z);
            return 1;
        }

        // R9: (x.y).z -> x.(y.z)  NOT the other way round
        if (isProd(L)) //left side is the product (x . y) . z
            {
                Node *x = L->child[0];
                Node *y = L->child[1];
                Node *z = R;

                L->child[0] = 0;
                L->child[1] = 0;
                n->child[0] = 0;
                n->child[1] = 0;

                delete L; // the old product node is no longer needed

                n->child[0] = x;
                n->child[1] = mkProd(y, z);

                return 1;
            }
     }

    return 0;

}

int applyRules(Node *n)
{
    if (n == 0) return 0;
   int fired = applyRules(n->child[0]) | applyRules(n->child[1]);
    return applyOnce(n) | fired;
}

void normalize(Node *root, const char *expr)
{
    printf("Input: %s\n", expr);
    printf("Initial parse tree:\n");
    printTree(root, 0);
    printf("\n");
    int step = 0;
    int changed = 1;
    while (changed)
    {
        changed = applyRules(root);
        if (changed)
        {
            step++;
            printf("After step %d:\n", step);
            printTree(root, 0);
            printExpr(root);
            printf("\n");
        }
    }
    printf("Normal form:\n");
    printTree(root, 0);
    printf("Expression: ");
    printExpr(root);
    printf("\n");
}

int isValidInput(const char *s)
{
    int i = 0;
    while (s[i])
    {
        char c = s[i];
        /* allowed: letters, e, dot, ^-1 sequence, parentheses */
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
            { i++; continue; }
        if (c == '(')
            { i++; continue; }
        if (c == ')')
            { i++; continue; }
        if (c == '.')
        {
            if (s[i+1] == '.' || s[i+1] == 0)  // double dot or trailing dot
                { printf("Error: bad use of '.'\n"); return 0; }
            i++; continue;
        }
        if (c == '^')
        {
            if (s[i+1] == '-' && s[i+2] == '1')
                { i += 3; continue; }
            else
                { printf("Error: bad use of '^'\n"); return 0; }
        }
        printf("Error: invalid character '%c'\n", c);
        return 0;
    }
    return 1;
}

int main()
{
    const char *tests[] =
        {
            /* 1  R10+R6      */ "((x.y^-1).z)^-1",
            /* 2  R9          */ "(z^-1.y).x^-1",
            /* 3  R1          */ "e.x",
            /* 4  R2          */ "x.e",
            /* 5  R3          */ "x^-1.x",
            /* 6  R4          */ "x.x^-1",
            /* 7  R5          */ "e^-1",
            /* 8  R6          */ "(x^-1)^-1",
            /* 9  R7          */ "y^-1.(y.z)",
            /* 10 R8          */ "y.(y^-1.z)",
            /* 11 R9          */ "(x.y).z",
            /* 12 R10         */ "(x.y)^-1",
            /* 13 R10+R9      */ "((x.y).z)^-1",
            /* 14 R10+R6      */ "(x^-1.y^-1)^-1",
            /* 15 R1+R3       */ "e.(x^-1.x)",
            /* 16 R10+R3+R4   */ "(x.y)^-1.(y.x)",
            /* 17 R10+R6+R9   */ "((x.y)^-1.z)^-1",
            /* 18 R10+R6+R9   */ "(x.(y.z^-1)^-1)^-1",
            /* 19 R10+R6+R9   */ "((a.b)^-1.(c.d)^-1)^-1",
            /* 20 R7+R8 chain */ "x.(x^-1.(x.(x^-1.y)))",
            /* 21 R5+R2       */ "(e.x^-1.e)^-1",
            /* 22 R10+R6      */ "((x.y^-1).z)^-1",
            /* 23 R6+R2       */ "((x^-1)^-1).e",
            /* 24 R7+R2       */ "y^-1.(y.(z.e))"
        }; 

   int i;
   printf("PROGRAM STARTED\n");
    printf("=== Assignment 2: Group Expression Normalizer ===\n\n");
    for (i = 0; i < 24; i++)
    {
        printf("--- Test %d ---\n", i + 1);
        strcpy(g_input, tests[i]);
        g_pos = 0;
        Node *tree = parseExpression();
        normalize(tree, tests[i]);
        destroyTree(tree);
        printf("==========================================\n\n");
    } 

    printf("=== Interactive Mode (enter 'q' to quit) ===\n\n");
    while (1)
    {
        printf("Expression: ");
        scanf("%s", g_input);
        if (g_input[0] == 'q' && g_input[1] == 0)
            break;
        if (!isValidInput(g_input))  
        {
            printf("Try again.\n\n");
            continue;
        }
        g_pos = 0;
        Node *tree = parseExpression();
        normalize(tree, g_input);
        destroyTree(tree);
    }

    return 0;
}