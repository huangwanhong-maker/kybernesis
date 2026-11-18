// Checks of the loop run, on the invented riverside example and the standard architecture.
//
// - 300 turns with the Bayes filter and the threshold rule: every turn run; the true state (the system's, never in the record)
//   always within the support of the belief estimated from what was observed, as soundness promises; the policy protects at
//   least once and waits at least once.
// - The same with the possibilistic filter: the truth is never lost. With the face-value estimator, which admission refuses, it
//   is lost on some turn: the refusal was not a formality.
// - With the readings both admitted when residents report, evaluations come with every standing (clear, arguable, untenable
//   all occur) and with intervals of probability: the dual verdict along the run.
// - A run is the same when repeated with its seed.
// - A revision that adds a block of transfer runs once its methods are carried over and the new block filled; an architecture
//   that is not admitted, or a block left without a method, is refused before any turn.

#include <kybernesis/architecture/language.hpp>
#include <kybernesis/loop/run.hpp>
#include <kybernesis/methods/example.hpp>

#include <calx/verification/check.hpp>

#include <fstream>
#include <map>
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

std::map<std::string, blocks::Value> initial(const Instance& in) {
  return {{"Estimation.belief -> Estimation.prior next", blocks::Value::belief({1.0 / 3, 1.0 / 3, 1.0 / 3})},
          {"Policy.current -> PolicyMaking.current next", blocks::Value::certain(in.index("PolicyVersion", "cautious"))},
          {"Decision.intervention -> Estimation.action next", blocks::Value::certain(in.index("Action", "wait"))},
          {"Decision.inquiry -> Collection.inquiry next", blocks::Value::certain(in.index("Inquiry", "routine"))},
          {"Realisation.state -> System.state next", blocks::Value::certain(in.index("State", "dry"))}};
}

// Turns on which the true state lies outside the support of the belief.
std::size_t lost(const loop::Run& r, const architecture::Architecture& a) {
  std::size_t n = 0;
  for (std::size_t t = 0; t < r.turns.size(); t++) {
    const auto truth = r.value(t, a, "System", "state").label;
    n += !(r.value(t, a, "Estimation", "belief").law[truth] > 0);
  }
  return n;
}

}  // namespace

int main() {
  calx::verification::Suite s;
  const auto a = architecture::read(slurp(std::string(KYBERNESIS_DATA) + "/architectures/standard.arch")).architecture;
  const Instance in = example::riverside();

  const loop::Run bayes = loop::run(a, example::assignment(a), in, initial(in), 300, 20261118);
  s.require("300 turns with the Bayes filter and the threshold rule are run", bayes.verdict.holds() && bayes.turns.size() == 300, bayes.verdict.reason);
  s.require("the true state is never outside the belief estimated from what was observed", lost(bayes, a) == 0, std::to_string(lost(bayes, a)) + " turns lost it");
  {
    std::size_t protect = 0;
    for (std::size_t t = 0; t < bayes.turns.size(); t++) protect += bayes.value(t, a, "Decision", "intervention").label == in.index("Action", "protect");
    s.require("the policy protects on some turns and waits on others", protect > 0 && protect < bayes.turns.size(), std::to_string(protect) + " turns of 300 protected");
  }
  const loop::Run again = loop::run(a, example::assignment(a), in, initial(in), 300, 20261118);
  bool same = true;
  for (std::size_t t = 0; t < 300; t++) same = same && again.value(t, a, "System", "state").label == bayes.value(t, a, "System", "state").label;
  s.require("a run is the same when repeated with its seed", same);

  const loop::Run poss = loop::run(a, example::assignment(a, "possibility filter"), in, initial(in), 300, 20261118);
  s.require("with the possibilistic filter the truth is never lost", poss.verdict.holds() && lost(poss, a) == 0);
  const loop::Run face = loop::run(a, example::assignment(a, "face value"), in, initial(in), 300, 20261118);
  s.require("with the face-value estimator, which admission refuses, the truth is lost on some turns", face.verdict.holds() && lost(face, a) > 0,
            std::to_string(lost(face, a)) + " turns of 300 lost it");

  {
    auto methods = example::assignment(a);
    methods["Interpretation"] = table(a, in, "Interpretation", "both when residents report", "words", "readings", {{"gauge", "strict"}, {"report", "both"}});
    const loop::Run r = loop::run(a, methods, in, initial(in), 300, 7);
    std::size_t clear = 0, arguable = 0, untenable = 0, intervals = 0;
    for (std::size_t t = 0; t < r.turns.size(); t++) {
      const auto& e = r.value(t, a, "Evaluation", "evaluations");
      clear += e.standing == kernel::Standing::clear, arguable += e.standing == kernel::Standing::arguable, untenable += e.standing == kernel::Standing::untenable;
      intervals += e.measure.kind == kernel::Measure::Kind::interval;
    }
    s.require("evaluations along the run come with every standing and with intervals of probability",
              r.verdict.holds() && clear > 0 && arguable > 0 && untenable > 0 && intervals > 0,
              std::to_string(clear) + " clear, " + std::to_string(arguable) + " arguable, " + std::to_string(untenable) + " untenable, " + std::to_string(intervals) + " intervals");
  }

  {
    architecture::Architecture grown = a;
    grown.version = 2;
    grown.blocks.push_back({"Transfer", "Policy", "recorded", false, {"total"}, {{"practice", "Practice"}}, {{"carried", "Practice"}}});
    for (auto& w : grown.wires)
      if (w.from == "Practice") w.to = "Transfer", w.in = "practice";
    grown.wires.push_back({"Transfer", "carried", "PolicyMaking", "practice", false});
    auto methods = example::assignment(a);
    std::vector<blocks::Method> list;
    for (const auto& [k, m] : methods) list.push_back(m);
    const auto w = example::world();
    const auto carried = blocks::carry(list, grown, in, &w);
    bool all = true;
    for (const auto& c : carried) all = all && c.verdict.holds();
    methods["Transfer"] = identity(grown, "Transfer", {{"carried", "practice"}});
    const auto ad = blocks::admit(grown, methods["Transfer"], in, &w);
    const loop::Run r = loop::run(grown, methods, in, initial(in), 50, 3);
    s.require("a revision adding a block of transfer runs once its methods are carried over and the block filled", all && ad.verdict.holds() && r.verdict.holds() && r.order.size() == 14,
              r.verdict.reason);
  }
  {
    architecture::Architecture broken = a;
    broken.wires.pop_back();  // the system's next state fed by nothing
    s.require("an architecture that is not admitted is refused before any turn", loop::run(broken, example::assignment(a), in, initial(in), 5, 1).verdict.fails());
    auto methods = example::assignment(a);
    methods.erase("Evaluation");
    s.require("a block left without a method is refused before any turn", loop::run(a, methods, in, initial(in), 5, 1).verdict.fails());
  }
  return s.done();
}
