#pragma once
#include <queue>
#include <string>
#include <unordered_set>
#include <vector>
// You are NOT allowed to add any headers
// without the permission of TAs.
namespace Grammar {
class NFA;
NFA MakeStar(const char &character);
NFA MakePlus(const char &character);
NFA MakeQuestion(const char &character);
NFA Concatenate(const NFA &nfa1, const NFA &nfa2);
NFA Union(const NFA &nfa1, const NFA &nfa2);
NFA MakeSimple(const char &character);
/*!
  \brief This class is used to store the type of the transition.
  \details You can use it like this:
  \code
      TransitionType type = TransitionType::Epsilon;
      TransitionType type2 = TransitionType::a;
      TransitionType type3 = TransitionType::b;
  \endcode
*/
enum class TransitionType { Epsilon, a, b };
struct Transition {
  /*!
    \brief value is used to store which character to match.
  */
  TransitionType type;
  /*!
    \brief This is used to store which node the transition is going to.
  */
  int to;
  Transition(TransitionType type, int to) : type(type), to(to) {}
};
class NFA {
private:
  /*!
    \brief This is used to store the start state of the NFA.
  */
  int start;
  /*!
    \brief This is used to store the end state of the NFA.
  */
  std::unordered_set<int> ends;
  /*!
    \brief This is used to store the transitions of the NFA.
    \details For example, transitions[3] stores all the transitions beginning
    with \details state 3.
  */
  std::vector<std::vector<Transition>> transitions;

public:
  NFA() = default;
  ~NFA() = default;
  /*!
    \brief Get the epsilon closure of a state.
    \param state The state to get the epsilon closure of.
    \return The epsilon closure of the state.
*/
  std::unordered_set<int>
  GetEpsilonClosure(std::unordered_set<int> states) const {
    std::unordered_set<int> closure;
    std::queue<int> queue;
    for (const auto &state : states) {
      if (closure.find(state) != closure.end())
        continue;
      queue.push(state);
      closure.insert(state);
    }
    while (!queue.empty()) {
      int current = queue.front();
      queue.pop();
      for (const auto &transition : transitions[current]) {
        if (transition.type == TransitionType::Epsilon) {
          if (closure.find(transition.to) == closure.end()) {
            queue.push(transition.to);
            closure.insert(transition.to);
          }
        }
      }
    }
    return closure;
  }
  /*!
    \brief Advance the NFA to the next states.
    \param current_states The current states of the NFA.
    \param character The character to match.
    \return The next states of the NFA.
  */
  std::unordered_set<int> Advance(std::unordered_set<int> current_states,
                                  char character) const {
    std::unordered_set<int> next;
    if (current_states.empty()) return next;
    auto closure = GetEpsilonClosure(current_states);
    TransitionType need = (character == 'a') ? TransitionType::a : TransitionType::b;
    std::unordered_set<int> direct;
    for (int s : closure) {
      for (const auto &tr : transitions[s]) {
        if (tr.type == need) direct.insert(tr.to);
      }
    }
    if (direct.empty()) return next;
    next = GetEpsilonClosure(direct);
    return next;
  }

  /*!
    \brief Check if a state is accepted.
    \param state The state to check.
    \return true if the state is accepted, false otherwise.
  */
  bool IsAccepted(int state) const { return ends.find(state) != ends.end(); }

  /*!
    \brief Return the start state.
    \return The start state.
  */

  int GetStart() const { return start; }

  friend NFA MakeStar(const char &character);
  friend NFA MakePlus(const char &character);
  friend NFA MakeQuestion(const char &character);
  friend NFA MakeSimple(const char &character);
  friend NFA Concatenate(const NFA &nfa1, const NFA &nfa2);
  friend NFA Union(const NFA &nfa1, const NFA &nfa2);
};
class RegexChecker {
private:
  /*!
    \brief This is used to store the regex string.
  */
  NFA nfa;

