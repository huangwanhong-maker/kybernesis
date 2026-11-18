// Checks of the predefined methods, on the invented riverside example (methods/example.hpp) and the standard architecture.
//
// - Every block of the standard architecture is filled by a predefined method, and every one is admitted: the boundaries'
//   constants, the world's methods in the referred blocks, the mechanism, the archive, the interpretation, the Bayes filter
//   (sound in the world of the POMDP's supports), the evaluation (deterministic), policy making, the threshold rule, the decision.
// - Among the alternatives: the possibilistic filter is admitted, the face-value estimator refused as unsound; construal and
//   precedents are admitted to Interpretation; HSVI is admitted to Policy.
// - What the methods answer, worked by hand: construal sets aside the conventional reading for the authority's; precedents
//   after 3 strict and 7 loose readings under Dirichlet(1) give 4/12 and 8/12; exceedance under both readings is arguable with
//   the interval of its probabilities; the Bayes filter is held to Bayes's rule computed here; HSVI protects a deep district.
//
// Run with --report, it prints the report of the architecture and every admission as JSON (architecture/report.hpp), as the
// visualization layer's computational graph reads it.

#include <kybernesis/architecture/checks.hpp>
#include <kybernesis/architecture/language.hpp>
#include <kybernesis/architecture/report.hpp>
#include <kybernesis/blocks/admission.hpp>
#include <kybernesis/methods/example.hpp>

#include <calx/verification/check.hpp>

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace kybernesis;
using namespace kybernesis::methods;

namespace {

std::string slurp(const std::string& path) {
  std::ifstream in(path);
  std::stringstream s;
  s << in.rdbuf();
  return s.str();
}

}  // namespace

