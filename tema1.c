/* POPESCU Florin-Cosmin 313CD*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tema1.h"

stackList createStack() {
    stackList newStack = (stackList)malloc(sizeof(stack));
    if(newStack == NULL)
        return NULL;
    newStack->top = NULL;
    newStack->size = 0;
    return newStack;
}

int emptyStack(stackList stack) {
    return (stack == NULL || stack->size == 0);
}

void push(stackList stack, pagina page) {
    if(stack == NULL)
        return;
    stackNod newNode = (stackNod)malloc(sizeof(stackNode));
    if(newNode == NULL)
        return;
    newNode->pagina = page;
    newNode->next = stack->top;
    stack->top = newNode;
    stack->size++;
}

pagina pop(stackList stack) {
    if(emptyStack(stack))
        return NULL;
    stackNod temp = stack->top;
    pagina page = temp->pagina;
    stack->top = temp->next;
    free(temp);
    stack->size--;
    return page;
}

void clearStack(stackList stack) {
    while(!emptyStack(stack)) {
        pagina page = pop(stack);
        if(page) {
            free(page->description);
            free(page);
        }
    }
}

void freeStack(stackList stack) {
    clearStack(stack);
    free(stack);
}

pagina createPage(int id, char *url, char *description) {
    pagina newPage = (pagina)malloc(sizeof(page));
    if(newPage == NULL)
        return NULL;
    newPage->id = id;
    strncpy(newPage->url, url, 50);
    newPage->url[49] = '\0';
    newPage->description = (char *)malloc(strlen(description) + 1);
    if(newPage->description == NULL) {
        free(newPage);
        return NULL;
    }
    strcpy(newPage->description, description);
    return newPage;
}

pagina DefaultPage() {
    return createPage(0, "https://acs.pub.ro/", "Computer Science\n");
}

TabList createTab(int id) {
    TabList newTab = (TabList)malloc(sizeof(tab));
    if(newTab == NULL)
        return NULL;
    newTab->currentPage = DefaultPage();
    newTab->backwardStack = createStack();
    newTab->forwardStack = createStack();
    newTab->id = id;
    return newTab;
}

void freeTab(TabList tab) {
    if(tab) {
        if(tab->currentPage) {
            free(tab->currentPage->description);
            free(tab->currentPage);
        }
        if(tab->backwardStack)
            freeStack(tab->backwardStack);
        if(tab->forwardStack)
            freeStack(tab->forwardStack);
        free(tab);
    }
}

TabNodeList createTabsList() {
    TabNodeList sentinel = (TabNodeList)malloc(sizeof(tabsList));
    if(sentinel == NULL)
        return NULL;
    sentinel->data = NULL;
    sentinel->next = sentinel;
    sentinel->prev = sentinel;
    return sentinel;
}

BrowserList initBrowser() {
    BrowserList browser = (BrowserList)malloc(sizeof(struct browser));
    if(browser == NULL)
        return NULL;
    browser->list = createTabsList();
    TabList initTab = createTab(0);
    TabNodeList node = (TabNodeList)malloc(sizeof(tabsList));
    node->data = initTab;
    // Inserare imediat după santinelă
    TabNodeList sentinel = browser->list;
    node->next = sentinel->next; // inițial, sentinel->next == sentinel
    node->prev = sentinel;
    sentinel->next->prev = node;
    sentinel->next = node;
    browser->current = initTab;
    return browser;
}

TabNodeList findTabNode(BrowserList browser, int id) {
    if(browser == NULL || browser->list == NULL)
        return NULL;
    TabNodeList sentinel = browser->list;
    TabNodeList curr = sentinel->next;
    while(curr != sentinel) {
        if(curr->data && curr->data->id == id)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

void addTab(BrowserList browser, TabList newTab) {
    if(browser == NULL || browser->list == NULL || newTab == NULL)
        return;
    TabNodeList sentinel = browser->list;
    TabNodeList node = (TabNodeList)malloc(sizeof(tabsList));
    if(node == NULL)
        return;
    node->data = newTab;
    // Inserare înaintea santenlei
    node->next = sentinel;
    node->prev = sentinel->prev;
    sentinel->prev->next = node;
    sentinel->prev = node;
}

void removeTab(BrowserList browser, TabNodeList node) {
    if(browser == NULL || browser->list == NULL || node == NULL || node == browser->list)
        return; // nu se elimină santenela
    node->prev->next = node->next;
    node->next->prev = node->prev;
    freeTab(node->data);
    free(node);
}

void freeBrowser(BrowserList browser) {
    if(browser == NULL)
        return;
    if(browser->list) {
        TabNodeList sentinel = browser->list;
        TabNodeList curr = sentinel->next;
        while(curr != sentinel) {
            TabNodeList temp = curr->next;
            freeTab(curr->data);
            free(curr);
            curr = temp;
        }
        free(sentinel);
    }
    free(browser);
}


void processNewTab(BrowserList browser, int *lastTabId) {
    int newId = *lastTabId + 1;
    TabList newTab = createTab(newId);
    if(newTab == NULL)
        return;
    addTab(browser, newTab);
    browser->current = newTab;
    *lastTabId = newId;
}

/* Se închide tabul curent. Nu se poate închide tabul cu id 0 
dacă se încearcă închiderea tabului cu ID 0 se afișează mesajul de eroare "403 Forbidden" */
void processClose(BrowserList browser, FILE *out) {
    if(browser->current->id == 0) {
        fprintf(out, "403 Forbidden\n");
        return;
    }
    TabNodeList currNode = findTabNode(browser, browser->current->id);
    if(currNode == NULL)
        return;
    // Se setează tabul curent ca fiind cel din stânga (prev)
    // dacă prev este santinela, se alege următorul
    TabNodeList newCurrent = currNode->prev;
    if(newCurrent == browser->list)
        newCurrent = currNode->next;
    browser->current = newCurrent->data;
    removeTab(browser, currNode);
}

