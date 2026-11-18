// Checks of the architecture module: the language read and printed back, the standard architecture verified, faults found
// each by its own rule, and revisions admitted or refused.
//
// - The standard architecture (architectures/standard.arch) reads with no fault, with the counts of its declaration, and
//   prints back to itself.
// - It is admitted: every constraint holds, and kategoria answers every categorical claim: the registers, kinds and stages
//   thin, Fact terminal, each kind in its register functorially, and each block's stage a functor from the step category.
// - A statement that is wrong is reported with its line; a wire backwards within a turn passes every constraint and is refused
//   by kategoria alone (the functor StageOf does not exist); an example yielded by a block, an input fed by nothing, an unknown
//   law, a cycle of kinds, a block in the wrong register are each refused by the rule that names them.
// - A revision that adds a block between the transferred practice and policy making is admitted, its difference named; one
//   that removes interpretation without rewiring is refused; one that does not raise the version is refused.

#include <kybernesis/architecture/checks.hpp>
#include <kybernesis/architecture/language.hpp>
#include <kybernesis/architecture/report.hpp>
#include <kybernesis/architecture/revision.hpp>

#include <calx/verification/check.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

using namespace kybernesis::architecture;

namespace {

std::string slurp(const std::string& path) {
  std::ifstream in(path);
  std::stringstream s;
  s << in.rdbuf();
  return s.str();
}

const Check* named(const Verification& v, const std::string& name) {
  for (const auto& c : v.checks)
    if (c.name == name) return &c;
  return nullptr;
}

// The checks that fail, by name.
std::string failing(const Verification& v) {
  std::string s;
  for (const auto& c : v.checks)
    if (!c.verdict.holds()) s += (s.empty() ? "" : " | ") + c.name + " [" + std::string(kybernesis::kernel::word(c.verdict.status)) + "]: " + c.verdict.reason;
  return s;
}

// Exactly one constraint fails, and it is this one.
bool only_fails(const Verification& v, const std::string& name) {
  std::size_t fails = 0;
  bool it = false;
  for (const auto& c : v.checks)
    if (c.kind == "constraint" && c.verdict.fails()) fails++, it = it || c.name == name;
  return fails == 1 && it;
}

}  // namespace