int main(int argc, char** argv) {
  const auto a = architecture::read(slurp(std::string(KYBERNESIS_DATA) + "/architectures/standard.arch")).architecture;
  const Instance in = example::riverside();
  const auto w = example::world();

  std::vector<blocks::Admission> admissions;
  const auto chosen = example::assignment(a);
  for (const auto& b : a.blocks) admissions.push_back(blocks::admit(a, chosen.at(b.name), in, &w));
  std::vector<blocks::Method> alternatives{example::assignment(a, "possibility filter").at("Estimation"), example::assignment(a, "face value").at("Estimation"),
                                           example::assignment(a, "Bayes filter", "threshold rule", "construal").at("Interpretation"),
                                           example::assignment(a, "Bayes filter", "threshold rule", "precedents").at("Interpretation"),
                                           example::assignment(a, "Bayes filter", "HSVI").at("Policy")};
  std::vector<blocks::Admission> others;
  for (const auto& m : alternatives) others.push_back(blocks::admit(a, m, in, &w));

  if (argc > 1 && std::string(argv[1]) == "--report") {
    const auto v = architecture::verify(a);
    std::vector<std::string> entries;
    for (const auto& ad : admissions) entries.push_back(blocks::json_entry(ad));
    for (const auto& ad : others) entries.push_back(blocks::json_entry(ad));
    std::cout << architecture::report_json(a, v.checks, entries);
    return 0;
  }

  calx::verification::Suite s;
  {
    std::size_t held = 0;
    std::string first;
    for (const auto& ad : admissions) {
      held += ad.verdict.holds();
      if (!ad.verdict.holds() && first.empty()) first = ad.block + ": " + ad.verdict.reason;
    }
    s.require("every block of the standard architecture is filled by a predefined method, and all 13 are admitted", held == 13 && admissions.size() == 13, first);
  }
  s.require("the possibilistic filter is admitted to Estimation", others[0].verdict.holds(), others[0].verdict.reason);
  s.require("the face-value estimator is refused: it is not sound", others[1].verdict.fails() && others[1].verdict.reason.find("sound") != std::string::npos, others[1].verdict.reason);
  s.require("construal is admitted to Interpretation", others[2].verdict.holds(), others[2].verdict.reason);
  s.require("precedents are admitted to Interpretation", others[3].verdict.holds(), others[3].verdict.reason);
  s.require("HSVI is admitted to Policy", others[4].verdict.holds(), others[4].verdict.reason);

  {
    const auto c = alternatives[2].apply({blocks::Value::certain(0), blocks::Value::certain(0)});
    s.require("construal sets aside the conventional reading for the authority's: strict, and clear", c[0].label == in.index("Readings", "strict") && c[0].standing == kernel::Standing::clear);
    const auto p = alternatives[3].apply({blocks::Value::certain(0), blocks::Value::certain(0)});
    s.require("precedents after 3 strict and 7 loose readings under Dirichlet(1): 4/12 strict, 8/12 loose",
              std::fabs(p[0].law[0] - 4.0 / 12) < 1e-12 && std::fabs(p[0].law[1] - 8.0 / 12) < 1e-12 && p[0].law[2] == 0);
  }
  {
    const auto& ev = chosen.at("Evaluation");
    const auto both = ev.apply({blocks::Value::belief({0, 0.6, 0.4}), blocks::Value::certain(in.index("Readings", "both")), blocks::Value::certain(0), blocks::Value::certain(0)});
    s.require("exceedance under both readings, shallow 0.6 and deep 0.4: a concern, arguable, its probability in [0.4, 1]",
              both[0].label == in.index("Evaluations", "concern") && both[0].standing == kernel::Standing::arguable && both[0].measure.kind == kernel::Measure::Kind::interval &&
                  std::fabs(both[0].measure.lower - 0.4) < 1e-12 && std::fabs(both[0].measure.upper - 1) < 1e-12);
    const auto none = ev.apply({blocks::Value::belief({0.5, 0.5, 0}), blocks::Value::certain(in.index("Readings", "strict")), blocks::Value::certain(0), blocks::Value::certain(0)});
    s.require("under the strict reading, dry 0.5 and shallow 0.5: no concern, untenable, probability 0",
              none[0].label == in.index("Evaluations", "none") && none[0].standing == kernel::Standing::untenable && none[0].measure.value == 0);
  }
  {
    // Bayes's rule by hand: prior uniform, wait, alarm. Predicted: dry (0.8+0.2)/3, shallow (0.2+0.5+0.3)/3, deep (0.3+0.7)/3, all 1/3.
    // Alarm likelihoods 0.1, 0.6, 1.0: posterior proportional to 0.1, 0.6, 1.0, over 1.7.
    const auto& bf = chosen.at("Estimation");
    const auto b = bf.apply({blocks::Value::certain(1), blocks::Value::certain(0), blocks::Value::belief({1.0 / 3, 1.0 / 3, 1.0 / 3}), blocks::Value::certain(0)});
    s.require("the Bayes filter is Bayes's rule: after an alarm, waiting, from a uniform prior, (0.1, 0.6, 1.0)/1.7",
              std::fabs(b[0].law[0] - 0.1 / 1.7) < 1e-12 && std::fabs(b[0].law[1] - 0.6 / 1.7) < 1e-12 && std::fabs(b[0].law[2] - 1.0 / 1.7) < 1e-12 &&
                  std::fabs(b[0].measure.value - 1.7 / 3) < 1e-12);
    // In a variant where a dry district never sounds an alarm, an alarm after protecting a district believed dry is impossible.
    auto P2 = example::pomdp();
    for (std::size_t act = 0; act < 2; act++) P2.observe(act, 0, 0) = 1, P2.observe(act, 0, 1) = 0;
    const auto strict = bayes_filter(a, "Estimation", w, P2);
    const auto z = strict.apply({blocks::Value::certain(1), blocks::Value::certain(0), blocks::Value::belief({1, 0, 0}), blocks::Value::certain(1)});
    s.require("evidence of probability zero under the belief is a contradiction: the belief predicted, untenable",
              z[0].standing == kernel::Standing::untenable && z[0].law[0] == 1);
  }
  {
    const auto& hp = alternatives[4];
    const auto deep = hp.apply({blocks::Value::certain(0), blocks::Value::belief({0, 0, 1})});
    const auto dry = hp.apply({blocks::Value::certain(0), blocks::Value::belief({1, 0, 0})});
    s.require("HSVI protects a district believed deep and waits in one believed dry",
              deep[0].label == in.index("Action", "protect") && dry[0].label == in.index("Action", "wait"));
  }
  return s.done();
}
