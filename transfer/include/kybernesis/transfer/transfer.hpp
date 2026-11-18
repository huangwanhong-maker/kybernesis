// Transfer between relational systems: a practice carried from the system it works in to another (decision 0004).
//
// A practice is a policy over the states of the system it came from, J. Carrying it to another system X needs two maps, a lens
// (the idea of 2026-11-18, §3.7):
//
//   f : S_X → S_J   the interpretation: the target read in the source's terms (a law over S_J for each state of X);
//   g : A_J → A_X   the realisation: each of the source's actions realised by an action of the target.
//
// The carried policy is π_X = g ∘ π_J ∘ f_*, applied to the target's belief pushed forward along f. It is warranted where the
// mechanism commutes: acting in X by the realised action and reading the result in J's terms is what J's own mechanism does
// from the state read,
//
//   f ∘ τ_X ∘ (1 ⊗ g)  ≈  τ_J ∘ (f ⊗ 1)   : S_X ⊗ A_J → S_J,
//
// within a declared tolerance. Two answers are given, kept apart:
//
//   the square in relations   decided by kategoria (kategoria::monoidal::holds_in, boolean matrices): whether the supports
//                             agree exactly, on the states and actions considered; where they do not, which side reaches what
//                             the other does not;
//   the square in laws        the total variation between the two laws over S_J at every state of X and action of J, and the
//                             states at which every action considered commutes within the tolerance.
//
// Support factors (Cartwright and Hardie 2012) are conditions on the target's state. A factor is sufficient when the square
// commutes wherever it holds, and necessary when the square commutes nowhere it fails. Whether the target is believed to meet
// it is a dual verdict over the readings of the target's open terms: each reading gives a lens and the target's belief under
// it. "It will work here" stands clear when, under every reading, the target is believed to be only where the square commutes;
// untenable when, under every reading, only where it does not; arguable otherwise, with the readings it turns on. Its measure
// is the interval of the probability, over the readings, that the target is at a state where the square commutes.
//
// Caveat. Commuting is the agreement of mechanisms, not the success of the practice: two systems in which a practice fails in
// the same way commute there. What the practice achieves in the target is its own question, answered by the carried policy
// and the target's dynamics.
#pragma once

#include <kybernesis/kernel/doctrines.hpp>
#include <kybernesis/kernel/verdicts.hpp>

