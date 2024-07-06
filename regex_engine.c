#include "regex_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SPLIT 256
#define MATCHSTATE 257
// Implementation based on: https://swtch.com/~rsc/regexp/regexp1.html

/*
 * symbol: type of State, 0 - 255 char character, SPLIT or MATCHSTATE
 * out1, out2: pointers to next states
 * last_state: prevents states being added multiple times during simulation
 */
typedef struct State {
  int symbol;
  struct State *out1;
  struct State *out2;
  int last_state;

} State;

// List of States in each step of NFA simulation
typedef struct StateList {
  int size;
  State **states;
} StateList;

// List of Dangling pointers from State
typedef struct PtrList {
  int size;
  State ***list;
} PtrList;

// Used to group together states and perform actions on them as a single object
typedef struct Fragment {
  State *start;
  PtrList *out;
} Fragment;

State *createState(int p, State *o1, State *o2) {
  State *state = malloc(sizeof(State));
  state->symbol = p;
  state->out1 = o1;
  state->out2 = o2;
  return state;
}
Fragment createFragment(State *s, PtrList *o) {
  Fragment frag;
  frag.start = s;
  frag.out = o;
  return frag;
}

// Global variables
State *arr1[1000], *arr2[1000], *startOfNfa;
StateList l1 = {.size = 0, .states = arr1}, l2 = {.size = 0, .states = arr2};
int listId = 0;

// creates initial pointer list
PtrList *list1(State **outp) {
  PtrList *ptr = malloc(sizeof(PtrList));
  ptr->size = 1;
  ptr->list = malloc(sizeof(State **));
  ptr->list[0] = outp;

  return ptr;
}

PtrList *append(PtrList *l1, PtrList *l2) {
  int total = l1->size + l2->size;
  PtrList *ptr = malloc(sizeof(PtrList));
  State ***list = malloc(sizeof(State **) * total);

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

// Assigns all dangling pointers in pointer list to a given state
void patch(PtrList *l, State *s) {
  for (int i = 0; i < l->size; i++) {
    **(l->list + i) = s;
  }
  l->size = 0;
}

// Refer to
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
    case '&':
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
int isAlphaNumeric(char t) {
  if ((t >= 'a' && t <= 'z') || (t >= 'A' && t <= 'Z') ||
      (t >= '0' && t <= '9') || (t == '.')) {
    return 1;
  }
  return 0;
}
int precedence(char c) {
  if (c == '*' || c == '+' || c == '?') {
    return 3;
  } else if (c == '&') {
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
  char *post = malloc(sizeof(char) * 1000), *postp;
  postp = post;

  char stack[1000], *stackp;
  stackp = stack;
  // Scan infix from left to right
  for (char *t = infix; *t != '\0'; t++) {
    if (isAlphaNumeric(*t)) {
      *postp++ = *t;
    } else if (*t == '(') {
      *stackp++ = *t;
    } else if (*t == ')') {
      while (*--stackp != '(') {
        *postp++ = *stackp;
      }
    } else {
      char *top = stackp - sizeof(char);

      while ((stackp - stack >= 0 && precedence(*t) < precedence(*top)) ||
             (precedence(*t) == precedence(*top) && associativity(*t) == 0)) {
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

// Adds states to next list (nlist)
void addState(StateList *l, State *s) {
  if (s == NULL || s->last_state == listId) {
    return;
  }
  if (s->symbol == SPLIT) {
    // Follow unlabeled arrow.
    addState(l, s->out1);
    addState(l, s->out2);
  }
  l->states[l->size++] = s;
}

// Adds first state to list
StateList *startList(State *s, StateList *l) {
  listId++;
  l->size = 0;
  addState(l, s);
  return l;
}

int isMatch(StateList *l) {
  for (int i = 0; i < l->size; i++) {
    if (l->states[i]->symbol == MATCHSTATE) {
      return 1;
    };
  }
  return 0;
}

// Takes a step for all paths
void step(StateList *clist, int c, StateList *nlist) {
  State *s;

  listId++;
  nlist->size = 0;
  for (int i = 0; i < clist->size; i++) {
    s = clist->states[i];
    if (s->symbol == c || s->symbol == '.') {
      addState(nlist, s->out1);
    }
  }
}

int match(char *s) {
  StateList *clist, *nlist, *t;

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

char *preprocessPostfix(char *regex) {
  char *t = malloc(sizeof(char) * strlen(regex));
  int position = 0;
  for (; *(regex + 1) != '\0'; regex++) {
    char cur1 = *regex;
    char cur2 = *(regex + 1);

    t[position++] = cur1;
    if ((isAlphaNumeric(cur1) || cur1 == ')' || cur1 == '+' || cur1 == '*' ||
         cur1 == '?') &&
        (isAlphaNumeric(cur2) || cur2 == '(')) {

      t[position++] = '&';
    }
  }
  t[position++] = *regex;
  t[position++] = '\0';
  return t;
}
void compileRegex(char *regex) {
  char *preprocessed = preprocessPostfix(regex);
  char *postfix = infixToPostfix(preprocessed);
  startOfNfa = postfixToNfa(postfix);
}
