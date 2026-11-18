// Dual verdicts: what the framework concludes, with its standing and its measure, and what it rests on (decision 0002).
//
// Every conclusion of kybernesis (an estimate, an evaluation, a choice of action, the admission of a method or of a revision
// of the architecture) is a finding with two answers kept apart:
//
//   standing   the formal answer: clear, arguable or untenable over the admissible readings of its open terms, or not
//              decided (apodeixis's derivability under interpretation, tekmerion's construal, an argument's grounded status);
//   measure    the statistical answer: a probability, an interval of probabilities, or an estimate with its error.
//
// A standard says how the two combine into a verdict, as the law domain's standards of proof do ("a probability, an
// argument's standing, or both"). Methods of either family answer in this one type, so that formal reasoning and statistics
// are compared and combined where they meet instead of in each method's own terms.
//
// A finding records its provenance: the method, the library it wraps, its family and grade, and what it rests on. A proposal
// (a model's word, grade C) never holds by a standard: it is shown as proposed, never as the record's.
#pragma once

#include <apodeixis/kernel/verdict.hpp>

#include <cmath>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace kybernesis::kernel {

using apodeixis::kernel::Status;
using apodeixis::kernel::Verdict;
using apodeixis::kernel::word;

// The family of a method: declared by a person or a body; formal (derivation, argument, logic); statistical (estimation,
// probability); a proposal (a model's or a person's word without warrant yet); or the world itself (what a referred block is
// filled with in a simulation, never in a record).
enum class Family { declared, formal, statistical, proposal, world };

inline const char* name(Family f) {
  switch (f) {
    case Family::declared: return "declared";
    case Family::formal: return "formal";
    case Family::statistical: return "statistical";
    case Family::proposal: return "proposal";
    default: return "world";
  }
}

inline Family family_of(const std::string& s) {
  for (Family f : {Family::declared, Family::formal, Family::statistical, Family::proposal, Family::world})
    if (s == name(f)) return f;
  throw std::invalid_argument("kybernesis::kernel: no family named " + s);
}

// The formal answer.
enum class Standing { clear, arguable, untenable, undecided };

inline const char* name(Standing s) {
  switch (s) {
    case Standing::clear: return "clear";
    case Standing::arguable: return "arguable";
    case Standing::untenable: return "untenable";
    default: return "undecided";
  }
}

// The statistical answer.
struct Measure {
  enum class Kind { none, probability, interval, estimate };
  Kind kind = Kind::none;
  double value = std::numeric_limits<double>::quiet_NaN();
  double lower = std::numeric_limits<double>::quiet_NaN(), upper = std::numeric_limits<double>::quiet_NaN();
  double error = std::numeric_limits<double>::quiet_NaN();

  static Measure nothing() { return {}; }
  static Measure probability(double p) {
    if (!(p >= 0 && p <= 1)) throw std::invalid_argument("kybernesis::kernel: a probability lies in [0, 1]");
    Measure m;
    m.kind = Kind::probability, m.value = m.lower = m.upper = p;
    return m;
  }
  static Measure interval(double lo, double hi) {
    if (!(lo >= 0 && lo <= hi && hi <= 1)) throw std::invalid_argument("kybernesis::kernel: an interval of probabilities is 0 <= lo <= hi <= 1");
    Measure m;
    m.kind = Kind::interval, m.lower = lo, m.upper = hi, m.value = (lo + hi) / 2;
    return m;
  }
  static Measure estimate(double x, double se) {
    Measure m;
    m.kind = Kind::estimate, m.value = x, m.error = se;
    return m;
  }
};

inline std::string number(double x) {
  char buf[32];
  std::snprintf(buf, sizeof buf, "%.4g", x);
  return buf;
}

inline std::string words(const Measure& m) {
  switch (m.kind) {
    case Measure::Kind::probability: return "probability " + number(m.value);
    case Measure::Kind::interval: return "probability in [" + number(m.lower) + ", " + number(m.upper) + "]";
    case Measure::Kind::estimate: return number(m.value) + " (standard error " + number(m.error) + ")";
    default: return "no measure";
  }
}

