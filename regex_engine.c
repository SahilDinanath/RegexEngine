#include "regex_engine.h"
#include <stdio.h>
#include <stdlib.h>
#define SPLIT 256
#define MATCHSTATE 257
typedef struct State {
  int c;
  struct State *out1;
  struct State *out2;
  int last_state;

} State;

typedef struct Statelist {
  State **set;
  int n;
} StateList;

typedef struct Ptrlist {
  int size;
  State ***list;
} Ptrlist;

typedef struct Fragment {
  State *start;
  Ptrlist *out;
} Fragment;

typedef struct List {
  int size;
  State **states;
} List;

State *arr1[1000];
State *arr2[1000];
List l1 = {.size = 0, .states = arr1};
List l2 = {.size = 0, .states = arr2};
int listId = 0;

State *startOfNfa;

Ptrlist *list1(State **outp) {
  Ptrlist *ptr = (Ptrlist *)malloc(sizeof(Ptrlist));
  ptr->size = 1;
  ptr->list = (State ***)malloc(sizeof(State **));
  ptr->list[0] = outp;

  return ptr;
}

Ptrlist *append(Ptrlist *l1, Ptrlist *l2) {
  int total = l1->size + l2->size;
  Ptrlist *ptr = (Ptrlist *)malloc(sizeof(Ptrlist));
  State ***list = (State ***)malloc(sizeof(State **) * total);

  for (int i = 0; i < l1->size; i++) {
    list[i] = l1->list[i];
  }

  for (int i = 0; i < l2->size; i++) {
    list[i + l1->size] = l2->list[i];
  }

  ptr->size = total;
  ptr->list = list;

  return ptr;
}

void patch(Ptrlist *l, State *s) {
  for (int i = 0; i < l->size; i++) {
    **(l->list + i) = s;
  }
  l->size = 0;
}

State *createState(int p, State *o1, State *o2) {
  State *state = (State *)malloc(sizeof(State));
  state->c = p;
  state->out1 = o1;
  state->out2 = o2;
  return state;
}
Fragment createFragment(State *s, Ptrlist *o) {
  Fragment frag;
  frag.start = s;
  frag.out = o;
  return frag;
}
State *postfixToNfa(char *postfix) {
  char *p;
  Fragment stack[1000], *stackp, e, e1, e2;
  stackp = stack;
  State *state;

  for (p = postfix; *p != '\0'; p++) {
    switch (*p) {
    default:
      state = createState(*p, NULL, NULL);
      *stackp++ = createFragment(state, list1(&state->out1));
      break;
    case '.':
      e2 = *--stackp;
      e1 = *--stackp;
      patch(e1.out, e2.start);
      *stackp++ = createFragment(e1.start, e2.out);
      break;
    case '|':
      e2 = *--stackp;
      e1 = *--stackp;
      state = createState(SPLIT, e1.start, e2.start);
      *stackp++ = createFragment(state, append(e1.out, e2.out));
      break;
    case '?':
      e = *--stackp;
      state = createState(SPLIT, e.start, NULL);
      *stackp++ = createFragment(state, append(e.out, list1(&state->out2)));
      break;
    case '*':
      e = *--stackp;
      state = createState(SPLIT, e.start, NULL);
      patch(e.out, state);
      *stackp++ = createFragment(state, list1(&state->out2));
      break;
    case '+':
      e = *--stackp;
      state = createState(SPLIT, e.start, NULL);
      patch(e.out, state);
      *stackp++ = createFragment(e.start, list1(&state->out2));
      break;
    }
  }
  e = *--stackp;
  patch(e.out, createState(MATCHSTATE, NULL, NULL));
  return e.start;
}

// Shunting Yard Algorithm by Edsger Djikstra
// https://mathcenter.oxford.emory.edu/site/cs171/shuntingYardAlgorithm/
int precedence(char c) {
  if (c == '*' || c == '+' || c == '?') {
    return 3;
  } else if (c == '.') {
    return 2;
  } else if (c == '|') {
    return 1;
  }
  return 0;
}
// NOTE: Right associative operators have higher precedence
int associativity(char c) {
  if (c == '*' || c == '+' || c == '?') {
    return 1;
  }
  return 0;
}
char *infixToPostfix(char *infix) {
  char *post = (char *)malloc(sizeof(char) * 1000), *postp;
  postp = post;

  char stack[1000], *stackp;
  stackp = stack;
  // Scan infix from left to right
  for (char *t = infix; *t != '\0'; t++) {
    if ((*t >= 'a' && *t <= 'z') || (*t >= 'A' && *t <= 'Z') ||
        (*t >= '0' && *t <= '9')) {
      *postp++ = *t;
    } else if (*t == '(') {
      *stackp++ = *t;
    } else if (*t == ')') {
      while (*--stackp != '(') {
        *postp++ = *stackp;
      }
    } else {
      char *top = stackp - sizeof(char);

      while (stackp - stack >= 0 && precedence(*t) < precedence(*top) ||
             precedence(*t) == precedence(*top) && associativity(*t) == 0) {
        *postp++ = *top--;
        stackp--;
      }
      *stackp++ = *t;
    }
  }

  while (stackp - stack >= 0) {
    *postp++ = *--stackp;
  }
  *postp++ = '\0';
  return post;
}

void addState(List *l, State *s) {
  if (s == NULL || s->last_state == listId) {
    return;
  }
  if (s->c == SPLIT) {
    // Follow unlabeled arrow.
    addState(l, s->out1);
    addState(l, s->out2);
  }
  l->states[l->size++] = s;
}

List *startList(State *s, List *l) {
  listId++;
  l->size = 0;
  addState(l, s);
  return l;
}
int isMatch(List *l) {
  for (int i = 0; i < l->size; i++) {
    if (l->states[i]->c == MATCHSTATE) {
      return 1;
    };
  }

  return 0;
}
void step(List *clist, int c, List *nlist) {
  State *s;

  listId++;
  nlist->size = 0;
  for (int i = 0; i < clist->size; i++) {
    s = clist->states[i];
    if (s->c == c) {
      addState(nlist, s->out1);
    }
  }
}
int match(char *s) {
  List *clist, *nlist, *t;

  clist = startList(startOfNfa, &l1);
  nlist = &l2;

  for (; *s != '\0'; s++) {
    step(clist, *s, nlist);

    t = clist;
    clist = nlist;
    nlist = t;
  }
  return isMatch(clist);
}

void compileRegex(char *regex) {
  char *postfix = infixToPostfix(regex);
  startOfNfa = postfixToNfa(postfix);
}