int main() {
  calx::verification::Suite s;
  const std::string text = slurp(std::string(KYBERNESIS_DATA) + "/architectures/standard.arch");
  s.require("the standard architecture's script is read", !text.empty());
  const Reading r = read(text);
  s.require("it reads with no fault", r.ok(), r.faults.empty() ? "" : "line " + std::to_string(r.faults[0].line) + ": " + r.faults[0].message);
  const Architecture& a = r.architecture;
  std::size_t next = 0, examples = 0, boundaries = 0;
  for (const auto& w : a.wires) next += w.next;
  for (const auto& k : a.kinds) examples += k.example;
  for (const auto& b : a.blocks) boundaries += b.boundary;
  s.require("its declaration: 3 registers, 4 stages, 23 kinds (5 examples), 15 types, 13 blocks (3 boundaries), 25 wires (5 to the next turn)",
            a.name == "standard" && a.version == 1 && a.registers.size() == 3 && a.stage_order.size() == 4 && a.kinds.size() == 23 && examples == 5 && a.types.size() == 15 &&
                a.blocks.size() == 13 && boundaries == 3 && a.wires.size() == 25 && next == 5);
  const Reading back = read(print(a));
  s.require("it prints back to itself: read(print(a)) = a", back.ok() && back.architecture == a);

  const Verification v = verify(a);
  s.require("the standard architecture is admitted", v.admitted(), failing(v));
  std::size_t categorical = 0;
  for (const auto& c : v.checks) categorical += c.kind == "category";
  s.require("kategoria answers its categorical claims (at least 9)", categorical >= 9, std::to_string(categorical) + " claims");
  {
    bool stage_functor = false, terminal = false;
    for (const auto& c : v.checks) {
      stage_functor = stage_functor || (c.verdict.claim.find("StageOf is a functor") == 0 && c.verdict.holds());
      terminal = terminal || (c.verdict.claim.find("Fact is a terminal object of Kinds") == 0 && c.verdict.holds());
    }
    s.require("every wire within a turn goes forward in the stages: StageOf is a functor Step -> Stages", stage_functor);
    s.require("the kinds have one terminal kind, Fact", terminal);
  }

  {
    const Reading bad = read("architecture broken version 1\nregisters A < B\nblock X yields at s {\n  out y : Y\n}\nwire X.y -> Z.w\n");
    s.require("a statement that is wrong is reported with its line", !bad.ok() && bad.faults[0].line == 3, bad.faults.empty() ? "" : bad.faults[0].message);
    s.require("and the other statements are still read", bad.architecture.registers.size() == 2 && bad.architecture.wires.size() == 1);
    const Reading open = read("architecture o version 1\nblock X yields K at s {\n  out y : Y\n");
    s.require("a block left open is reported", !open.ok() && open.faults[0].message.find("not closed") != std::string::npos);
  }

  {
    // A small architecture whose only fault is a wire going backwards within a turn: from what is recorded to what is observed.
    const std::string backwards = R"(architecture backwards version 1
registers M < R < O
stages before < observed < recorded < after
stage before in O
stage observed in M
stage recorded in R
stage after in O
kind Fact in O
kind Trace under Fact in M
kind Finding under Fact in R
kind Occurrence under Fact
type X in R
type Y in M
type Z in R
type W in O
boundary Source yields Finding at recorded {
  out x : X
}
block Observe yields Trace at observed laws total {
  in x : X
  in w : W
  out y : Y
}
block Use yields Finding at recorded laws total {
  in y : Y
  out z : Z
}
block Act yields Occurrence at after laws referred {
  in z : Z
  out w : W
}
wire Source.x -> Observe.x
wire Observe.y -> Use.y
wire Use.z -> Act.z
wire Act.w -> Observe.w next
)";
    const Reading rb = read(backwards);
    const Verification vb = verify(rb.architecture);
    std::size_t constraint_fails = 0;
    bool functor_fails = false;
    for (const auto& c : vb.checks) {
      constraint_fails += c.kind == "constraint" && !c.verdict.holds();
      functor_fails = functor_fails || (c.kind == "category" && c.verdict.claim.find("StageOf is a functor") == 0 && c.verdict.fails());
    }
    s.require("a wire backwards within a turn passes every constraint", rb.ok() && constraint_fails == 0, failing(vb));
    s.require("and kategoria refuses it: StageOf is no functor", functor_fails && !vb.admitted(), failing(vb));
  }

  {
    Architecture b = a;
    b.block("Evaluation")->yields = "Need";
    const Verification vb = verify(b);
    s.require("a block yielding an example is refused", only_fails(vb, "every block yields a declared kind that is no example, at a declared stage"), failing(vb));
  }
  {
    Architecture b = a;
    b.wires.erase(b.wires.begin() + 5);  // Constitution.terms -> Interpretation.terms
    const Verification vb = verify(b);
    s.require("an input fed by no wire is refused (and the output that fed it feeds nothing)",
              !vb.admitted() && named(vb, "every input is fed by exactly one wire")->verdict.fails() && named(vb, "every output feeds a wire")->verdict.fails(), failing(vb));
  }
  {
    Architecture b = a;
    b.block("Evaluation")->laws.push_back("monotone");
    const Verification vb = verify(b);
    s.require("a law no checker knows is refused", only_fails(vb, "every law is known, and a referred block lies in the highest register"), failing(vb));
  }
  {
    Architecture b = a;
    for (auto& k : b.kinds)
      if (k.name == "Fact") k.parent = "Finding";
    const Verification vb = verify(b);
    bool undecided = false;
    for (const auto& c : vb.checks) undecided = undecided || (c.kind == "category" && c.verdict.undecided());
    s.require("a cycle of kinds is refused, and the categorical claims are not attempted", named(vb, "kinds form a forest without cycles, each in a declared register")->verdict.fails() && undecided, failing(vb));
  }
  {
    // The evaluation given the system's state directly: forward in the stages, and over what is observed.
    Architecture b = a;
    b.block("Evaluation")->inputs.push_back({"truth", "State"});
    b.wires.push_back({"System", "state", "Evaluation", "truth", false});
    const Verification vb = verify(b);
    bool functor = false;
    for (const auto& c : vb.checks) functor = functor || (c.verdict.claim.find("StageOf is a functor") == 0 && c.verdict.holds());
    s.require("the record given the system's state directly is refused, though StageOf is still a functor: the rule is of paths, not of functors",
              only_fails(vb, "a wire going forward within a turn reaches the next stage, or a later stage of its own register") && functor, failing(vb));
  }
  {
    Architecture b = a;
    b.kinds.push_back({"Hazard", "Risk", "", false});
    const Verification vb = verify(b);
    s.require("a kind under an example is refused: examples are leaves", only_fails(vb, "kinds form a forest without cycles, each in a declared register"), failing(vb));
  }
  {
    Architecture b = a;
    b.block("Estimation")->stage = "observed";
    const Verification vb = verify(b);
    s.require("a block at a stage of another register is refused", named(vb, "every block yields a kind in its stage's register")->verdict.fails(), failing(vb));
  }

  {
    // Revisions.
    Architecture t = a;
    t.version = 2;
    t.blocks.push_back({"Transfer", "Policy", "recorded", false, {"total"}, {{"practice", "Practice"}}, {{"carried", "Practice"}}});
    for (auto& w : t.wires)
      if (w.from == "Practice") w.to = "Transfer", w.in = "practice";
    t.wires.push_back({"Transfer", "carried", "PolicyMaking", "practice", false});
    const Reading tr = read(print(t));
    s.require("a revision made in code prints and reads back", tr.ok() && tr.architecture == t);
    const Revision rev = revise(a, t);
    s.require("a revision adding a block of transfer is admitted, its difference named",
              rev.verdict.holds() && rev.difference.blocks_added == std::vector<std::string>{"Transfer"} && rev.difference.wires_added.size() == 2 && rev.difference.wires_removed.size() == 1,
              rev.verdict.reason);

    Architecture u = a;
    u.version = 2;
    u.blocks.erase(std::remove_if(u.blocks.begin(), u.blocks.end(), [](const Block& x) { return x.name == "Interpretation"; }), u.blocks.end());
    const Revision bad = revise(a, u);
    s.require("a revision removing interpretation without rewiring is refused", bad.verdict.fails() && bad.difference.blocks_removed == std::vector<std::string>{"Interpretation"},
              bad.verdict.reason);
    const Revision same = revise(a, a);
    s.require("a revision that does not raise the version is refused", same.verdict.fails());
  }

  {
    const std::string json = report_json(a, v.checks);
    s.require("the report names the architecture and every check", json.find("\"architecture\": \"standard\"") != std::string::npos && json.find("StageOf") != std::string::npos);
  }
  return s.done();
}
