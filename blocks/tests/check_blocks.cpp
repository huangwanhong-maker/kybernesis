// Checks of admission: a method is put into a block when it meets the block's contract and laws, decided on an instance.
//
// The standard architecture, on a small instance (the district of the idea's schema: dry or flooded, calm or alarm, a belief
// the beliefs over the two states). Methods are written here as plain functions, so that admission is checked apart from the
// predefined methods:
// - the exact possibilistic filter is admitted to Estimation: total, and sound in the world given as relations (kategoria
//   decides member ; step = member ; step ; member in the model of the world and the tabulation);
// - the estimator that takes a trace at its word is refused: not sound, at the input and output named;
// - a deterministic evaluation is admitted; one that answers a belief two ways is refused by the law deterministic;
// - a method with other ports, a method that throws, and a method of the record put into a referred block are refused by
//   their contract;
// - after a revision that changes Evaluation's ports, the evaluation carried over is refused; after one that adds a block,
//   it is still admitted.

#include <kybernesis/architecture/language.hpp>
#include <kybernesis/blocks/admission.hpp>

#include <calx/verification/check.hpp>

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <string>

using namespace kybernesis;
using namespace kybernesis::blocks;

namespace {

std::string slurp(const std::string& path) {
  std::ifstream in(path);
  std::stringstream s;
  s << in.rdbuf();
  return s.str();
}

Instance district() {
  Instance in;
  in.name = "district";
  in.types = {{"State", {{"dry", "flooded"}, ""}}, {"Belief", {{}, "State"}}, {"Traces", {{"calm", "alarm"}, ""}}, {"Words", {{"gauge"}, ""}},
              {"Terms", {{"flooded"}, ""}}, {"Readings", {{"strict"}, ""}}, {"Operators", {{"exposure"}, ""}}, {"Standards", {{"half"}, ""}},
              {"Rules", {{"council"}, ""}}, {"Proposals", {{"none"}, ""}}, {"Practice", {{"none"}, ""}}, {"Evaluations", {{"none", "concern"}, ""}},
              {"PolicyVersion", {{"v1"}, ""}}, {"Action", {{"wait", "protect"}, ""}}, {"Inquiry", {{"routine"}, ""}}};
  return in;
}

// The world of the idea's schema: waiting, a dry district may flood; protecting, a flooded one may dry; an alarm may be false.
World relations() {
  World w;
  w.state = "State", w.belief = "Belief", w.traces = "Traces", w.action = "Action";
  w.dynamics = [](std::size_t s, std::size_t a) {
    if (s == 0 && a == 0) return kernel::Possible{1, 1};
    if (s == 0) return kernel::Possible{1, 0};
    if (a == 0) return kernel::Possible{0, 1};
    return kernel::Possible{1, 1};
  };
  w.observation = [](std::size_t s) { return s == 0 ? kernel::Possible{1, 1} : kernel::Possible{0, 1}; };
  return w;
}

const kernel::Check* law(const Admission& ad, const std::string& prefix) {
  for (const auto& c : ad.checks)
    if (c.name.find(prefix) == 0) return &c;
  return nullptr;
}

}  // namespace