#include <kategoria/monoidal/monoidal.hpp>

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace kybernesis::transfer {

using kernel::Distribution;
using kernel::Verdict;
namespace km = kategoria::monoidal;

// A declared system: finite states and actions, and the law of the next state for each state and action.
struct System {
  std::string name;
  std::vector<std::string> states, actions;
  std::vector<std::vector<Distribution>> dynamics;  // dynamics[s][a]: a law over states
};

struct Lens {
  std::vector<Distribution> interpret;  // f: for each target state, a law over the source's states
  std::vector<std::size_t> realise;     // g: for each source action, a target action
};

// A reading of the target's open terms, with its source: the lens it gives and the target's belief under it.
struct Reading {
  std::string name, source;
  Lens lens;
  Distribution belief;  // over the target's states
};

struct Factor {
  std::string name, source;  // source: how its truth on the target is known (computed, declared, a model's proposal…)
  std::vector<char> holds;   // over the target's states
};

struct Pair {
  std::size_t state = 0, action = 0;  // a target state, a source action
  double distance = 0;                // total variation between the two laws over the source's states
};

struct Square {
  std::string reading;
  Verdict relations;               // the square in relations, decided by kategoria on the states and actions considered
  std::vector<std::string> beyond; // where the target reaches, in the source's terms, what the source does not
  std::vector<std::string> short_; // where the source reaches what the target does not
  std::vector<Pair> pairs;         // every pair considered, the worst first
  double worst = 0;
  std::vector<char> commutes;      // per target state: every action considered commutes within the tolerance
};

struct FactorFinding {
  std::string name, source;
  kernel::Standing standing = kernel::Standing::undecided;  // whether the target is believed to meet it, over the readings
  kernel::Measure measure;                                  // the probability that it holds, over the readings
  bool sufficient = false, necessary = false;               // for the square, under every reading
  std::string words;
};

struct Assessment {
  std::vector<Square> squares;               // one per reading
  kernel::Finding<bool> carries;             // "it will work here": standing over the readings, measure the interval
  std::vector<std::string> by_reading;       // under each reading: shown, open or refuted, and why
  std::vector<FactorFinding> factors;
  Verdict verdict;                           // whether the assessment could be made
};

namespace detail {

inline bool law(const Distribution& p, std::size_t n) {
  if (p.size() != n) return false;
  double s = 0;
  for (double x : p) {
    if (!(x >= 0) || !std::isfinite(x)) return false;
    s += x;
  }
  return std::fabs(s - 1) < 1e-9;
}

inline Distribution mix(const Distribution& weights, const std::vector<Distribution>& rows, std::size_t n) {
  Distribution out(n, 0.0);
  for (std::size_t i = 0; i < weights.size(); i++)
    if (weights[i] > 0)
      for (std::size_t j = 0; j < n; j++) out[j] += weights[i] * rows[i][j];
  return out;
}

inline double tv(const Distribution& p, const Distribution& q) {
  double s = 0;
  for (std::size_t i = 0; i < p.size(); i++) s += std::fabs(p[i] - q[i]);
  return s / 2;
}

inline double mass(const Distribution& b, const std::vector<char>& set) {
  double s = 0;
  for (std::size_t i = 0; i < b.size() && i < set.size(); i++)
    if (set[i]) s += b[i];
  return s;
}

inline bool within(const Distribution& b, const std::vector<char>& set) {
  for (std::size_t i = 0; i < b.size(); i++)
    if (b[i] > 0 && !set[i]) return false;
  return true;
}

inline bool apart(const Distribution& b, const std::vector<char>& set) {
  for (std::size_t i = 0; i < b.size(); i++)
    if (b[i] > 0 && set[i]) return false;
  return true;
}

inline std::string list(const std::vector<std::string>& names, const std::vector<char>& set) {
  std::string s;
  for (std::size_t i = 0; i < names.size(); i++)
    if (set[i]) s += (s.empty() ? "" : ", ") + names[i];
  return s.empty() ? "none" : s;
}

}  // namespace detail

inline Verdict well_formed(const System& s) {
  const std::string claim = "the system " + s.name + " is well formed";
  if (s.states.empty() || s.actions.empty()) return Verdict::no(claim, "it has no states or no actions");
  if (s.dynamics.size() != s.states.size()) return Verdict::no(claim, "its dynamics has " + std::to_string(s.dynamics.size()) + " rows for " + std::to_string(s.states.size()) + " states");
  for (std::size_t i = 0; i < s.states.size(); i++) {
    if (s.dynamics[i].size() != s.actions.size()) return Verdict::no(claim, "from " + s.states[i] + " its dynamics does not give every action");
    for (std::size_t a = 0; a < s.actions.size(); a++)
      if (!detail::law(s.dynamics[i][a], s.states.size())) return Verdict::no(claim, "from " + s.states[i] + " under " + s.actions[a] + " its next state is no law over its states");
  }
  return Verdict::yes(claim);
}

inline Verdict well_formed(const Lens& l, const System& source, const System& target) {
  const std::string claim = "the lens from " + target.name + " to " + source.name + " is well formed";
  if (l.interpret.size() != target.states.size()) return Verdict::no(claim, "it interprets " + std::to_string(l.interpret.size()) + " states of " + std::to_string(target.states.size()));
  for (std::size_t s = 0; s < target.states.size(); s++)
    if (!detail::law(l.interpret[s], source.states.size())) return Verdict::no(claim, "it reads " + target.states[s] + " as no law over the source's states");
  if (l.realise.size() != source.actions.size()) return Verdict::no(claim, "it realises " + std::to_string(l.realise.size()) + " actions of " + std::to_string(source.actions.size()));
  for (std::size_t a = 0; a < source.actions.size(); a++)
    if (l.realise[a] >= target.actions.size()) return Verdict::no(claim, "it realises " + source.actions[a] + " by no action of " + target.name);
  return Verdict::yes(claim);
}

// The target's belief read in the source's terms: f_* b.
inline Distribution push(const Lens& l, const Distribution& belief, std::size_t source_states) { return detail::mix(belief, l.interpret, source_states); }

// The carried policy's action for a target belief: g(π_J(f_* b)).
inline std::size_t carry(const std::function<std::size_t(const Distribution&)>& source_policy, const Lens& l, const Distribution& belief, std::size_t source_states) {
  return l.realise.at(source_policy(push(l, belief, source_states)));
}

// The square under one lens, on the target states and source actions considered (all, when the masks are empty).
inline Square square(const System& source, const System& target, const Lens& l, const std::string& reading, std::vector<char> states = {}, std::vector<char> actions = {},
                     double tolerance = 0.0) {
  Square q;
  q.reading = reading;
  const std::size_t nX = target.states.size(), nJ = source.states.size(), aJ = source.actions.size(), aX = target.actions.size();
  if (states.empty()) states.assign(nX, 1);
  if (actions.empty()) actions.assign(aJ, 1);
  // in laws
  q.commutes.assign(nX, 1);
  for (std::size_t s = 0; s < nX; s++) {
    for (std::size_t a = 0; a < aJ; a++) {
      if (!actions[a]) continue;
      const Distribution& next = target.dynamics[s][l.realise[a]];
      const Distribution left = detail::mix(next, l.interpret, nJ);  // f ∘ τ_X(s, g a)
      std::vector<Distribution> rows;
      for (std::size_t j = 0; j < nJ; j++) rows.push_back(source.dynamics[j][a]);
      const Distribution right = detail::mix(l.interpret[s], rows, nJ);  // τ_J(f s, a)
      const double d = detail::tv(left, right);
      if (d > tolerance + 1e-12) q.commutes[s] = 0;
      if (states[s]) q.pairs.push_back({s, a, d});
    }
  }
  std::stable_sort(q.pairs.begin(), q.pairs.end(), [](const Pair& x, const Pair& y) { return x.distance > y.distance; });
  q.worst = q.pairs.empty() ? 0.0 : q.pairs.front().distance;
  // in relations: types SX, AJ, SJ, AX; generators f, g, tX, tJ, and the restrictions to what is considered
  km::Signature S{"Transfer", {"SX", "AJ", "SJ", "AX"},
                  {{"f", {0}, {2}}, {"g", {1}, {3}}, {"tX", {0, 3}, {0}}, {"tJ", {2, 1}, {2}}, {"rX", {0}, {0}}, {"rA", {1}, {1}}}};
  km::Model M;
  M.name = "the supports of " + target.name + " and " + source.name + " under " + reading;
  M.semiring = km::Semiring::boolean;
  M.dimension = {nX, aJ, nJ, aX};
  km::Matrix f{nJ, nX, std::vector<long long>(nJ * nX, 0)};
  for (std::size_t s = 0; s < nX; s++)
    for (std::size_t j = 0; j < nJ; j++) f.at(j, s) = l.interpret[s][j] > 0;
  km::Matrix g{aX, aJ, std::vector<long long>(aX * aJ, 0)};
  for (std::size_t a = 0; a < aJ; a++) g.at(l.realise[a], a) = 1;
  km::Matrix tX{nX, nX * aX, std::vector<long long>(nX * nX * aX, 0)};
  for (std::size_t s = 0; s < nX; s++)
    for (std::size_t b = 0; b < aX; b++)
      for (std::size_t s2 = 0; s2 < nX; s2++) tX.at(s2, s * aX + b) = target.dynamics[s][b][s2] > 0;
  km::Matrix tJ{nJ, nJ * aJ, std::vector<long long>(nJ * nJ * aJ, 0)};
  for (std::size_t j = 0; j < nJ; j++)
    for (std::size_t a = 0; a < aJ; a++)
      for (std::size_t j2 = 0; j2 < nJ; j2++) tJ.at(j2, j * aJ + a) = source.dynamics[j][a][j2] > 0;
  km::Matrix rX{nX, nX, std::vector<long long>(nX * nX, 0)};
  for (std::size_t s = 0; s < nX; s++) rX.at(s, s) = states[s] ? 1 : 0;
  km::Matrix rA{aJ, aJ, std::vector<long long>(aJ * aJ, 0)};
  for (std::size_t a = 0; a < aJ; a++) rA.at(a, a) = actions[a] ? 1 : 0;
  M.generators = {f, g, tX, tJ, rX, rA};
  const km::Term restrict = km::par({km::gen(4), km::gen(5)});
  const km::Term left = km::seq({restrict, km::par({km::id({0}), km::gen(1)}), km::gen(2), km::gen(0)});
  const km::Term right = km::seq({restrict, km::par({km::gen(0), km::id({1})}), km::gen(3)});
  const auto v = km::holds_in(S, M, left, right, "the square commutes in relations under " + reading);
  const auto st = v.status == kategoria::kernel::Status::holds ? kernel::Status::holds : v.status == kategoria::kernel::Status::fails ? kernel::Status::fails : kernel::Status::undecided;
  q.relations = {st, v.claim, v.reason, {}, v.examined};
  if (q.relations.fails()) {
    const km::Matrix L = km::evaluate(S, M, left), R = km::evaluate(S, M, right);
    for (std::size_t s = 0; s < nX; s++)
      for (std::size_t a = 0; a < aJ; a++)
        for (std::size_t j = 0; j < nJ; j++) {
          const bool l1 = L.at(j, s * aJ + a) != 0, r1 = R.at(j, s * aJ + a) != 0;
          const std::string at = "from " + target.states[s] + " under " + source.actions[a] + ", " + source.states[j];
          if (l1 && !r1) q.beyond.push_back(at);
          if (r1 && !l1) q.short_.push_back(at);
        }
  }
  return q;
}

// "It will work here", over the readings of the target's terms, with each support factor.
inline Assessment assess(const System& source, const System& target, const std::vector<Reading>& readings, const std::vector<Factor>& factors, std::vector<char> actions = {},
                         double tolerance = 0.0) {
  Assessment out;
  const std::string claim = "the transfer from " + source.name + " to " + target.name + " is assessed";
  if (const auto v = well_formed(source); v.fails()) return out.verdict = Verdict::no(claim, v.reason), out;
  if (const auto v = well_formed(target); v.fails()) return out.verdict = Verdict::no(claim, v.reason), out;
  if (readings.empty()) return out.verdict = Verdict::no(claim, "no reading of the target's terms is given"), out;
  for (const auto& r : readings) {
    if (const auto v = well_formed(r.lens, source, target); v.fails()) return out.verdict = Verdict::no(claim, "under " + r.name + ": " + v.reason), out;
    if (!detail::law(r.belief, target.states.size())) return out.verdict = Verdict::no(claim, "under " + r.name + " the target's belief is no law over its states"), out;
  }
  for (const auto& f : factors)
    if (f.holds.size() != target.states.size()) return out.verdict = Verdict::no(claim, "the factor " + f.name + " is not given on every state of " + target.name), out;
  std::size_t shown = 0, refuted = 0;
  double lo = 1, hi = 0;
  for (const auto& r : readings) {
    Square q = square(source, target, r.lens, r.name, {}, actions, tolerance);
    const double p = detail::mass(r.belief, q.commutes);
    lo = std::min(lo, p), hi = std::max(hi, p);
    if (detail::within(r.belief, q.commutes)) shown++, out.by_reading.push_back("under " + r.name + " (" + r.source + "): shown, the target is believed only where the square commutes");
    else if (detail::apart(r.belief, q.commutes)) refuted++, out.by_reading.push_back("under " + r.name + " (" + r.source + "): refuted, the target is believed only where it does not");
    else out.by_reading.push_back("under " + r.name + " (" + r.source + "): open, probability " + kernel::number(p) + " that the square commutes where the target is");
    out.squares.push_back(std::move(q));
  }
  out.carries.value = shown == readings.size();
  out.carries.standing = shown == readings.size() ? kernel::Standing::clear : refuted == readings.size() ? kernel::Standing::untenable : kernel::Standing::arguable;
  out.carries.measure = kernel::Measure::interval(std::clamp(lo, 0.0, 1.0), std::clamp(hi, 0.0, 1.0));
  out.carries.provenance.method = "transfer by a lens";
  out.carries.provenance.family = kernel::Family::formal;
  for (const auto& f : factors) {
    FactorFinding ff;
    ff.name = f.name, ff.source = f.source;
    std::size_t within = 0, apart = 0;
    double flo = 1, fhi = 0;
    bool sufficient = true, necessary = true;
    for (std::size_t k = 0; k < readings.size(); k++) {
      const auto& b = readings[k].belief;
      const double p = detail::mass(b, f.holds);
      flo = std::min(flo, p), fhi = std::max(fhi, p);
      within += detail::within(b, f.holds), apart += detail::apart(b, f.holds);
      for (std::size_t s = 0; s < f.holds.size(); s++) {
        if (f.holds[s] && !out.squares[k].commutes[s]) sufficient = false;
        if (!f.holds[s] && out.squares[k].commutes[s]) necessary = false;
      }
    }
    ff.standing = within == readings.size() ? kernel::Standing::clear : apart == readings.size() ? kernel::Standing::untenable : kernel::Standing::arguable;
    ff.measure = kernel::Measure::interval(std::clamp(flo, 0.0, 1.0), std::clamp(fhi, 0.0, 1.0));
    ff.sufficient = sufficient, ff.necessary = necessary;
    ff.words = f.name + " (" + f.source + ") is " + kernel::name(ff.standing) + " for " + target.name + ", probability " + kernel::words(ff.measure) + "; " +
               (sufficient && necessary ? "the square commutes exactly where it holds" : sufficient ? "where it holds the square commutes" : necessary ? "where it fails the square does not commute" : "the square does not follow it") +
               " (where it holds: " + detail::list(target.states, f.holds) + ")";
    out.factors.push_back(std::move(ff));
  }
  out.verdict = Verdict::yes(claim, std::string("it will work here: ") + kernel::name(out.carries.standing) + ", " + kernel::words(out.carries.measure));
  return out;
}

}  // namespace kybernesis::transfer
