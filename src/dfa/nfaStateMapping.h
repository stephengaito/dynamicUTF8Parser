#ifndef DFA_NFA_STATE_MAPPING_H
#define DFA_NFA_STATE_MAPPING_H

#include "nfa.h"

namespace DeterministicFiniteAutomaton {

   /// \brief Our goal is to make a DFA::State very light weight.
   typedef char State;

  // Forward declare the DFA::StateAllocator since the
  // DFA::NFAStateMapping must know the allocator which allocate
  // the DFA::States for which this DFA::NFAStateMapping maps.
  class StateAllocator;

  /// \brief The NFA2DFAStateMapping class is used to build an invertible
  /// mapping from the NFA::State(s) of a given NFA to the DFAState(s) of a
  /// DFA which is interpreting the NFA.
  class NFAStateMapping {
    public:

      /// \brief Create an NFA2DFAStateMapping object corresponding to a
      /// given NFA.
      NFAStateMapping(StateAllocator *anAllocator);

      /// \brief Destroy the NFA2DFAStateMapping object.
      ~NFAStateMapping(void);

      /// \brief The NFAStateNumber structure represents a single
      /// bit in the DFA::DState bit set.
      ///
      /// The bit speficied by an NFAStateNumber, corresponds to a given
      /// NFA::State in the nfaStatePtr2int mapping.
      typedef struct NFAStateNumber {
        /// \brief The byte which contains this NFA::State bit.
        size_t  stateByte;
        /// \brief The bit which represents this NFA::State.
        uint8_t stateBit;
      } NFAStateNumber;

      /// \brief Return the NFAStateNumber corresponding to the given
      /// NFA::State.
      ///
      /// This method uses the nfaStatePtr2int mapping.
      NFAStateNumber getNFAStateNumber(NFA::State *nfaState)
        throw (LexerException);

      NFA::State *getNFAStateFor(size_t nfaStateNumber) {
        if (numKnownNFAStates <= nfaStateNumber) {
          throw LexerException("invalid NFA state requested in NFAStateMapping");
        }
        return int2nfaStatePtr[nfaStateNumber];
      }

    private:

      /// \brief The DFA::StateAllocator for this NFAStateMapping.
      ///
      /// This NFAStateMapping maps NFA states to corresponding
      /// DFA state bits for the DFA::States allocated by this
      /// DFA::StateAllocator.
      StateAllocator *allocator;

      /// \brief A vector of known NFA::States.
      ///
      /// This vector provides an integer to NFA::State mapping.
      ///
      /// Note that for a given NFA, the number of NFA::States is fixed,
      /// so the length of this vector if fixed when the DFA is created.
      NFA::State **int2nfaStatePtr;

      /// \brief The size of the int2nfaStatePtr vector.
      size_t int2nfaStatePtrSize;

      /// \brief The number of *currently* "known" NFA::State(s).
      ///
      /// The "known" NFA::State(s) are those that have been reached,
      /// while compiling the DFA on the fly, from the NFA start state.
      size_t numKnownNFAStates;

      /// \brief The Hat-Trie based nfaStatePtr to integer mapping.
      ///
      /// This is the inverse mapping to the int2nfaStatePtr mapping.
      hattrie_t   *nfaStatePtr2int;

  }; // class StateMapping
};  // namespace DeterministicFiniteAutomaton

#endif