  static NFA BuildSequence(const std::string &seq) {
    // Build NFA for a concatenation-only sequence with postfix operators
    bool has = false;
    NFA result; // uninitialized until first token
    for (size_t i = 0; i < seq.size(); ++i) {
      char c = seq[i];
      if (c != 'a' && c != 'b') continue; // ignore invalid
      char op = 0;
      if (i + 1 < seq.size()) {
        char t = seq[i + 1];
        if (t == '*' || t == '+' || t == '?') { op = t; ++i; }
      }
      NFA atom;
      if (op == '*') atom = MakeStar(c);
      else if (op == '+') atom = MakePlus(c);
      else if (op == '?') atom = MakeQuestion(c);
      else atom = MakeSimple(c);

      if (!has) {
        result = atom;
        has = true;
      } else {
        result = Concatenate(result, atom);
      }
    }
    // If sequence is empty, create an epsilon NFA (matches empty)
    if (!has) {
      NFA eps;
      eps.start = 0;
      eps.ends.clear();
      eps.ends.insert(0);
      eps.transitions.assign(1, std::vector<Transition>());
      return eps;
    }
    return result;
  }

public:
  /*!
    \brief To check if the string is accepted by the regex.
    \param str The string to be checked.
    \return true if the string is accepted by the regex, false otherwise.
  */
  bool Check(const std::string &str) const {
    std::unordered_set<int> cur;
    cur.insert(nfa.start);
    for (char ch : str) {
      cur = nfa.Advance(cur, ch);
      if (cur.empty()) return false;
    }
    auto closure = nfa.GetEpsilonClosure(cur);
    for (int s : closure) if (nfa.IsAccepted(s)) return true;
    return false;
  }
  /*!
    \brief This is used to build the NFA from the regex string.
  */
  RegexChecker(const std::string &regex) {
    // Split by '|' (no parentheses, so flat)
    std::vector<std::string> parts;
    std::string cur;
    for (char ch : regex) {
      if (ch == '|') { parts.push_back(cur); cur.clear(); }
      else cur.push_back(ch);
    }
    parts.push_back(cur);

    NFA built;
    bool has = false;
    for (const auto &p : parts) {
      NFA seq = BuildSequence(p);
      if (!has) { built = seq; has = true; }
      else { built = Union(built, seq); }
    }
    nfa = built;
  }
};

/*!
  \brief Return a NFA to match a* or b*.
  \param character The character to match.
  \details This function will create a NFA that matches a* or b*.
*/
NFA MakeStar(const char &character) {
  NFA nfa;
  nfa.start = 0;
  nfa.ends.insert(0);
  nfa.transitions.push_back(std::vector<Transition>());
  if (character == 'a') {
    nfa.transitions[0].push_back({TransitionType::a, 0});
  } else {
    nfa.transitions[0].push_back({TransitionType::b, 0});
  }
  return nfa;
}
/*!
  \brief Return a NFA to match a+ or b+.
  \param character The character to match.
  \details This function will create a NFA that matches a+ or b+.
*/
NFA MakePlus(const char &character) {
  NFA nfa;
  nfa.start = 0;
  nfa.ends.clear();
  nfa.ends.insert(1);
  nfa.transitions.assign(2, std::vector<Transition>());
  TransitionType t = (character == 'a') ? TransitionType::a : TransitionType::b;
  nfa.transitions[0].push_back({t, 1});
  nfa.transitions[1].push_back({t, 1});
  return nfa;
}
/*!
  \brief Return a NFA to match a? or b?.
  \param character The character to match.
  \details This function will create a NFA that matches a? or b?.
*/
NFA MakeQuestion(const char &character) {
  NFA nfa;
  nfa.start = 0;
  nfa.ends.clear();
  nfa.ends.insert(1);
  nfa.transitions.assign(2, std::vector<Transition>());
  TransitionType t = (character == 'a') ? TransitionType::a : TransitionType::b;
  nfa.transitions[0].push_back({TransitionType::Epsilon, 1});
  nfa.transitions[0].push_back({t, 1});
  return nfa;
}

/*!
  \brief Return a NFA to match ...(the first part) ...(the second part).
  \param nfa1 The first NFA to match, for example, it is a.
  \param nfa2 The second NFA to match, for example, it is b.
  \details This function will create a NFA that matches ab.
*/
NFA Concatenate(const NFA &nfa1, const NFA &nfa2) {
  NFA res;
  int n1 = (int)nfa1.transitions.size();
  int n2 = (int)nfa2.transitions.size();
  res.start = 0; // nfa1.start is 0 in our constructors
  res.ends.clear();
  res.transitions.assign(n1 + n2, std::vector<Transition>());
  // copy nfa1
  for (int i = 0; i < n1; ++i) {
    for (const auto &tr : nfa1.transitions[i]) res.transitions[i].push_back(tr);
  }
  // epsilon from each end of nfa1 to start of nfa2 (offset)
  int off2 = n1;
  for (int e : nfa1.ends) {
    res.transitions[e].push_back({TransitionType::Epsilon, off2 + nfa2.start});
  }
  // copy nfa2 with offset
  for (int i = 0; i < n2; ++i) {
    for (const auto &tr : nfa2.transitions[i]) {
      res.transitions[off2 + i].push_back({tr.type, off2 + tr.to});
    }
  }
  for (int e : nfa2.ends) res.ends.insert(off2 + e);
  return res;
}
/*!
  \brief Return a NFA to match ... | ...
  \param nfa1 The first NFA to match, for example, it is a.
  \param nfa2 The second NFA to match, for example, it is b.
  \details This function will create a NFA that matches a|b.
*/
NFA Union(const NFA &nfa1, const NFA &nfa2) {
  NFA res;
  int n1 = (int)nfa1.transitions.size();
  int n2 = (int)nfa2.transitions.size();
  res.start = 0;
  res.ends.clear();
  res.transitions.assign(1 + n1 + n2, std::vector<Transition>());
  int off1 = 1;
  int off2 = 1 + n1;
  // start epsilons
  res.transitions[0].push_back({TransitionType::Epsilon, off1 + nfa1.start});
  res.transitions[0].push_back({TransitionType::Epsilon, off2 + nfa2.start});
  // copy nfa1
  for (int i = 0; i < n1; ++i) {
    for (const auto &tr : nfa1.transitions[i])
      res.transitions[off1 + i].push_back({tr.type, off1 + tr.to});
  }
  // copy nfa2
  for (int i = 0; i < n2; ++i) {
    for (const auto &tr : nfa2.transitions[i])
      res.transitions[off2 + i].push_back({tr.type, off2 + tr.to});
  }
  for (int e : nfa1.ends) res.ends.insert(off1 + e);
  for (int e : nfa2.ends) res.ends.insert(off2 + e);
  return res;
}

/*!
  \brief Return a NFA to match a, or return a NFA to match b.
  \param character The character to match.
  \return The required NFA.
*/
NFA MakeSimple(const char &character) {
  NFA nfa;
  nfa.start = 0;
  nfa.ends.clear();
  nfa.ends.insert(1);
  nfa.transitions.assign(2, std::vector<Transition>());
  TransitionType t = (character == 'a') ? TransitionType::a : TransitionType::b;
  nfa.transitions[0].push_back({t, 1});
  return nfa;
}
} // namespace Grammar
