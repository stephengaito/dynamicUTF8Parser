/*
 * Regular expression implementation.
 * Supports only ( | ) * + ?.  No escapes.
 * Compiles to NFA and then simulates NFA
 * using Thompson's algorithm.
 * Caches steps of Thompson's algorithm to
 * build DFA on the fly, as in Aho's egrep.
 *
 * See also http://swtch.com/~rsc/regexp/ and
 * Thompson, Ken.  Regular Expression Search Algorithm,
 * Communications of the ACM 11(6) (June 1968), pp. 419-422.
 *
 * Copyright (c) 2007 Russ Cox.
 *
 * Extensive modifications for use as a utf8 lexer compiled by clang
 * are
 *   Copyright (c) 2015 Stephen Gaito
 *
 * Can be distributed under the MIT license, see bottom of file.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dfa.h"

DFA::DFA(NFA *anNFA, Classifier *aUTF8Classifier) {
  nfa = anNFA;
  utf8Classifier = aUTF8Classifier;
  nfaStatePtr2int = hattrie_create();
  dfaStateSize = (nfa->getNumberStates() / 8) + 1;
  dfaStateProbeSize = dfaStateSize + sizeof(utf8Char_t);
  dfaStateProbe = (char*)calloc(dfaStateProbeSize, sizeof(uint8_t));
  int2nfaStatePtr = (NFA::State**)calloc(nfa->getNumberStates(),
                                         sizeof(NFA::State*));
  numKnownNFAStates = 0;

  knownDFAStatesMap = hattrie_create();
  nextDFAStateMap   = hattrie_create();

  dStates      = NULL;
  curDState    = NULL;
  lastDState   = NULL;
  curDStateVector  = -1;
  numDStateVectors = 0;
  allocateANewDState(); // get space for the stateDState
  dfaStartState  = curDState;
  computeDFAStartState();
  allocateANewDState(); // get space for the tokensDState
  tokensDState = curDState;
  emptyDState(tokensDState);
}

DFA::~DFA(void) {
  nfa = NULL;
  utf8Classifier = NULL;
  if (nfaStatePtr2int) hattrie_free(nfaStatePtr2int);
  nfaStatePtr2int = NULL;
  if (dfaStateProbe) free(dfaStateProbe);
  dfaStateProbe = NULL;
  dfaStateProbeSize = 0;
  if (int2nfaStatePtr) free(int2nfaStatePtr);
  int2nfaStatePtr = NULL;
  numKnownNFAStates = 0;
  if (knownDFAStatesMap) hattrie_free(knownDFAStatesMap);
  knownDFAStatesMap = NULL;
  if (nextDFAStateMap) hattrie_free(nextDFAStateMap);
  nextDFAStateMap = NULL;

  if (dStates) {
    for (size_t i = 0; i < numDStateVectors; i++) {
      if (dStates[i]) free(dStates[i]);
      dStates[i] = NULL;
    }
    free(dStates);
  }
  dStates        = NULL;
  dfaStartState  = NULL;
  tokensDState   = NULL;
  curDState      = NULL;
  lastDState     = NULL;
  curDStateVector  = 0;
  numDStateVectors = 0;
}

void DFA::emptyDState(DFA::DState *dfaState) {
  if (!dfaState) throw LexerException("invalid DFA state");
  DState *dfaStateEnd = dfaState + dfaStateSize;
  for (; dfaState < dfaStateEnd; dfaState++) {
    *dfaState = 0;
  }
}

void DFA::allocateANewDState(void) {
  // TODO FINISH THIS
}

void DFA::assembleDFAStateProbe(DState *dfaState) {
  for (size_t i = 0; i < dfaStateSize; i++) {
    dfaStateProbe[i] = dfaState[i];
  }
}

void DFA::assembleDFAStateCharacterProbe(DState *dfaState,
                                         utf8Char_t curChar) {
  assembleDFAStateProbe(dfaState);
  for (size_t j = 0; j < sizeof(utf8Char_t); j++) {
    dfaStateProbe[dfaStateSize+j] = curChar.c[j];
  }
}

void DFA::assembleDFAStateClassificationProbe(DState *dfaState,
                                              classSet_t classification) {
  assembleDFAStateProbe(dfaState);
  for (size_t j = 0; j < sizeof(classSet_t); j++) {
    dfaStateProbe[dfaStateSize+j] = ((uint8_t*)(&classification))[j];
  }
}

void DFA::registerDState(DFA::DState *dfaState) {
  // TODO FINISH THIS
}

/* Check whether state list contains a match. */
bool DFA::matchesToken(DFA::DState *dfaState) {
  for (size_t i = 0; i < dfaStateSize; i++) {
    if (dfaState[i] & tokensDState[i]) return true;
  }
  return false;
}

