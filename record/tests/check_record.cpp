// Checks of the record, on the invented riverside example and the standard architecture.
//
// A trajectory is kept turn by turn: what the world feeds is observed, the methods fill the rest; a person entering what their
// role does not empower is kept as a proposal until a reviewer accepts it; an amendment keeps the value it replaces, makes what
// rests on it stale, and a turn run again recomputes what methods made but never a person's entry; an estimate that excludes
// every state the evidence allows is refused; a refusing review holds its block; a branch re-reads a turn without touching the
// main line; replay recomputes every value a method made; verify finds any alteration; what rests on an entry, a file or an
// agent is found; the record is W3C PROV without fault.

#include <kybernesis/architecture/language.hpp>
#include <kybernesis/methods/example.hpp>
#include <kybernesis/record/finite.hpp>
#include <kybernesis/record/record.hpp>

#include <calx/verification/check.hpp>

#include <fstream>
#include <sstream>
#include <string>

using namespace kybernesis;
using namespace kybernesis::record;
using blocks::Value;

namespace {

std::string slurp(const std::string& path) {
  std::ifstream in(path);
  std::stringstream s;
  s << in.rdbuf();
  return s.str();
}

// Instants, a minute apart.
std::string at(int k) {
  char buf[32];
  std::snprintf(buf, sizeof buf, "2026-11-19T%02d:%02d:00Z", 2 + k / 60, k % 60);
  return buf;
}

std::string label(const blocks::Instance& in, const std::string& type, const std::string& l) { return write(Value::certain(in.index(type, l))); }

bool contains(const std::string& s, const std::string& part) { return s.find(part) != std::string::npos; }

}  // namespace

