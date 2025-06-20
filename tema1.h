/* POPESCU Florin-Cosmin 313CD*/
#include <stdio.h>
#include <stdlib.h>

typedef struct page {
    int id;
    char url[50];
    char *description;
} page, *pagina;

typedef struct stackNode {
    struct page *pagina;
    struct stackNode *next;
} stackNode, *stackNod;

typedef struct stack {
    struct stackNode *top;
    int size;
} stack, *stackList;

typedef struct tab {
    int id;
    struct page *currentPage;
    struct stack *backwardStack;
    struct stack *forwardStack;
} tab, *TabList;

typedef struct tabsList {
    struct tab *data;
    struct tabsList *next;
    struct tabsList *prev;
} tabsList, *TabNodeList;

typedef struct browser {
    struct tab *current;
    TabNodeList list;     // lista de taburi (santinela este stocată aici)
} browser, *BrowserList;