/* Schimbă tabul curent în cel ce are ID-ul specificat.
   Dacă nu există tab cu acel ID se afișează "403 Forbidden" */
void processOpen(BrowserList browser, int id, FILE *out) {
    TabNodeList node = findTabNode(browser, id);
    if(node == NULL) {
        fprintf(out, "403 Forbidden\n");
        return;
    }
    browser->current = node->data;
}

// schimba tabul curent în următorul din lista circulară
void processNext(BrowserList browser) {
    TabNodeList currNode = findTabNode(browser, browser->current->id);
    if(currNode == NULL)
        return;
    TabNodeList nextNode = currNode->next;
    if(nextNode == browser->list)
        nextNode = nextNode->next;
    browser->current = nextNode->data;
}

// schimba tabul curent în cel precedent din lista circulară
void processPrev(BrowserList browser) {
    TabNodeList currNode = findTabNode(browser, browser->current->id);
    if(currNode == NULL)
        return;
    TabNodeList prevNode = currNode->prev;
    if(prevNode == browser->list)
        prevNode = prevNode->prev;
    browser->current = prevNode->data;
}

/* Se verifică existența paginii cu ID-ul dat (se caută şi în cazul ID-ului 0 se creează pagina implicită)
    Se mută pagina curentă în stiva backward, se golește stiva forward,
    iar noua pagină devine pagina curentă (se creează o copie din memoria din array-ul de pagini)
    Dacă pagina nu există se afișează mesajul de eroare "403 Forbidden" */
void processPage(BrowserList browser, page **pages, int numPages, int pageId, FILE *out) {
    pagina newPage = NULL;
    if(pageId == 0) {
        newPage = DefaultPage();
    } else {
        int found = 0;
        for(int i = 0; i < numPages; i++) {
            if(pages[i] && pages[i]->id == pageId) {
                newPage = createPage(pages[i]->id, pages[i]->url, pages[i]->description);
                found = 1;
                break;
            }
        }
        if(!found) {
            fprintf(out, "403 Forbidden\n");
            return;
        }
    }
    push(browser->current->backwardStack, browser->current->currentPage);
    clearStack(browser->current->forwardStack);
    browser->current->currentPage = newPage;
}

/* Se verifică dacă stiva backward nu este goală .dacă este, se afișează "403 Forbidden"
   Altfel, se mută pagina curentă în stiva forward și se setează ca pagină curentă cea scoasă din stiva backward */
void processBackward(BrowserList browser, FILE *out) {
    if(emptyStack(browser->current->backwardStack)) {
        fprintf(out, "403 Forbidden\n");
        return;
    }
    push(browser->current->forwardStack, browser->current->currentPage);
    pagina prevPage = pop(browser->current->backwardStack);
    browser->current->currentPage = prevPage;
}

/* Se verifică dacă stiva forward nu este goală .dacă este, se afișează "403 Forbidden"
   Altfel, se mută pagina curentă în stiva backward și se setează ca pagină curentă cea scoasă din stiva forward */
void processForward(BrowserList browser, FILE *out) {
    if(emptyStack(browser->current->forwardStack)) {
        fprintf(out, "403 Forbidden\n");
        return;
    }
    push(browser->current->backwardStack, browser->current->currentPage);
    pagina nextPage = pop(browser->current->forwardStack);
    browser->current->currentPage = nextPage;
}

/* Se afișează, pe o singură linie, ID-urile taburilor începând de la tabul curent, parcurgând spre dreapta (lista circulară)
   Pe linia următoare se afișează descrierea paginii curente a tabului curent */
