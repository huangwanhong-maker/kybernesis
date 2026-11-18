// Checks of transfer by a lens, on an invented example of leaving a hazard's reach.
//
// Each system's state is (refuges within reach or far, people heed a warning or ignore it, most or few reached safety). In
// the source, preparing brings most to safety exactly when refuges are within reach and people heed; otherwise few. In the
// target the same holds within reach (most with 0.9 when people heed), but where refuges are far preparing still brings some
// to safety (people find high ground of their own), which the source's mechanism never does. So the square commutes, within a
// tolerance of 0.15, exactly where refuges are within reach: "within reach" is the support factor, sufficient and necessary;
// heeding is neither. Whether the target meets it turns on the reading of "within reach": within 15 minutes the target is
// believed to be within reach, within 10 minutes it may not be. Everything here is invented and describes no place.

#include <kybernesis/transfer/transfer.hpp>

#include <calx/verification/check.hpp>

#include <string>
#include <vector>

using namespace kybernesis;
using namespace kybernesis::transfer;

namespace {

// i = r·4 + h·2 + o: r 0 reach, 1 far; h 0 heed, 1 ignore; o 0 most, 1 few.
std::vector<std::string> states() {
  std::vector<std::string> s;
  for (const char* r : {"reach", "far"})
    for (const char* h : {"heed", "ignore"})
      for (const char* o : {"most", "few"}) s.push_back(std::string(r) + "·" + h + "·" + o);
  return s;
}

std::size_t at(int r, int h, int o) { return static_cast<std::size_t>(r * 4 + h * 2 + o); }

// The next state keeps the conditions; the outcome is most with probability p.
Distribution outcome(int r, int h, double p) {
  Distribution d(8, 0.0);
  d[at(r, h, 0)] = p, d[at(r, h, 1)] = 1 - p;
  return d;
}

System source() {
  System s{"the source", states(), {"prepare", "none"}, {}};
  for (int r = 0; r < 2; r++)
    for (int h = 0; h < 2; h++)
      for (int o = 0; o < 2; o++) s.dynamics.push_back({outcome(r, h, r == 0 && h == 0 ? 1.0 : 0.0), outcome(r, h, 0.0)});
  return s;
}

System target() {
  System s{"the target", states(), {"prepare", "none"}, {}};
  for (int r = 0; r < 2; r++)
    for (int h = 0; h < 2; h++)
      for (int o = 0; o < 2; o++) {
        const double p = r == 0 ? (h == 0 ? 0.9 : 0.0) : (h == 0 ? 0.5 : 0.2);
        s.dynamics.push_back({outcome(r, h, p), outcome(r, h, 0.0)});
      }
  return s;
}

Lens identity() {
  Lens l;
  for (std::size_t s = 0; s < 8; s++) {
    Distribution d(8, 0.0);
    d[s] = 1;
    l.interpret.push_back(d);
  }
  l.realise = {0, 1};
  return l;
}

Distribution belief(double reach, double heed) {
  Distribution b(8, 0.0);
  b[at(0, 0, 1)] = reach * heed, b[at(0, 1, 1)] = reach * (1 - heed), b[at(1, 0, 1)] = (1 - reach) * heed, b[at(1, 1, 1)] = (1 - reach) * (1 - heed);
  return b;
}

std::vector<char> where(int which, int value) {
  std::vector<char> s(8, 0);
  for (int r = 0; r < 2; r++)
    for (int h = 0; h < 2; h++)
      for (int o = 0; o < 2; o++) s[at(r, h, o)] = (which == 0 ? r : h) == value;
  return s;
}

}  // namespace

