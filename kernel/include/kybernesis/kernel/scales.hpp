// Scales and what may be said on them (decision 0002; Stevens 1946; Krantz, Luce, Suppes and Tversky 1971, ch. 1).
//
// An evaluation's values lie in a space with a scale, and the scale is a group of admissible transformations: the
// re-expressions that change nothing the values mean.
//
//   nominal    every bijection: only sameness and difference mean anything;
//   ordinal    the strictly increasing maps: order means something, distances do not;
//   interval   x -> a x + b with a > 0: order and ratios of differences;
//   ratio      x -> a x with a > 0: ratios of values as well;
//   absolute   the identity alone (counts, probabilities).
//
// A statement about values is meaningful exactly when its truth is the same under every admissible transformation (Suppes
// and Zinnes 1963). That the mean of an ordinal score is higher in one group than another is not meaningful; that the median
// is, is. The check here is a refutation search: it applies a family of admissible transformations chosen for the scale,
// deterministic, and reports the first under which the statement's truth changes. A statement it cannot refute is said to be
// meaningful as far as the family reaches, which is what the verdict says; the families are chosen to break the common
// errors (means and differences on ordinal scales, ratios on interval scales, anything but identity on nominal ones).
#pragma once

#include <kybernesis/kernel/verdicts.hpp>

#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace kybernesis::kernel {

enum class Scale { nominal, ordinal, interval, ratio, absolute };

inline const char* name(Scale s) {
  switch (s) {
    case Scale::nominal: return "nominal";
    case Scale::ordinal: return "ordinal";
    case Scale::interval: return "interval";
    case Scale::ratio: return "ratio";
    default: return "absolute";
  }
}

inline Scale scale_of(const std::string& s) {
  for (Scale x : {Scale::nominal, Scale::ordinal, Scale::interval, Scale::ratio, Scale::absolute})
    if (s == name(x)) return x;
  throw std::invalid_argument("kybernesis::kernel: no scale named " + s);
}

struct Transformation {
  std::string name;
  std::function<double(double)> map;
};

// Whether a map is admissible for a scale on the given values (checked on them, pairwise).
inline bool admissible(Scale s, const std::vector<double>& xs, const std::function<double(double)>& f, double tol = 1e-9) {
  for (std::size_t i = 0; i < xs.size(); i++)
    for (std::size_t j = 0; j < xs.size(); j++) {
      const double a = xs[i], b = xs[j], fa = f(a), fb = f(b);
      if (s == Scale::nominal && ((a == b) != (std::fabs(fa - fb) <= tol))) return false;
      if (s != Scale::nominal && a < b && !(fa < fb)) return false;
    }
  if (s == Scale::interval || s == Scale::ratio || s == Scale::absolute) {
    // affine: f(x) = a x + b, fitted on two distinct values and checked on all
    std::vector<double> d = xs;
    std::sort(d.begin(), d.end());
    d.erase(std::unique(d.begin(), d.end()), d.end());
    if (d.size() >= 2) {
      const double a = (f(d.back()) - f(d.front())) / (d.back() - d.front()), b = f(d.front()) - a * d.front();
      for (double x : d)
        if (std::fabs(f(x) - (a * x + b)) > tol * (1 + std::fabs(x))) return false;
      if (s == Scale::ratio && std::fabs(b) > tol) return false;
      if (s == Scale::absolute && (std::fabs(a - 1) > tol || std::fabs(b) > tol)) return false;
    }
  }
  return true;
}

// A deterministic family of admissible transformations for the scale on these values.
inline std::vector<Transformation> transformations(Scale s, const std::vector<double>& xs) {
  std::vector<Transformation> out;
  switch (s) {
    case Scale::absolute: break;
    case Scale::ratio:
      out.push_back({"x -> 3x", [](double x) { return 3 * x; }});
      out.push_back({"x -> x/4", [](double x) { return x / 4; }});
      break;
    case Scale::interval:
      out.push_back({"x -> 2x + 5", [](double x) { return 2 * x + 5; }});
      out.push_back({"x -> x/2 - 3", [](double x) { return x / 2 - 3; }});
      out.push_back({"x -> 10x - 100", [](double x) { return 10 * x - 100; }});
      break;
    case Scale::ordinal: {
      std::vector<double> d = xs;
      std::sort(d.begin(), d.end());
      d.erase(std::unique(d.begin(), d.end()), d.end());
      out.push_back({"x -> x^3", [](double x) { return x * x * x; }});
      out.push_back({"x -> e^x", [](double x) { return std::exp(x); }});
      out.push_back({"x -> -e^(-x)", [](double x) { return -std::exp(-x); }});
      // the top value stretched far above the rest, and the bottom far below
      if (d.size() >= 2) {
        const double top = d.back(), bottom = d.front();
        out.push_back({"the highest value raised by 1000", [top](double x) { return x >= top ? x + 1000 : x; }});
        out.push_back({"the lowest value lowered by 1000", [bottom](double x) { return x <= bottom ? x - 1000 : x; }});
      }
      break;
    }
    case Scale::nominal: {
      std::vector<double> d = xs;
      std::sort(d.begin(), d.end());
      d.erase(std::unique(d.begin(), d.end()), d.end());
      // the order of the values reversed: a bijection that is no monotone map
      out.push_back({"the values relabelled in reverse order", [d](double x) {
                       const auto it = std::lower_bound(d.begin(), d.end(), x);
                       if (it == d.end() || *it != x) return x;
                       return d[d.size() - 1 - static_cast<std::size_t>(it - d.begin())];
                     }});
      out.push_back({"x -> x^3", [](double x) { return x * x * x; }});
      break;
    }
  }
  return out;
}

// Whether a statement about values is meaningful on the scale: its truth unchanged by every transformation of the family.
inline Verdict meaningful(Scale s, const std::vector<double>& values, const std::function<bool(const std::vector<double>&)>& statement, const std::string& claim) {
  const bool truth = statement(values);
  const auto family = transformations(s, values);
  for (const auto& t : family) {
    if (!admissible(s, values, t.map)) continue;  // a member of the family not admissible on these values is skipped
    std::vector<double> y(values.size());
    for (std::size_t k = 0; k < values.size(); k++) y[k] = t.map(values[k]);
    if (statement(y) != truth)
      return Verdict::no(claim + " is meaningful on a " + name(s) + " scale",
                         "its truth changes under the admissible transformation " + t.name + " (" + (truth ? "true" : "false") + " before, " + (truth ? "false" : "true") + " after)", family.size())
          .with("at", t.name);
  }
  return Verdict::yes(claim + " is meaningful on a " + name(s) + " scale",
                      "its truth is unchanged under the " + std::to_string(family.size()) + " admissible transformations tried", family.size());
}

}  // namespace kybernesis::kernel