DFA::NFAStateNumber DFA::getNFAStateNumber(NFA::State *nfaState) {
  NFA::State *nfaStatePtr = nfaState;
  value_t *nfaStateIntPtr = hattrie_get(nfaStatePtr2int,
                                        (char*)&nfaStatePtr,
                                        sizeof(NFA::State*));
  if (!nfaStateIntPtr) throw new LexerException("corrupted HAT-Trie nfaStatePtr2int");
  if (!*nfaStateIntPtr) {
    // this NFA::State needs to be added to our mapping
    int2nfaStatePtr[numKnownNFAStates] = nfaState;
    numKnownNFAStates++;
    *nfaStateIntPtr = numKnownNFAStates;
  }
  NFAStateNumber nfaStateNumber;
  nfaStateNumber.stateByte = *nfaStateIntPtr / 8;
  nfaStateNumber.stateBit  = 1 << (*nfaStateIntPtr % 8);
  // check to see if this NFAState is a token state
  // .... if so record it in the tokensDState
  if (nfaState->matchType == NFA::Token) {
   tokensDState[nfaStateNumber.stateByte] |= nfaStateNumber.stateBit;
  }
  return nfaStateNumber;
}

/* Add nfaState to dfaState, following unlabeled arrows. */
void DFA::addNFAStateToDFAState(DFA::DState *dfaState, NFA::State *nfaState) {
  if (nfaState == NULL) return;
  if (nfaState->matchType == NFA::Split) {
    /* follow unlabeled arrows */
    addNFAStateToDFAState(dfaState, nfaState->out);
    addNFAStateToDFAState(dfaState, nfaState->out1);
    return;
  }
  NFAStateNumber nfaStateNumber = getNFAStateNumber(nfaState);
  dfaState[nfaStateNumber.stateByte] |= nfaStateNumber.stateBit;
}

/* Compute initial state list */
void DFA::computeDFAStartState(void) {
  emptyDState(dfaStartState);
  addNFAStateToDFAState(dfaStartState, nfa->getNFAStartState());
  registerDState(dfaStartState);
}

/*
 * Step the NFA from the states in clist
 * past the character c,
 * to create next NFA state set nlist.
 */
void DFA::computeNextDFAState(DFA::DState *curDFAState,
               utf8Char_t c,
               classSet_t classificationSet,
               DFA::DState *nextDFAState) {
  NFA::State *nfaState;
  emptyDState(nextDFAState);

  size_t nfaStateNum = 0;
  for (size_t i = 0; i < dfaStateSize; i++) {
    uint8_t curByte = curDFAState[i] & 0xFF;
    for (size_t j = 0; j < 8; j++) {
      if (curByte & 0x01) {
        nfaState = int2nfaStatePtr[nfaStateNum];
        switch (nfaState->matchType) {
          case NFA::Character:
            if (nfaState->matchData.c.u == c.u) {
              addNFAStateToDFAState(nextDFAState, nfaState->out);
            }
            break;
          case NFA::ClassSet:
            if (nfaState->matchData.s & classificationSet) {
              addNFAStateToDFAState(nextDFAState, nfaState->out);
            }
            break;
          default:
            // do nothing
            break;
        }
      }
      curByte >>= 1;
      nfaStateNum++;
    }
  }
  // TODO NEED TO STORE THIS NEW STATE
}

/* Run DFA to determine whether it matches s. */
bool DFA::match(DFA::DState *dfaStartState, const char *stream) {
  DState *curDFAState, *nextDFAState;

  Utf8Chars *utf8Stream = new Utf8Chars(stream);

  curDFAState = dfaStartState;
  utf8Char_t curChar = utf8Stream->nextUtf8Char();
  while (curChar.u) {
    assembleDFAStateCharacterProbe(curDFAState, curChar);
    // now probe the nextDFAState hat-trie
    // start with most specific
    DState *nextDFAState = (DState*)hattrie_tryget(nextDFAStateMap,
                                                   dfaStateProbe,
                                                   dfaStateProbeSize);
    if (!nextDFAState) {
      // try the more generic
      classSet_t classificationSet = utf8Classifier->getClassSet(curChar);
      assembleDFAStateClassificationProbe(curDFAState, classificationSet);
      nextDFAState = (DState*)hattrie_tryget(nextDFAStateMap,
                                             dfaStateProbe,
                                             dfaStateProbeSize);
      if (!nextDFAState) {
        computeNextDFAState(curDFAState,
                            curChar,
                            classificationSet,
                            nextDFAState);
      }
    }
    curDFAState = nextDFAState;
  }
  return matchesToken(curDFAState);
}

#ifdef NOT_DEFINED

int main(int argc, char **argv) {
  int i;
  char *post;
  State *start;

  if (argc < 3) {
    fprintf(stderr, "usage: nfa regexp string...\n");
    return 1;
  }

  post = re2post(argv[1]);
  if (post == NULL) {
    fprintf(stderr, "bad regexp %s\n", argv[1]);
    return 1;
  }

  start = post2nfa(post);
  if (start == NULL) {
    fprintf(stderr, "error in post2nfa %s\n", post);
    return 1;
  }

  l1.s = malloc(nstate*sizeof l1.s[0]);
  l2.s = malloc(nstate*sizeof l2.s[0]);
  for (i=2; i<argc; i++) {
    if (match(startdstate(start), argv[i])) printf("%s\n", argv[i]);
  }
  return 0;
}
#endif

/*
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall
 * be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