int main() {
  calx::verification::Suite s;
  const System J = source(), X = target();
  const Lens l = identity();

  s.require("both systems and the lens are well formed", well_formed(J).holds() && well_formed(X).holds() && well_formed(l, J, X).holds());
  {
    System bad = J;
    bad.dynamics[0][0][0] = 0.5;
    Lens wrong = l;
    wrong.realise = {0, 5};
    Lens short_ = l;
    short_.interpret.pop_back();
    s.require("a next state that is no law, an action realised by none, a state not read: each refused, named",
              well_formed(bad).fails() && well_formed(wrong, J, X).fails() && well_formed(short_, J, X).fails(),
              well_formed(bad).reason + " | " + well_formed(wrong, J, X).reason + " | " + well_formed(short_, J, X).reason);
  }
  {
    const Distribution b = belief(0.5, 0.6);
    const Distribution p = push(l, b, 8);
    bool same = true;
    for (std::size_t i = 0; i < 8; i++) same = same && std::abs(p[i] - b[i]) < 1e-15;
    Lens collapse = l;
    for (auto& row : collapse.interpret) row.assign(8, 0.0), row[0] = 1;
    const Distribution q = push(collapse, b, 8);
    s.require("pushing a belief along the identity leaves it, along a constant lens gathers it on one state", same && std::abs(q[0] - 1) < 1e-15);
  }
  {
    const Square q = square(J, X, l, "identity", {}, {}, 0.15);
    s.require("in laws the worst pair is preparing where refuges are far and people heed, at 0.5",
              std::abs(q.worst - 0.5) < 1e-12 && q.pairs.front().action == 0 && (q.pairs.front().state == at(1, 0, 0) || q.pairs.front().state == at(1, 0, 1)), kernel::number(q.worst));
    bool exact = true;
    for (std::size_t i = 0; i < 8; i++) exact = exact && (q.commutes[i] != 0) == (i < 4);
    s.require("within 0.15 the square commutes exactly where refuges are within reach", exact);
    s.require("in relations the square fails, kategoria naming where the target reaches what the source does not", q.relations.fails() && !q.beyond.empty() && q.short_.empty(),
              q.relations.reason + "; " + (q.beyond.empty() ? "" : q.beyond.front()));
    const Square only = square(J, X, l, "within reach, ignoring", where(1, 1) /* ignore */, {}, 0.0);
    std::vector<char> reach_ignore(8, 0);
    reach_ignore[at(0, 1, 0)] = reach_ignore[at(0, 1, 1)] = 1;
    const Square narrow = square(J, X, l, "within reach, ignoring", reach_ignore, {}, 0.0);
    s.require("restricted to where refuges are within reach and people ignore the warning, the square holds in relations", narrow.relations.holds() && only.relations.fails(),
              narrow.relations.reason);
    const Square self = square(J, J, l, "the source onto itself", {}, {}, 0.0);
    s.require("a system onto itself by the identity commutes in relations and in laws", self.relations.holds() && self.worst == 0.0);
  }
  {
    const std::vector<Reading> readings{{"within 15 minutes", "convention", l, belief(1.0, 0.6)}, {"within 10 minutes", "analyst", l, belief(0.5, 0.6)}};
    const std::vector<Factor> factors{{"refuges within reach", "computed", where(0, 0)}, {"people heed a warning", "declared", where(1, 0)}};
    const Assessment a = assess(J, X, readings, factors, {}, 0.15);
    s.require("the assessment is made", a.verdict.holds(), a.verdict.reason);
    s.require("it will work here is arguable, shown within 15 minutes and open within 10",
              a.carries.standing == kernel::Standing::arguable && a.by_reading[0].find("shown") != std::string::npos && a.by_reading[1].find("open") != std::string::npos,
              a.by_reading[0] + " | " + a.by_reading[1]);
    s.require("its measure is the interval [0.5, 1] of the probability that the square commutes where the target is",
              a.carries.measure.kind == kernel::Measure::Kind::interval && std::abs(a.carries.measure.lower - 0.5) < 1e-12 && std::abs(a.carries.measure.upper - 1) < 1e-12,
              kernel::words(a.carries.measure));
    s.require("refuges within reach is the support factor: sufficient and necessary, arguable for the target, probability in [0.5, 1]",
              a.factors[0].sufficient && a.factors[0].necessary && a.factors[0].standing == kernel::Standing::arguable && std::abs(a.factors[0].measure.lower - 0.5) < 1e-12,
              a.factors[0].words);
    s.require("heeding a warning is neither sufficient nor necessary for the square, and its probability is 0.6 under both readings",
              !a.factors[1].sufficient && !a.factors[1].necessary && std::abs(a.factors[1].measure.lower - 0.6) < 1e-12 && std::abs(a.factors[1].measure.upper - 0.6) < 1e-12,
              a.factors[1].words);
    const Assessment strict = assess(J, X, readings, factors, {}, 0.05);
    s.require("with a tolerance of 0.05 the target's 0.9 no longer passes for the source's certainty: within reach is no longer sufficient", !strict.factors[0].sufficient);
    const Assessment both = assess(J, X, {readings[0]}, factors, {}, 0.15);
    s.require("under the reading of 15 minutes alone it will work here is clear, with probability 1", both.carries.standing == kernel::Standing::clear && both.carries.value);
    const Assessment far = assess(J, X, {{"far", "analyst", l, belief(0.0, 0.6)}}, factors, {}, 0.15);
    s.require("a target believed far from refuges under every reading: untenable", far.carries.standing == kernel::Standing::untenable);
    std::vector<Reading> broken = readings;
    broken[1].belief[0] = 0.9;
    s.require("a reading whose belief is no law is refused before anything is assessed", assess(J, X, broken, factors).verdict.fails());
  }
  {
    auto policy = [](const Distribution& b) { return b[at(0, 0, 0)] + b[at(0, 0, 1)] >= 0.5 ? std::size_t{0} : std::size_t{1}; };
    s.require("the carried policy prepares in the target when the source's policy would, read through the lens",
              carry(policy, l, belief(1.0, 0.6), 8) == 0 && carry(policy, l, belief(0.5, 0.6), 8) == 1);
  }
  return s.done();
}