int main() {
  calx::verification::Suite s;
  const auto a = architecture::read(slurp(std::string(KYBERNESIS_DATA) + "/architectures/standard.arch")).architecture;
  const blocks::Instance in = methods::example::riverside();
  const auto ms = methods_of(methods::example::assignment(a));
  int clock = 0;

  Rules rules{{{"*", Act::observe, "analyst"},
               {"Interpretation", Act::fill, "analyst"},
               {"Estimation", Act::fill, "analyst"},
               {"Estimation", Act::amend, "analyst"},
               {"Policy", Act::fill, "lead"},
               {"*", Act::review, "lead"}}};
  const blocks::Method pf = methods::possibility_filter(a, "Estimation", methods::example::world(), 3);
  std::map<std::string, Bound> bounds{{"Estimation", support_bound(in, "Belief", 0, [pf](const std::vector<Value>& args) {
                                         const auto out = pf.apply(args);
                                         kernel::Possible p(out[0].law.size());
                                         for (std::size_t i = 0; i < p.size(); i++) p[i] = out[0].law[i] > 0;
                                         return p;
                                       })}};
  Trajectory t = open(a, rules, checks(in), bounds, "kybernesis", at(clock++));
  s.require("a trajectory opens on the standard architecture and verifies", verify(t).holds() && t.branches.size() == 1, verify(t).reason);

  bool firsts = initial(t, "Estimation.belief -> Estimation.prior next", write(Value::belief({1.0 / 3, 1.0 / 3, 1.0 / 3})), "analyst", at(clock++)).holds();
  firsts = firsts && initial(t, "Policy.current -> PolicyMaking.current next", label(in, "PolicyVersion", "cautious"), "analyst", at(clock++)).holds();
  firsts = firsts && initial(t, "Decision.intervention -> Estimation.action next", label(in, "Action", "wait"), "analyst", at(clock++)).holds();
  firsts = firsts && initial(t, "Decision.inquiry -> Collection.inquiry next", label(in, "Inquiry", "routine"), "analyst", at(clock++)).holds();
  s.require("the first values of the wires carried to the next turn are recorded; a wire that does not exist is refused",
            firsts && initial(t, "Nothing.out -> Nowhere.in next", "x", "analyst", at(clock++)).fails());

  // Turn 0: the world is observed, the methods fill the rest.
  {
    const Progress p = run_turn(t, "main", 0, ms, at(clock++));
    bool collection = false;
    for (const auto& [b, why] : p.waiting) collection = collection || (b == "Collection" && contains(why, "the world"));
    s.require("before anything is observed, Collection waits on the world, and what follows waits on it", collection && current(t, "main", 0, "Decision") == nullptr);
    s.require("observing at a block the world does not feed is refused",
              observe(t, "main", 0, "Evidence", "gauge network", "analyst", {label(in, "Traces", "alarm"), label(in, "Words", "gauge")}, "", {}, at(clock++)).verdict.fails());
    const Outcome o = observe(t, "main", 0, "Collection", "gauge network", "analyst", {label(in, "Traces", "alarm")}, "the gauge sounded", {}, at(clock++));
    const Progress q = run_turn(t, "main", 0, ms, at(clock++));
    s.require("once Collection is observed, the turn is filled by the methods with nothing waiting", o.verdict.holds() && q.waiting.empty() && current(t, "main", 0, "Decision") != nullptr,
              o.verdict.reason);
  }
  // Turn 1.
  observe(t, "main", 1, "Collection", "gauge network", "analyst", {label(in, "Traces", "calm")}, "", {}, at(clock++));
  run_turn(t, "main", 1, ms, at(clock++));
  s.require("turn 1 is filled", current(t, "main", 1, "Decision") != nullptr && last_turn(t, "main") == 1u);

  // A proposal and its review.
  {
    const Entry* method_eval = current(t, "main", 1, "Evaluation");
    const Outcome p = fill(t, "main", 1, "Evaluation", "a visitor", "guest", {label(in, "Evaluations", "concern")}, "it looked worrying", {}, at(clock++));
    s.require("a person whose role is not empowered enters a proposal, which does not replace the current value",
              p.verdict.undecided() && !t.entries[static_cast<std::size_t>(p.entry)].counts && current(t, "main", 1, "Evaluation") == method_eval, p.verdict.reason);
    s.require("a reviewer without the role is refused", review(t, static_cast<std::size_t>(p.entry), "a passer-by", "guest", true, "", at(clock++)).verdict.fails());
    const Outcome r = review(t, static_cast<std::size_t>(p.entry), "the lead", "lead", true, "concern is warranted after the visit", at(clock++));
    const Entry* now = current(t, "main", 1, "Evaluation");
    s.require("accepted by the lead, the proposal becomes the record's", r.verdict.holds() && now && now->act == Act::review && now->outputs == t.entries[static_cast<std::size_t>(p.entry)].outputs,
              r.verdict.reason);
    const Entry* pm = current(t, "main", 1, "PolicyMaking");
    s.require("what a method made from the replaced value is stale", pm && stale(t, *pm).has_value(), pm ? stale(t, *pm).value_or("not stale") : "no entry");
    const Progress q = propagate(t, "main", 1, ms, at(clock++));
    const Entry* pm2 = current(t, "main", 1, "PolicyMaking");
    s.require("a turn run again recomputes what methods made after it, and nothing is stale", !q.made.empty() && pm2 && pm2 != pm && !stale(t, *pm2), std::to_string(q.made.size()) + " recomputed");
  }
  // An amendment, its bound, and a person's entry left stale.
  std::size_t amended = 0;
  {
    const Entry* est = current(t, "main", 1, "Estimation");
    const auto args = [&] {
      std::vector<Value> v;
      for (const auto& d : est->inputs) v.push_back(*parse(payload(t, d)));
      return v;
    }();
    const auto allowed = pf.apply(args)[0].law;
    std::size_t outside = 3, inside = 3;
    for (std::size_t i = 0; i < 3; i++) (allowed[i] > 0 ? inside : outside) = i;
    s.require("the evidence of turn 1 allows some states and not others", outside < 3 && inside < 3);
    Value excluded = Value::belief({0, 0, 0});
    excluded.law[outside] = 1;
    s.require("an estimate that excludes every state the evidence allows is refused",
              amend(t, est->id, "an engineer", "analyst", {write(excluded)}, "", {}, at(clock++)).verdict.fails());
    Value narrow = Value::belief({0, 0, 0});
    narrow.law[inside] = 1;
    const File minutes = file_of("minutes.txt", "The gauge was serviced; the engineer inspected the levee.", "text/plain");
    s.require("a file is recorded by its digest", attach(t, minutes, "an engineer", at(clock++)).holds());
    const Outcome o = amend(t, est->id, "an engineer", "analyst", {write(narrow)}, "inspected on site", {minutes.digest}, at(clock++));
    amended = static_cast<std::size_t>(o.entry);
    s.require("an estimate narrower than the evidence is entered with the remark, the computed value kept beside it",
              o.verdict.holds() && contains(o.verdict.reason, "leaves out states the evidence allows") && t.entries[amended].computed == est->outputs, o.verdict.reason);
    const Progress q = run_turn(t, "main", 1, ms, at(clock++));
    bool reported = false;
    for (const auto& [id, why] : q.stale) reported = reported || t.entries[id].block == "Evaluation";
    s.require("the person's evaluation resting on the old estimate is reported stale, never recomputed", reported && current(t, "main", 1, "Evaluation")->act == Act::review);
  }
  // A refusing review holds its block.
  {
    const Entry* pol = current(t, "main", 1, "Policy");
    review(t, pol->id, "the lead", "lead", false, "the policy is under discussion", at(clock++));
    const Progress q = run_turn(t, "main", 1, ms, at(clock++));
    bool held = false;
    for (const auto& [b, why] : q.waiting) held = held || (b == "Decision" && contains(why, "refused"));
    s.require("a refused policy holds its block, and the decision waits on it", held && !holds_value(current(t, "main", 1, "Policy")));
    const Outcome f = fill(t, "main", 1, "Policy", "the lead", "lead", {label(in, "Action", "protect"), label(in, "PolicyVersion", "cautious")}, "protect while it is discussed", {},
                           at(clock++));
    const Progress r = run_turn(t, "main", 1, ms, at(clock++));
    s.require("filled by the lead, the decision follows", f.verdict.holds() && holds_value(current(t, "main", 1, "Decision")) && r.waiting.empty());
  }
  s.require("an ill-typed entry is refused", fill(t, "main", 1, "Policy", "the lead", "lead", {write(Value::certain(7)), label(in, "PolicyVersion", "cautious")}, "", {}, at(clock++)).verdict.fails());

  // A branch re-reads turn 1 under another reading.
  {
    const std::size_t before = t.entries.size();
    s.require("a branch is taken from the main line at turn 1", branch(t, "main", 1, "loose reading", "read flooded loosely", "an analyst", at(clock++)).holds());
    s.require("the branch reads what it inherits as the main line stood", current(t, "loose reading", 1, "Decision") == current(t, "main", 1, "Decision"));
    const Outcome f = fill(t, "loose reading", 1, "Interpretation", "an analyst", "analyst", {label(in, "Readings", "loose")}, "residents mean water in the streets", {}, at(clock++));
    const Progress p = run_turn(t, "loose reading", 1, ms, at(clock++));
    const auto d0 = diff(t, "main", 0, "loose reading", 0);
    const auto d1 = diff(t, "main", 1, "loose reading", 1);
    bool interp = false;
    for (const auto& c : d1) interp = interp || c.block == "Interpretation";
    bool untouched = true;
    for (std::size_t k = before; k < t.entries.size(); k++) untouched = untouched && t.entries[k].branch == "loose reading";
    bool persons = false;
    for (const auto& [id, why] : p.stale) persons = persons || t.entries[id].block == "Estimation";
    s.require("re-reading turn 1 on the branch: turn 0 shared, Interpretation differs, the persons' entries resting on the old reading reported stale there, the main line untouched",
              f.verdict.holds() && d0.empty() && interp && untouched && persons && !stale(t, *current(t, "main", 1, "Estimation")), std::to_string(d1.size()) + " outputs differ at turn 1");
    s.require("a turn before the branch was taken cannot be written on it",
              fill(t, "loose reading", 0, "Interpretation", "an analyst", "analyst", {label(in, "Readings", "loose")}, "", {}, at(clock++)).verdict.fails());
    // From the start, where methods made everything after Collection: a change there is carried by the methods, on the branch alone.
    s.require("a second branch is taken from turn 0", branch(t, "main", 0, "from the start", "read flooded strictly from the first turn", "an analyst", at(clock++)).holds());
    fill(t, "from the start", 0, "Interpretation", "an analyst", "analyst", {label(in, "Readings", "strict")}, "only deep water floods", {}, at(clock++));
    const Progress q = propagate(t, "from the start", 0, ms, at(clock++));
    std::set<std::string> recomputed;
    for (std::size_t id : q.made) recomputed.insert(t.entries[id].block + "@" + std::to_string(t.entries[id].turn));
    s.require("on it, the methods after Interpretation recompute turn 0 and carry the change into turn 1, where the persons' entries are reported",
              recomputed.count("Estimation@0") && recomputed.count("Decision@0") && !q.stale.empty() && diff(t, "main", 0, "from the start", 0).size() > 1,
              [&] {
                std::string w;
                for (const auto& r : recomputed) w += r + " ";
                return w + "recomputed, " + std::to_string(q.stale.size()) + " reported stale, " + std::to_string(diff(t, "main", 0, "from the start", 0).size()) + " outputs differ at turn 0";
              }());
  }

  // Replay and verification.
  {
    const auto rs = replay(t, ms);
    bool all = !rs.empty();
    for (const auto& r : rs) all = all && r.verdict.holds();
    s.require("every value a method made replays from its recorded inputs", all, std::to_string(rs.size()) + " replayed");
    auto changed = ms;
    changed["PolicyMaking"].apply = [](const std::vector<std::string>&) { return std::vector<std::string>{write(Value::certain(1))}; };
    std::size_t differ = 0;
    for (const auto& r : replay(t, changed)) differ += r.verdict.fails();
    s.require("a method that has changed under its name is found by replay", differ > 0);
    s.require("the record verifies", verify(t).holds(), verify(t).reason);
    Trajectory a1 = t;
    a1.entries[5].reason = "rewritten";
    Trajectory a2 = t;
    a2.values.begin()->second += " ";
    Trajectory a3 = t;
    std::swap(a3.journal.entries[6], a3.journal.entries[7]);
    Trajectory a4 = t;
    a4.entries.pop_back();
    s.require("an altered entry, an altered value, a reordered journal and a missing entry are each found",
              verify(a1).fails() && contains(verify(a1).reason, "altered") && verify(a2).fails() && verify(a3).fails() && verify(a4).fails(),
              verify(a1).reason + " | " + verify(a2).reason + " | " + verify(a3).reason + " | " + verify(a4).reason);
  }

  // Revisiting.
  {
    const auto from_amend = resting_on(t, {amended});
    bool downstream = false;
    for (std::size_t id : from_amend) downstream = downstream || (id != amended && t.entries[id].block == "Policy");
    const auto from_file = resting_on_file(t, file_of("minutes.txt", "The gauge was serviced; the engineer inspected the levee.").digest);
    const auto from_visitor = resting_on_agent(t, "a visitor");
    bool reviewed = false;
    for (std::size_t id : from_visitor) reviewed = reviewed || t.entries[id].act == Act::review;
    s.require("what rests on the amendment, on the minutes and on the visitor's proposal is found",
              downstream && from_file.count(amended) && from_file.size() > 1 && reviewed);
    const auto g = lineage(t);
    const auto faults = archeion::provenance::faults(g);
    const Entry* dec = current(t, "main", 1, "Decision");
    const auto back = archeion::provenance::lineage(g, "e" + std::to_string(dec->id) + ".0");
    bool observed = false;
    for (const auto& id : back)
      if (id.rfind("e", 0) == 0) {
        const std::size_t eid = std::stoul(id.substr(1, id.find('.') - 1));
        observed = observed || (t.entries[eid].block == "Collection" && t.entries[eid].act == Act::observe);
      }
    s.require("the record as PROV has no fault, and the decision's lineage reaches what was observed", faults.empty() && observed,
              faults.empty() ? std::to_string(back.size()) + " entities in its lineage" : faults.front().where + ": " + faults.front().what);
  }
  // A choice of method: the possibilistic filter chosen for Estimation on a branch from turn 0.
  {
    Registry alternatives = registry_of(ms);
    alternatives["Estimation"].push_back(method_of(pf));
    s.require("a role the rules do not empower may not choose a block's method",
              choose(t, "main", 0, "Estimation", "possibility filter", "an analyst", "analyst", "", at(clock++), &alternatives).fails());
    t.rules.grants.push_back({"Estimation", Act::choose, "lead"});
    s.require("a method not among the block's alternatives is refused",
              choose(t, "main", 0, "Estimation", "a crystal ball", "the lead", "lead", "", at(clock++), &alternatives).fails());
    branch(t, "main", 0, "possibilistic", "estimate by what the evidence allows, without a law", "the lead", at(clock++));
    const Verdict c = choose(t, "possibilistic", 0, "Estimation", "possibility filter", "the lead", "lead", "no law of the river is agreed", at(clock++), &alternatives);
    const Progress q = propagate(t, "possibilistic", 0, alternatives, at(clock++));
    const Entry* e0 = current(t, "possibilistic", 0, "Estimation");
    bool reported = false;
    for (const auto& [id, why] : q.stale) reported = reported || t.entries[id].block == "Estimation";
    s.require("on the branch the chosen method fills Estimation from turn 0, the person's estimate at turn 1 reported stale, the main line unchanged",
              c.holds() && e0 && e0->by == "possibility filter" && reported && current(t, "main", 0, "Estimation")->by == "Bayes filter" &&
                  chosen(t, "possibilistic", 1, "Estimation") == "possibility filter" && chosen(t, "main", 1, "Estimation").empty(),
              c.reason);
    const auto rs = replay(t, alternatives);
    bool all = !rs.empty();
    for (const auto& r : rs) all = all && r.verdict.holds();
    Trajectory altered = t;
    altered.choices.back().method = "Bayes filter";
    s.require("a choice is sealed: the record verifies and replays with the alternatives, and a choice altered afterwards is found",
              verify(t).holds() && all && verify(altered).fails(), verify(altered).reason);
  }
  {
    const auto props = carry(t, "main", 2, "the lead", "lead", at(clock++));
    bool proposals = props.size() == 3;
    for (const auto& o : props) proposals = proposals && o.verdict.undecided();
    t.rules.grants.push_back({"*", Act::fill, "keeper"});
    const auto kept = carry(t, "main", 2, "the keeper", "keeper", at(clock++));
    bool all = kept.size() == 3;
    for (const auto& o : kept) all = all && o.verdict.holds();
    const bool said = all && contains(t.entries[static_cast<std::size_t>(kept[0].entry)].reason, "carried from turn 1");
    const auto again = carry(t, "main", 2, "the keeper", "keeper", at(clock++));
    s.require("the boundaries carried to the next turn: by a role not empowered as proposals, by an empowered one as entries saying where they came from, once only",
              proposals && all && said && again.empty(), std::to_string(props.size()) + " proposals, " + std::to_string(kept.size()) + " carried");
  }
  {
    architecture::Architecture v2 = a;
    v2.version = 2;
    const std::string before = t.arch_digest;
    s.require("a revision of the architecture is sealed, and later entries carry it",
              revise(t, v2, "the council", "a second version", at(clock++)).holds() && t.arch_digest != before &&
                  observe(t, "main", 2, "Collection", "gauge network", "analyst", {label(in, "Traces", "calm")}, "", {}, at(clock++)).verdict.holds() &&
                  t.entries.back().architecture == t.arch_digest && verify(t).holds());
    s.require("an act dated before the record's last is refused", fill(t, "main", 2, "Interpretation", "an analyst", "analyst", {label(in, "Readings", "strict")}, "", {}, at(0)).verdict.fails());
  }
  return s.done();
}