void processPrint(BrowserList browser, FILE *out) {
    TabNodeList currNode = findTabNode(browser, browser->current->id);
    if(currNode == NULL)
        return;
    TabNodeList start = currNode;
    TabNodeList temp = currNode;
    do {
        if(temp->data)
            fprintf(out, "%d ", temp->data->id);
        temp = temp->next;
        if(temp == browser->list)
            temp = temp->next; // sărit de santinelă
    } while(temp != start);
    fprintf(out, "\n");
    if(browser->current->currentPage && browser->current->currentPage->description)
        fprintf(out, "%s", browser->current->currentPage->description);
}

/* Se caută tabul cu ID-ul dat; dacă nu se găsește se afișează "403 Forbidden"
   Se afișează istoricul: mai întâi URL-urile din stiva forward (în ordine de la cel mai vechi)
   apoi URL-ul paginii curente, după care URL-urile din stiva backward (de la cel mai recent pus)
   fiecare URL pe o linie separată.
*/
void processPrintHistory(BrowserList browser, int tabId, FILE *out) {
    TabNodeList node = findTabNode(browser, tabId);
    if(node == NULL) {
        fprintf(out, "403 Forbidden\n");
        return;
    }
    // Pentru stiva forward: deoarece elementele sunt stocate LIFO (ultimul la vârf),
    // colectăm elementele într-un array și le parcurgem de la început (cea mai veche) spre sfârșit.
    stackList forward = node->data->forwardStack;
    int fSize = forward->size;
    if(fSize > 0) {
        pagina *fPages = (pagina *)malloc(fSize * sizeof(pagina));
        if(fPages) {
            int idx = fSize - 1;
            stackNod cur = forward->top;
            while(cur) {
                fPages[idx] = cur->pagina;
                idx--;
                cur = cur->next;
            }
            for(int i = 0; i < fSize; i++) {
                fprintf(out, "%s\n", fPages[i]->url);
            }
            free(fPages);
        }
    }
    // Afișează URL-ul paginii curente
    if(node->data->currentPage)
        fprintf(out, "%s\n", node->data->currentPage->url);
    // Pentru stiva backward parcurgem de la vârf la bază
    stackList backward = node->data->backwardStack;
    stackNod curB = backward->top;
    while(curB) {
        fprintf(out, "%s\n", curB->pagina->url);
        curB = curB->next;
    }
}

int main(void) {
    FILE *in  = fopen("tema1.in",  "r");
    FILE *out = fopen("tema1.out", "w");
    if (in == NULL || out == NULL) {
        if (in) fclose(in);
        if (out) fclose(out);
        return 1;
    }

    int numPages;
    fscanf(in, "%d\n", &numPages);
    
    pagina *pages = malloc(numPages * sizeof(pagina));
    for (int i = 0; i < numPages; i++) {
        char line[5], url[50], desc[256];
        int id;

        fgets(line, sizeof(line), in);
        id = atoi(line);

        fgets(url, sizeof(url), in);

        fgets(desc, sizeof(desc), in);

        pages[i] = createPage(id, url, strcat(desc, "\n"));
    }

    BrowserList browser = initBrowser();
    int lastTabId = 0;

    int numOps;
    fscanf(in, "%d\n", &numOps);
    char line[256];

    for (int i = 0; i < numOps; i++) {
        if (!fgets(line, sizeof(line), in))
            break;

        char cmd[20];
        int arg;
        int n = sscanf(line, "%s %d", cmd, &arg);

        if (strcmp(cmd, "NEW_TAB") == 0) processNewTab(browser, &lastTabId);
        else if (strcmp(cmd, "CLOSE") == 0) processClose(browser, out);
        else if (strcmp(cmd, "OPEN") == 0 && n == 2) processOpen(browser, arg, out);
        else if (strcmp(cmd, "NEXT") == 0) processNext(browser);
        else if (strcmp(cmd, "PREV") == 0) processPrev(browser);
        else if (strcmp(cmd, "PAGE") == 0 && n == 2) processPage(browser, pages, numPages, arg, out);
        else if (strcmp(cmd, "BACKWARD") == 0) processBackward(browser, out);
        else if (strcmp(cmd, "FORWARD") == 0) processForward(browser, out);
        else if (strcmp(cmd, "PRINT") == 0) processPrint(browser, out);
        else if (strcmp(cmd, "PRINT_HISTORY") == 0 && n == 2)
            processPrintHistory(browser, arg, out);
    }

    for (int i = 0; i < numPages; i++) {
        free(pages[i]->description);
        free(pages[i]);
    }
    free(pages);
    freeBrowser(browser);

    fclose(in);
    fclose(out);
    return 0;
}