struct Provenance {
  std::string method, wraps;
  Family family = Family::declared;
  char grade = 'A';                     // A a structured source, B a text with its passage, C a model's word from memory
  std::vector<std::string> rests_on;    // the findings and traces it rests on
};

// A dual verdict over a value of any type.
template <class T>
struct Finding {
  T value{};
  Standing standing = Standing::undecided;
  Measure measure;
  Provenance provenance;
};

// How standing and measure combine. A standard of standing asks a least standing (clear, or at least arguable); a standard
// of probability asks a probability above a threshold (for an interval, its lower end above it to hold and its upper end at
// or below it to fail); a standard of both asks both.
struct Standard {
  enum class Kind { standing, probability, both };
  Kind kind = Kind::both;
  double threshold = 0.5;
  Standing least = Standing::clear;
};

namespace detail {

inline Status by_standing(Standing s, Standing least, std::string& why) {
  if (s == Standing::undecided) return why = "its standing is not decided", Status::undecided;
  if (s == Standing::untenable) return why = "it is untenable", Status::fails;
  if (s == Standing::clear) return why = "it is clear", Status::holds;
  // arguable
  if (least == Standing::arguable) return why = "it is arguable, and the standard asks no more", Status::holds;
  return why = "it is only arguable", Status::fails;
}

inline Status by_measure(const Measure& m, double t, std::string& why) {
  switch (m.kind) {
    case Measure::Kind::probability:
      if (m.value > t) return why = words(m) + " exceeds " + number(t), Status::holds;
      return why = words(m) + " does not exceed " + number(t), Status::fails;
    case Measure::Kind::interval:
      if (m.lower > t) return why = words(m) + " lies above " + number(t), Status::holds;
      if (m.upper <= t) return why = words(m) + " lies at or below " + number(t), Status::fails;
      return why = words(m) + " straddles " + number(t), Status::undecided;
    default: return why = "it has no probability", Status::undecided;
  }
}

}  // namespace detail

// The verdict of a standard on a finding's standing and measure. A proposal is never the record's: it is undecided by every
// standard, with the reason said.
inline Verdict judge(const Standard& st, Standing s, const Measure& m, const std::string& claim, Family family = Family::declared) {
  if (family == Family::proposal) return Verdict::unknown(claim, "a proposal is shown as proposed and is never the record's conclusion");
  std::string ws, wm;
  const Status a = detail::by_standing(s, st.least, ws), b = detail::by_measure(m, st.threshold, wm);
  if (st.kind == Standard::Kind::standing) return a == Status::holds ? Verdict::yes(claim, ws) : a == Status::fails ? Verdict::no(claim, ws) : Verdict::unknown(claim, ws);
  if (st.kind == Standard::Kind::probability) return b == Status::holds ? Verdict::yes(claim, wm) : b == Status::fails ? Verdict::no(claim, wm) : Verdict::unknown(claim, wm);
  const std::string both = ws + "; " + wm;
  if (a == Status::fails || b == Status::fails) return Verdict::no(claim, both);
  if (a == Status::holds && b == Status::holds) return Verdict::yes(claim, both);
  return Verdict::unknown(claim, both);
}

template <class T>
Verdict judge(const Standard& st, const Finding<T>& f, const std::string& claim) {
  return judge(st, f.standing, f.measure, claim, f.provenance.family);
}

// A named check and its verdict, as every part of kybernesis reports them.
struct Check {
  std::string name;
  Verdict verdict;
  std::string kind;  // "constraint", "category", "law", "contract"
};

// The verdict of several checks together: fails when any fails, holds when all hold, undecided otherwise.
inline Verdict all_of(const std::vector<Check>& checks, const std::string& claim) {
  std::size_t held = 0;
  for (const auto& c : checks) {
    if (c.verdict.fails()) return Verdict::no(claim, c.name + ": " + c.verdict.reason, checks.size()).with("at", c.name);
    held += c.verdict.holds();
  }
  if (held == checks.size()) return Verdict::yes(claim, std::to_string(held) + " checks hold", checks.size());
  for (const auto& c : checks)
    if (c.verdict.undecided()) return Verdict::unknown(claim, c.name + ": " + c.verdict.reason, checks.size()).with("at", c.name);
  return Verdict::unknown(claim, "undecided", checks.size());
}

}  // namespace kybernesis::kernel