int main() {
  calx::verification::Suite s;
  const auto a = architecture::read(slurp(std::string(KYBERNESIS_DATA) + "/architectures/standard.arch")).architecture;
  const Instance in = district();
  const World w = relations();
  const std::size_t traces = 0, prior = 2, action = 3;  // Estimation's inputs: traces, readings, prior, action

  const Method kept = for_block(a, "Estimation", "possibilities kept", kernel::Family::formal, "this check", [&](const std::vector<Value>& v) {
    kernel::Possible next(2, 0);
    for (std::size_t st = 0; st < 2; st++)
      if (v[prior].law[st] > 0) {
        const auto after = w.dynamics(st, v[action].label);
        for (std::size_t s2 = 0; s2 < 2; s2++)
          if (after[s2] && w.observation(s2)[v[traces].label]) next[s2] = 1;
      }
    std::size_t n = next[0] + next[1];
    kernel::Distribution law(2, 0.0);
    for (std::size_t st = 0; st < 2; st++) law[st] = n ? next[st] / static_cast<double>(n) : 0.5;
    return std::vector<Value>{Value::belief(law)};
  });
  const Admission ak = admit(a, kept, in, &w);
  s.require("the exact possibilistic filter is admitted to Estimation", ak.verdict.holds(), ak.verdict.reason);
  s.require("its soundness is decided by kategoria in the model of the world", law(ak, "the law sound") && law(ak, "the law sound")->verdict.holds(),
            law(ak, "the law sound") ? law(ak, "the law sound")->verdict.reason : "");

  const Method guess = for_block(a, "Estimation", "face value", kernel::Family::declared, "this check", [&](const std::vector<Value>& v) {
    return std::vector<Value>{Value::belief(kernel::dirac(2, v[traces].label == 0 ? 0 : 1))};
  });
  const Admission ag = admit(a, guess, in, &w);
  s.require("the estimator that takes a trace at its word is refused: not sound", ag.verdict.fails() && law(ag, "the law sound")->verdict.fails(), ag.verdict.reason);
  s.require("without a world, soundness is undecided and the admission is too", admit(a, kept, in).verdict.undecided());

  const std::size_t belief = 0;  // Evaluation's inputs: belief, readings, operators, standards
  const Method concern = for_block(a, "Evaluation", "concern above a half", kernel::Family::statistical, "this check", [&](const std::vector<Value>& v) {
    return std::vector<Value>{Value::certain(v[belief].law[1] > 0.5 ? 1 : 0)};
  });
  const Admission ac = admit(a, concern, in, &w);
  s.require("a deterministic evaluation is admitted to Evaluation (total and deterministic)", ac.verdict.holds() && law(ac, "the law deterministic")->verdict.holds(), ac.verdict.reason);
  const Method coin = for_block(a, "Evaluation", "a coin", kernel::Family::statistical, "this check", [&](const std::vector<Value>& v) {
    return std::vector<Value>{v[belief].law[0] > 0 && v[belief].law[1] > 0 ? Value::drawn({0.5, 0.5}) : Value::certain(0)};
  });
  const Admission acn = admit(a, coin, in, &w);
  s.require("an evaluation that answers a belief two ways is refused by the law deterministic", acn.verdict.fails() && law(acn, "the law deterministic")->verdict.fails(), acn.verdict.reason);

  Method other = concern;
  other.inputs.pop_back();
  s.require("a method with other ports than its block's is refused", admit(a, other, in).verdict.fails());
  const Method throws = for_block(a, "Evaluation", "a method that throws", kernel::Family::statistical, "this check", [](const std::vector<Value>&) -> std::vector<Value> {
    throw std::runtime_error("not implemented");
  });
  const Admission at = admit(a, throws, in);
  s.require("a method that throws on an input is refused, the input named", at.verdict.fails() && at.verdict.reason.find("throws: not implemented") != std::string::npos, at.verdict.reason);
  const Method record = for_block(a, "Realisation", "a table", kernel::Family::declared, "this check", [](const std::vector<Value>& v) { return std::vector<Value>{v[0]}; });
  s.require("a method of the record put into a referred block is refused", admit(a, record, in).verdict.fails());
  Method worldly = record;
  worldly.family = kernel::Family::world;
  s.require("a method of the world fills it", admit(a, worldly, in).verdict.holds(), admit(a, worldly, in).verdict.reason);

  {
    architecture::Architecture changed = a;
    changed.version = 2;
    changed.block("Evaluation")->inputs.pop_back();  // standards no longer an input
    changed.wires.erase(std::remove_if(changed.wires.begin(), changed.wires.end(), [](const architecture::Wire& x) { return x.to == "Evaluation" && x.in == "standards"; }),
                        changed.wires.end());
    const auto carried = carry({concern}, changed, in, &w);
    s.require("after a revision that changes its block's ports, the evaluation carried over is refused", carried[0].verdict.fails(), carried[0].verdict.reason);
    architecture::Architecture grown = a;
    grown.version = 2;
    grown.blocks.push_back({"Transfer", "Policy", "recorded", false, {"total"}, {{"practice", "Practice"}}, {{"carried", "Practice"}}});
    for (auto& x : grown.wires)
      if (x.from == "Practice") x.to = "Transfer", x.in = "practice";
    grown.wires.push_back({"Transfer", "carried", "PolicyMaking", "practice", false});
    s.require("after a revision that adds a block elsewhere, it is still admitted", carry({concern}, grown, in, &w)[0].verdict.holds());
  }
  {
    const std::string entry = json_entry(ak);
    s.require("an admission is written as an entry of the report", entry.find("\"block\": \"Estimation\"") != std::string::npos && entry.find("\"status\": \"holds\"") != std::string::npos);
  }
  return s.done();
}
