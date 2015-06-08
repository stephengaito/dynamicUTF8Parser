#include <bandit/bandit.h>
using namespace bandit;

#include <string.h>
#include <stdio.h>
#include <exception>

#ifndef private
#define private public
#endif

#include "nfaBuilder.h"
#include "dfa/automataState.h"

using namespace DeterministicFiniteAutomaton;

// TODO: TEST MEMORY LEAKS (using C++ mocks)

go_bandit([](){

  printf("\n----------------------------------\n");
  printf(  "AutomataState\n");
  printf(  "AutomataState = %zu bytes (%zu bits)\n", sizeof(AutomataState),  sizeof(AutomataState)*8);
  printf(  "----------------------------------\n");

  /// \brief We test the AutomataState class.
  describe("AutomataState", [](){

    it("Create a NULL AutomataState and initialize it", [&](){
      AutomataState automataState;
      AssertThat(automataState.message, Equals((void*)0));
      AssertThat(automataState.tokens, Equals((void*)0));
      AssertThat(automataState.stream, Equals((void*)0));
      AssertThat(automataState.iterator, Equals((void*)0));
      AssertThat(automataState.dState, Equals((void*)0));
      AssertThat(automataState.allocator, Equals((void*)0));
      Classifier *classifier = new Classifier();
      NFA *nfa = new NFA(classifier);
      NFABuilder *nfaBuilder = new NFABuilder(nfa);
      nfaBuilder->compileRegularExpressionForTokenId("start", "simple", 1);
      ParseTrees *parseTrees = new ParseTrees();
      DFA *dfa = new DFA(nfa, parseTrees);
      StateAllocator *allocator = dfa->getStateAllocator();
      Utf8Chars *someChars = new Utf8Chars("some characters");
      const char *testMessage = "this is a test message";
      State *dState = dfa->getDFAStartState("start");
      automataState.initialize(allocator, someChars, dState, testMessage);
      AssertThat(automataState.message, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.message, Is().Not().EqualTo((char*)testMessage));
      AssertThat(automataState.message, Equals(testMessage));
      AssertThat(automataState.tokens, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.stream, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.stream, Is().Not().EqualTo(someChars));
      AssertThat(automataState.iterator, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.dState, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.dState, Is().Not().EqualTo(dState));
      AssertThat(automataState.allocator, Is().Not().EqualTo((void*)0));
    });

    it("Initialize an AutomataState and update it", [&](){
      Classifier *classifier = new Classifier();
      NFA *nfa = new NFA(classifier);
      NFABuilder *nfaBuilder = new NFABuilder(nfa);
      nfaBuilder->compileRegularExpressionForTokenId("start", "simple", 1);
      nfaBuilder->compileRegularExpressionForTokenId("other", "otherSimple", 1);
      ParseTrees *parseTrees = new ParseTrees();
      DFA *dfa = new DFA(nfa, parseTrees);
      StateAllocator *allocator = dfa->getStateAllocator();
      Utf8Chars *someChars = new Utf8Chars("some characters");
      const char *testMessage = "this is a test message";
      State *dState = dfa->getDFAStartState("start");
      AutomataState automataState;
      automataState.initialize(allocator, someChars, dState, testMessage);
      AssertThat(automataState.message, Is().Not().EqualTo((char*)testMessage));
      AssertThat(automataState.message, Equals(testMessage));
      AssertThat(automataState.tokens, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.stream, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.iterator, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.dState, Is().Not().EqualTo(dState));
      AssertThat(automataState.allocator, Equals(allocator));
      State *otherDState = dfa->getDFAStartState("other");
      const char *otherMessage = "this is another message";
      automataState.update(otherDState, otherMessage);
      AssertThat(automataState.message, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.message, Is().Not().EqualTo((char*)testMessage));
      AssertThat(automataState.message, Is().Not().EqualTo((char*)otherMessage));
      AssertThat(automataState.message, Equals(otherMessage));
      AssertThat(automataState.tokens, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.stream, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.iterator, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.dState, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.dState, Is().Not().EqualTo(dState));
      AssertThat(automataState.dState, Is().Not().EqualTo(otherDState));
      AssertThat(automataState.allocator, Equals(allocator));
    });

    it("CopyFrom an AutomataState", [&](){
      Classifier *classifier = new Classifier();
      NFA *nfa = new NFA(classifier);
      NFABuilder *nfaBuilder = new NFABuilder(nfa);
      nfaBuilder->compileRegularExpressionForTokenId("start", "simple", 1);
      nfaBuilder->compileRegularExpressionForTokenId("other", "otherSimple", 1);
      ParseTrees *parseTrees = new ParseTrees();
      DFA *dfa = new DFA(nfa, parseTrees);
      StateAllocator *allocator = dfa->getStateAllocator();
      Utf8Chars *someChars = new Utf8Chars("some characters");
      const char *testMessage = "this is a test message";
      State *dState = dfa->getDFAStartState("start");
      AutomataState automataState;
      automataState.initialize(allocator, someChars, dState, testMessage);
      AssertThat(automataState.message, Is().Not().EqualTo((char*)testMessage));
      AssertThat(automataState.message, Equals(testMessage));
      AssertThat(automataState.tokens, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.stream, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.iterator, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.dState, Is().Not().EqualTo(dState));
      AssertThat(automataState.allocator, Equals(allocator));
      AutomataState newAutomataState;
      AssertThat(newAutomataState.message, Equals((void*)0));
      AssertThat(newAutomataState.tokens, Equals((void*)0));
      AssertThat(newAutomataState.stream, Equals((void*)0));
      AssertThat(newAutomataState.iterator, Equals((void*)0));
      AssertThat(newAutomataState.dState, Equals((void*)0));
      AssertThat(newAutomataState.allocator, Equals((void*)0));
      newAutomataState.copyFrom(automataState, true);
      AssertThat(newAutomataState.message, Equals((char*)automataState.message));
      AssertThat(newAutomataState.tokens, Equals(automataState.tokens));
      AssertThat(newAutomataState.stream, Equals((void*)0));
      AssertThat(newAutomataState.iterator, Equals(automataState.iterator));
      AssertThat(newAutomataState.dState, Equals(automataState.dState));
      AssertThat(newAutomataState.allocator, Equals(allocator));
      newAutomataState.copyFrom(automataState, false);
      AssertThat(newAutomataState.message, Equals((char*)automataState.message));
      AssertThat(newAutomataState.tokens, Equals(automataState.tokens));
      AssertThat(newAutomataState.stream, Equals(automataState.stream));
      AssertThat(newAutomataState.iterator, Equals(automataState.iterator));
      AssertThat(newAutomataState.dState, Equals(automataState.dState));
      AssertThat(newAutomataState.allocator, Equals(allocator));
    });

    it("Clear an AutomataState", [&](){
      Classifier *classifier = new Classifier();
      NFA *nfa = new NFA(classifier);
      NFABuilder *nfaBuilder = new NFABuilder(nfa);
      nfaBuilder->compileRegularExpressionForTokenId("start", "simple", 1);
      nfaBuilder->compileRegularExpressionForTokenId("other", "otherSimple", 1);
      ParseTrees *parseTrees = new ParseTrees();
      DFA *dfa = new DFA(nfa, parseTrees);
      StateAllocator *allocator = dfa->getStateAllocator();
      Utf8Chars *someChars = new Utf8Chars("some characters");
      const char *testMessage = "this is a test message";
      State *dState = dfa->getDFAStartState("start");
      AutomataState automataState;
      automataState.initialize(allocator, someChars, dState, testMessage);
      AssertThat(automataState.message, Is().Not().EqualTo((char*)testMessage));
      AssertThat(automataState.message, Equals(testMessage));
      AssertThat(automataState.tokens, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.stream, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.iterator, Is().Not().EqualTo((void*)0));
      AssertThat(automataState.dState, Is().Not().EqualTo(dState));
      AssertThat(automataState.allocator, Equals(allocator));
      automataState.clear();
      AssertThat(automataState.message, Equals((void*)0));
      AssertThat(automataState.tokens, Equals((void*)0));
      AssertThat(automataState.stream, Equals((void*)0));
      AssertThat(automataState.iterator, Equals((void*)0));
      AssertThat(automataState.dState, Equals((void*)0));
      AssertThat(automataState.allocator, Equals((void*)0));
    });

  }); // describe parser

});