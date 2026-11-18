// Doctrines on finite spaces: what an estimate is, and how estimates of different kinds are compared (decision 0002).
//
// A doctrine is a kind of uncertainty, a monad T on spaces: a point (the identity), a set of possible states (the non-empty
// powerset; the Kleisli category is that of total relations), a law (finite distributions; Giry's monad restricted to finite
// sets). On finite spaces they are represented here directly:
//
//   Point          an index;
//   Possible       a characteristic vector over the points;
//   Distribution   a law, a vector summing to 1.
//
// Estimators of different doctrines are compared through monad morphisms: the Dirac law of a point, δ : Id ⇒ D, and the
// support of a law, supp : D ⇒ P. supp is a monad morphism on finite distributions: the support of a mixture is the union of
// the supports of the components of positive weight. By naturality, for every map f between finite spaces,
//
//   supp ∘ D f = P f ∘ supp,
//
// so computing a feature and then coarsening a law agrees with coarsening and then computing; `natural` checks it on a law.
// The conformance of methods (blocks/admission.hpp) is decided in the possibilistic shadow of their laws, through supp, which is
// sound because supp is a monad morphism: composites of supports are supports of composites.
#pragma once

#include <kybernesis/kernel/verdicts.hpp>

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace kybernesis::kernel {

using Distribution = std::vector<double>;
using Possible = std::vector<char>;

inline void check_law(const Distribution& p, double tol = 1e-9) {
  double sum = 0;
  for (double x : p) {
    if (!(x >= -tol) || !std::isfinite(x)) throw std::invalid_argument("kybernesis::kernel: a law has a negative or undefined entry");
    sum += x;
  }
  if (std::fabs(sum - 1) > tol * (1 + p.size())) throw std::invalid_argument("kybernesis::kernel: a law sums to 1");
}

// δ: the law of a point.
inline Distribution dirac(std::size_t n, std::size_t i) {
  if (i >= n) throw std::invalid_argument("kybernesis::kernel: a point outside its space");
  Distribution d(n, 0.0);
  d[i] = 1;
  return d;
}

// supp: the points of positive probability.
inline Possible support(const Distribution& p, double tol = 0) {
  Possible s(p.size(), 0);
  for (std::size_t i = 0; i < p.size(); i++) s[i] = p[i] > tol;
  return s;
}

// D f: the law pushed forward along f : [n] -> [m].
inline Distribution push(const std::vector<std::size_t>& f, std::size_t m, const Distribution& p) {
  if (f.size() != p.size()) throw std::invalid_argument("kybernesis::kernel: a map and a law over different spaces");
  Distribution q(m, 0.0);
  for (std::size_t i = 0; i < p.size(); i++) {
    if (f[i] >= m) throw std::invalid_argument("kybernesis::kernel: a map leaves its target");
    q[f[i]] += p[i];
  }
  return q;
}

// P f: the image of a set of possible points.
inline Possible push(const std::vector<std::size_t>& f, std::size_t m, const Possible& s) {
  if (f.size() != s.size()) throw std::invalid_argument("kybernesis::kernel: a map and a set over different spaces");
  Possible q(m, 0);
  for (std::size_t i = 0; i < s.size(); i++)
    if (s[i]) q[f[i]] = 1;
  return q;
}

// The probability of a set of points under a law.
inline double probability(const Distribution& p, const Possible& set) {
  double x = 0;
  for (std::size_t i = 0; i < p.size(); i++)
    if (set[i]) x += p[i];
  return x;
}

// Naturality of supp at f, on one law: supp(D f (p)) = P f (supp p).
inline Verdict natural(const std::vector<std::size_t>& f, std::size_t m, const Distribution& p) {
  const Possible a = support(push(f, m, p)), b = push(f, m, support(p));
  for (std::size_t i = 0; i < m; i++)
    if (a[i] != b[i]) return Verdict::no("the support commutes with the map", "they differ at point " + std::to_string(i));
  return Verdict::yes("the support commutes with the map", "supp ∘ D f = P f ∘ supp on this law", m);
}

}  // namespace kybernesis::kernel
