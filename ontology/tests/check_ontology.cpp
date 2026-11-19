// Checks of the ontology of facts as data (decision 0005), on the ontology of the standard architecture's facts (ontologies/).
//
// - The ontology reads, prints back to itself, and is admitted: its constraints hold and kategoria answers its claims.
// - A faulty declaration is refused by the constraint that names its fault, and kategoria is not asked about it.
// - A record of facts is held to it: a required axis missing, a value that is none, an axis that does not apply, a link whose ends
//   do not fit, each named.
// - A revision is admitted with its mapping, and what it changes is said; a revision that moves a kind to another register, breaks
//   the order of kinds (no functor) or reverses an ordered axis is refused.
// - A record is carried along the revision: kinds and values renamed, what the revision does not keep said, what it now requires
//   and the record lacks named.

#include <kybernesis/ontology/ontology.hpp>

#include <calx/verification/check.hpp>

#include <fstream>
#include <sstream>
#include <string>

using namespace kybernesis::ontology;

namespace {

std::string slurp(const std::string& path) {
  std::ifstream in(path);
  std::stringstream s;
  s << in.rdbuf();
  return s.str();
}

bool says(const std::vector<std::string>& xs, const std::string& part) {
  for (const auto& x : xs)
    if (x.find(part) != std::string::npos) return true;
  return false;
}

bool names(const std::vector<Problem>& ps, const std::string& part) {
  for (const auto& p : ps)
    if ((p.where + ": " + p.what).find(part) != std::string::npos) return true;
  return false;
}

std::string failing(const Verification& v) {
  for (const auto& c : v.checks)
    if (!c.verdict.holds()) return c.name + ": " + c.verdict.reason;
  return "";
}

}  // namespace

