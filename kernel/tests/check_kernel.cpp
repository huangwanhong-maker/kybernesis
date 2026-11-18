// Checks of the kernel: dual verdicts and standards, scales and meaningfulness, doctrines and the naturality of the support.
//
// - A standard of both holds only when standing and measure both hold; a proposal is undecided by every standard.
// - On an ordinal scale a comparison of means is refuted by an admissible transformation and a comparison of medians is not;
//   on an interval scale a ratio of values is refuted and a ratio of differences is not; on a nominal scale order is refuted.
// - supp ∘ D f = P f ∘ supp on random laws and maps (worked by construction: the identity holds for every law).

#include <kybernesis/kernel/doctrines.hpp>
#include <kybernesis/kernel/scales.hpp>
#include <kybernesis/kernel/verdicts.hpp>

#include <calx/random/stream.hpp>
#include <calx/verification/check.hpp>

#include <algorithm>
#include <numeric>
#include <vector>

using namespace kybernesis::kernel;

namespace {

double mean(const std::vector<double>& v, std::size_t a, std::size_t b) {
  double s = 0;
  for (std::size_t i = a; i < b; i++) s += v[i];
  return s / static_cast<double>(b - a);
}

double median(std::vector<double> v) {
  std::sort(v.begin(), v.end());
  const std::size_t n = v.size();
  return n % 2 ? v[n / 2] : (v[n / 2 - 1] + v[n / 2]) / 2;
}

}  // namespace

int main() {
  calx::verification::Suite s;

  {
    Standard both;  // probability above 0.5 and clear
    s.require("clear and probable holds by a standard of both", judge(both, Standing::clear, Measure::probability(0.8), "c").holds());
    s.require("arguable fails a standard asking clear", judge(both, Standing::arguable, Measure::probability(0.8), "c").fails());
    Standard lenient{Standard::Kind::standing, 0.5, Standing::arguable};
    s.require("arguable holds a standard asking only arguable", judge(lenient, Standing::arguable, Measure::nothing(), "c").holds());
    Standard prob{Standard::Kind::probability, 0.5, Standing::clear};
    s.require("an interval straddling the threshold is undecided", judge(prob, Standing::clear, Measure::interval(0.3, 0.7), "c").undecided());
    s.require("an interval above the threshold holds", judge(prob, Standing::undecided, Measure::interval(0.6, 0.9), "c").holds());
    s.require("an interval at or below the threshold fails", judge(prob, Standing::undecided, Measure::interval(0.1, 0.5), "c").fails());
    s.require("untenable fails a standard of both whatever the probability", judge(both, Standing::untenable, Measure::probability(0.99), "c").fails());
    s.require("a proposal is undecided by every standard, however clear and probable",
              judge(both, Standing::clear, Measure::probability(0.99), "c", Family::proposal).undecided() &&
                  judge(lenient, Standing::clear, Measure::nothing(), "c", Family::proposal).undecided());
    std::vector<Check> checks{{"a", Verdict::yes("a"), "law"}, {"b", Verdict::unknown("b", "not decided"), "law"}};
    s.require("checks together: undecided when one is undecided and none fails", all_of(checks, "x").undecided());
    checks.push_back({"c", Verdict::no("c", "broken"), "law"});
    s.require("checks together: fail when one fails, naming it", all_of(checks, "x").fails() && all_of(checks, "x").reason.find("c: broken") == 0);
  }

  {
    // Two groups of scores 1-5: A = {1, 1, 5}, B = {2, 2, 2}. Mean of A (2.33) exceeds B's (2); median of A (1) does not.
    const std::vector<double> scores{1, 1, 5, 2, 2, 2};
    auto means = [](const std::vector<double>& v) { return mean(v, 0, 3) > mean(v, 3, 6); };
    auto medians = [](const std::vector<double>& v) { return median({v[0], v[1], v[2]}) > median({v[3], v[4], v[5]}); };
    const Verdict m1 = meaningful(Scale::ordinal, scores, means, "that A's mean exceeds B's");
    s.require("on an ordinal scale a comparison of means is not meaningful", m1.fails(), m1.reason);
    s.require("on an ordinal scale a comparison of medians is meaningful", meaningful(Scale::ordinal, scores, medians, "that A's median exceeds B's").holds());
    s.require("on an interval scale a comparison of means is meaningful", meaningful(Scale::interval, scores, means, "that A's mean exceeds B's").holds());

    // a = 10, b = 5: "a is twice b" is a ratio of values; "a - b is twice b - c" (c = 2.5) a ratio of differences.
    const std::vector<double> v{10, 5, 2.5};
    auto twice = [](const std::vector<double>& x) { return std::fabs(x[0] - 2 * x[1]) < 1e-9; };
    auto diffs = [](const std::vector<double>& x) { return std::fabs((x[0] - x[1]) - 2 * (x[1] - x[2])) < 1e-9; };
    s.require("on an interval scale a ratio of values is not meaningful", meaningful(Scale::interval, v, twice, "that a is twice b").fails());
    s.require("on a ratio scale a ratio of values is meaningful", meaningful(Scale::ratio, v, twice, "that a is twice b").holds());
    s.require("on an interval scale a ratio of differences is meaningful", meaningful(Scale::interval, v, diffs, "that a - b is twice b - c").holds());

    const std::vector<double> labels{1, 2, 3};
    auto equal = [](const std::vector<double>& x) { return x[0] != x[1]; };
    auto order = [](const std::vector<double>& x) { return x[0] < x[1]; };
    s.require("on a nominal scale difference is meaningful", meaningful(Scale::nominal, labels, equal, "that the first two differ").holds());
    s.require("on a nominal scale order is not meaningful", meaningful(Scale::nominal, labels, order, "that the first is below the second").fails());
    s.require("the transformations tried are admissible on their scale",
              admissible(Scale::interval, v, [](double x) { return 2 * x + 5; }) && !admissible(Scale::ratio, v, [](double x) { return 2 * x + 5; }) &&
                  admissible(Scale::ordinal, v, [](double x) { return x * x * x; }) && !admissible(Scale::interval, v, [](double x) { return x * x * x; }));
  }

  {
    calx::random::Stream rng(20261118, 1, 0, 0);
    std::size_t natural_ok = 0, sums_ok = 0;
    const std::size_t trials = 200;
    for (std::size_t t = 0; t < trials; t++) {
      const std::size_t n = 2 + static_cast<std::size_t>(rng.below(6)), m = 1 + static_cast<std::size_t>(rng.below(5));
      Distribution p(n, 0.0);
      for (auto& x : p) x = rng.uniform() < 0.3 ? 0.0 : rng.uniform();
      double sum = std::accumulate(p.begin(), p.end(), 0.0);
      if (sum == 0) p[0] = sum = 1;
      for (auto& x : p) x /= sum;
      std::vector<std::size_t> f(n);
      for (auto& y : f) y = static_cast<std::size_t>(rng.below(m));
      natural_ok += natural(f, m, p).holds();
      const Distribution q = push(f, m, p);
      sums_ok += std::fabs(std::accumulate(q.begin(), q.end(), 0.0) - 1) < 1e-12;
    }
    s.require("the support commutes with every map, on 200 random laws (supp is natural)", natural_ok == trials);
    s.require("a law pushed forward is a law", sums_ok == trials);
    s.require("the support of a Dirac law is its point", support(dirac(4, 2)) == Possible{0, 0, 1, 0});
  }
  return s.done();
}