int main() {
  calx::verification::Suite s;
  const std::string dir = std::string(KYBERNESIS_DATA) + "/ontologies/";
  const auto r1 = read(slurp(dir + "facts.ont"));
  const auto r2 = read(slurp(dir + "facts-v2.ont"));
  s.require("both versions of the ontology of facts read", r1.ok() && r2.ok(), r1.ok() ? (r2.ok() ? "" : r2.faults[0].message) : r1.faults[0].message);
  const Ontology& o1 = r1.ontology;
  const Ontology& o2 = r2.ontology;
  s.require("an ontology prints back to itself", read(print(o1)).ontology == o1 && read(print(o2)).ontology == o2);
  const auto v1 = verify(o1);
  s.require("version 1 is admitted: its constraints hold and kategoria answers its claims", v1.admitted() && v1.checks.size() == 7, failing(v1));
  s.require("version 2 is admitted", verify(o2).admitted(), failing(verify(o2)));

  {
    const auto bad = read("ontology x version 1\nregisters A < B\nkind Fact in B\nkind Odd undr Fact\n");
    s.require("a statement that does not read is refused with its line", !bad.ok() && bad.faults[0].line == 4, bad.ok() ? "" : bad.faults[0].message);
    Ontology up = o1;
    up.kinds.push_back({"Claim", "Trace", "Ontological", false});
    const auto vu = verify(up);
    s.require("a kind more about what obtains than the kind it is a kind of is refused, and kategoria is not asked",
              !vu.admitted() && failing(vu).find("never more about what obtains") != std::string::npos && vu.checks.back().verdict.undecided(), failing(vu));
    Ontology orphan = o1;
    orphan.kinds.push_back({"Stray", "Nowhere", "Ontological", false});
    s.require("a kind of an undeclared kind is refused", !verify(orphan).admitted() && failing(verify(orphan)).find("Nowhere") != std::string::npos, failing(verify(orphan)));
    Ontology loose = o1;
    for (auto& a : loose.axes)
      if (a.name == "grade") a.requires_.push_back("Finding");
    s.require("an axis required where it does not apply is refused", !verify(loose).admitted() && failing(verify(loose)).find("does not apply") != std::string::npos, failing(verify(loose)));
    Ontology two_roots = o1;
    two_roots.kinds.push_back({"Other", "", "Ontological", false});
    const auto vr = verify(two_roots);
    s.require("an ontology with two roots is refused by kategoria: its kinds have no terminal object", !vr.admitted() && vr.checks.back().verdict.fails(), failing(vr));
  }

  Record rec;
  rec.facts = {{"t1", "Trace", "a gauge's reading", {{"grade", "A"}, {"scope", "individual"}}},
               {"f1", "Finding", "the street floods at high water", {{"standing", "asserted"}, {"scope", "general"}}},
               {"o1", "Occurrence", "the flood of the third", {}},
               {"p1", "Proposal", "raise the kerb", {}},
               {"c1", "Construal", "flooded read as water in the street", {{"source", "convention"}, {"standing", "admitted"}}}};
  rec.links = {{"rests_on", "f1", "t1"}, {"construes", "c1", "t1"}};
  s.require("a record that fits the ontology has no problem", problems(o1, rec).empty(), problems(o1, rec).empty() ? "" : problems(o1, rec)[0].what);
  {
    Record bad = rec;
    bad.facts.push_back({"t2", "Trace", "a report", {{"standing", "admitted"}}});
    bad.facts.push_back({"f2", "Finding", "it is safe", {{"standing", "certain"}}});
    bad.facts.push_back({"x1", "Rumour", "", {}});
    bad.links.push_back({"rests_on", "t1", "f1"});
    bad.links.push_back({"owes", "f1", "t1"});
    const auto ps = problems(o1, bad);
    s.require("a record that does not fit has each problem named",
              names(ps, "grade is required for Trace") && names(ps, "standing does not apply to Trace") && names(ps, "certain is no value of standing") &&
                  names(ps, "Rumour is not in the ontology") && names(ps, "Trace is not a kind of Finding") && names(ps, "owes is no relation"),
              std::to_string(ps.size()) + " problems");
  }

  const auto mr = read_mapping(slurp(dir + "facts-1-to-2.rev"));
  s.require("the mapping reads", mr.ok(), mr.ok() ? "" : mr.faults[0].message);
  const auto ad = admit(o1, o2, mr.mapping);
  s.require("the revision is admitted, kategoria deciding that the map of kinds is a functor of their orders", ad.admitted() && ad.checks.back().verdict.holds(), ad.verdict.reason);
  s.require("what the revision changes is said by name",
            says(ad.changes, "Occurrence becomes Event") && says(ad.changes, "Commitment is added") && says(ad.changes, "asserted becomes claimed") &&
                says(ad.changes, "construes is dropped") && says(ad.changes, "axis scope is dropped") && says(ad.changes, "honours is added"),
            std::to_string(ad.changes.size()) + " changes");
  {
    Mapping other = mr.mapping;
    other.kinds["Trace"] = "Condition";
    s.require("a revision that moves a kind to another register is refused", !admit(o1, o2, other).admitted() && admit(o1, o2, other).verdict.reason.find("Trace") != std::string::npos);
    Mapping twisted = mr.mapping;
    twisted.kinds["Decision"] = "Proposal";
    const auto at = admit(o1, o2, twisted);
    s.require("a revision whose map of kinds is no functor (a decision made a proposal, outside the institutional) is refused by kategoria",
              !at.admitted() && at.checks.back().verdict.fails(), at.checks.back().verdict.reason);
    Mapping reversed = mr.mapping;
    reversed.values["standing"] = {{"proposed", "established"}, {"asserted", "admitted"}, {"admitted", "claimed"}, {"established", "proposed"}};
    s.require("a revision that reverses an ordered axis is refused", !admit(o1, o2, reversed).admitted() && admit(o1, o2, reversed).verdict.reason.find("order") != std::string::npos);
  }
  {
    const auto mg = migrate(rec, o1, o2, mr.mapping);
    const Fact* o = nullptr;
    const Fact* f = nullptr;
    for (const auto& x : mg.record.facts) {
      if (x.id == "o1") o = &x;
      if (x.id == "f1") f = &x;
    }
    s.require("carried along the revision, the occurrence is an event and asserted is now claimed",
              o && o->kind == "Event" && f && f->axes.at("standing") == "claimed" && says(mg.kept, "Occurrence is now Event"));
    s.require("what the revision does not keep is said: the scope of facts, the construal's link",
              says(mg.lost, "scope = general") && says(mg.lost, "construes") && mg.record.links.size() == 1);
    s.require("what the revision now requires and the record lacks is named: the proposal's standing",
              mg.problems.size() == 1 && names(mg.problems, "standing is required for Proposal"), mg.problems.empty() ? "none" : mg.problems[0].what);
  }
  return s.done();
}
