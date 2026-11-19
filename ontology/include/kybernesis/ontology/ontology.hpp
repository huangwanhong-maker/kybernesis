// The ontology of facts as data: what kinds of fact there are, and how a fact is described, declared, revised and admitted
// (decision 0005).
//
// A fixed taxonomy of facts serves one domain well until the next needs another kind, another axis, another relation. As the
// architecture of the loop is data (architecture/), so is the ontology of facts: a script declares it, constraints and categorical
// claims admit it, records of facts are checked against it, and a revision is admitted with a mapping from the old to the new,
// along which records are carried and what does not survive the carrying is said by name.
//
//   registers      the order of aboutness, lowest first (Manifestational < Relational < Ontological in the user's framework);
//   kinds          a forest of kinds, each a kind of its parent, each in a register (inherited unless stated); a kind is never
//                  more about what obtains than the kind it is a kind of;
//   axes           ways a fact of a kind is described: nominal (a set of values) or ordered (values in an order); an axis
//                  applies to some kinds and everything beneath them, and may be required for some of them;
//   relations      typed by the kinds of their ends: a link between two facts names a relation whose ends contain the facts'
//                  kinds.
//
// The language, one statement a line, '#' a comment, read by morphe's BNF:
//
//   ontology <Name> version <Integer>
//   registers <R> < <R> < ...
//   kind <Name> [under <Parent>] [in <Register>]
//   axis <name> : <value>, <value>, ...                 nominal
//   ordered axis <name> : <value> < <value> < ...       ordered, lowest first
//   applies <axis> to <Kind>, <Kind>, ...
//   requires <axis> for <Kind>, <Kind>, ...
//   relation <name> : <Kind> -> <Kind>
//
// A revision is a second ontology and a mapping, written:
//
//   revision <Name> from <Integer> to <Integer>
//   map kind <Old> -> <New>
//   map value <axis> <old> -> <new>
//   map relation <old> -> <new>
//   drop relation <old>
//
// Names that are the same in both need no mapping. The revision is admitted when both ontologies are, every old kind, value and
// relation has an image, the map of kinds is a functor of the kinds' orders (decided by kategoria) that keeps each kind's
// register, the map of an ordered axis keeps its order, and each relation's image contains the images of its ends. A record is
// carried by relabelling (the left Kan extension of a record seen as a set over the old kinds along the functor): kinds and values
// mapped, axes the new ontology does not keep for a kind dropped and said, links whose relation is dropped said; the carried
// record is then checked against the new ontology, and what it lacks (a newly required axis) is named.
#pragma once

#include <kybernesis/architecture/model.hpp>
#include <kybernesis/kernel/verdicts.hpp>

#include <kategoria/script/session.hpp>
#include <morphe/formal/bnf.hpp>

#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace kybernesis::ontology {

using architecture::Kind;
using kernel::Check;
using kernel::Verdict;

struct Axis {
  std::string name;
  bool ordered = false;
  std::vector<std::string> values;      // lowest first when ordered
  std::vector<std::string> applies;     // kinds it applies to, with everything beneath them
  std::vector<std::string> requires_;   // kinds for which it is required, with everything beneath them
  bool operator==(const Axis&) const = default;
};

struct Relation {
  std::string name, from, to;
  bool operator==(const Relation&) const = default;
};

struct Ontology {
  std::string name;
  long version = 0;
  std::vector<std::string> registers;
  std::vector<Kind> kinds;
  std::vector<Axis> axes;
  std::vector<Relation> relations;
  bool operator==(const Ontology&) const = default;

  const Kind* kind(const std::string& n) const {
    for (const auto& k : kinds)
      if (k.name == n) return &k;
    return nullptr;
  }
  const Axis* axis(const std::string& n) const {
    for (const auto& a : axes)
      if (a.name == n) return &a;
    return nullptr;
  }
  const Relation* relation(const std::string& n) const {
    for (const auto& r : relations)
      if (r.name == n) return &r;
    return nullptr;
  }
  // A kind's register, inherited along its parents; empty when none is declared on the way or the parents form a cycle.
  std::string register_of(const std::string& n) const {
    std::string at = n;
    for (std::size_t steps = 0; steps <= kinds.size(); steps++) {
      const Kind* k = kind(at);
      if (!k) return {};
      if (!k->register_.empty()) return k->register_;
      if (k->parent.empty()) return {};
      at = k->parent;
    }
    return {};
  }
  long register_index(const std::string& r) const {
    for (std::size_t i = 0; i < registers.size(); i++)
      if (registers[i] == r) return static_cast<long>(i);
    return -1;
  }
  // Whether a kind is the other or beneath it.
  bool beneath(const std::string& n, const std::string& ancestor) const {
    std::string at = n;
    for (std::size_t steps = 0; steps <= kinds.size(); steps++) {
      if (at == ancestor) return true;
      const Kind* k = kind(at);
      if (!k || k->parent.empty()) return false;
      at = k->parent;
    }
    return false;
  }
  bool applies(const Axis& a, const std::string& k) const {
    for (const auto& x : a.applies)
      if (beneath(k, x)) return true;
    return false;
  }
  bool required(const Axis& a, const std::string& k) const {
    for (const auto& x : a.requires_)
      if (beneath(k, x)) return true;
    return false;
  }
};

struct Fault {
  std::size_t line = 0;
  std::string message;
};

// ---- The language ---------------------------------------------------------------------------------------------------------------

namespace detail {

inline std::string_view grammar_text() {
  return R"bnf(
statement ::= header | registers | kind | axis | oaxis | applies | requires | relation | revision | mapkind | mapvalue | maprel | droprel ;
header    ::= "ontology" _sp oname _sp "version" _sp over ;
oname     ::= _name ;
over      ::= [0-9]+ ;
registers ::= "registers" _sp rname ( _s "<" _s rname )* ;
rname     ::= _name ;
kind      ::= "kind" _sp kname ( _sp "under" _sp kparent )? ( _sp "in" _sp regname )? ;
kname     ::= _name ;
kparent   ::= _name ;
regname   ::= _name ;
axis      ::= "axis" _sp xname _s ":" _s value ( _s "," _s value )* ;
oaxis     ::= "ordered" _sp "axis" _sp xname _s ":" _s value ( _s "<" _s value )* ;
xname     ::= _name ;
value     ::= _name ;
applies   ::= "applies" _sp xname _sp "to" _sp kref ( _s "," _s kref )* ;
requires  ::= "requires" _sp xname _sp "for" _sp kref ( _s "," _s kref )* ;
kref      ::= _name ;
relation  ::= "relation" _sp lname _s ":" _s lfrom _s "->" _s lto ;
lname     ::= _name ;
lfrom     ::= _name ;
lto       ::= _name ;
revision  ::= "revision" _sp oname _sp "from" _sp vfrom _sp "to" _sp vto ;
vfrom     ::= [0-9]+ ;
vto       ::= [0-9]+ ;
mapkind   ::= "map" _sp "kind" _sp old _s "->" _s new ;
mapvalue  ::= "map" _sp "value" _sp xname _sp old _s "->" _s new ;
maprel    ::= "map" _sp "relation" _sp old _s "->" _s new ;
droprel   ::= "drop" _sp "relation" _sp old ;
old       ::= _name ;
new       ::= _name ;
_name     ::= [A-Za-z_] [A-Za-z0-9_]* ;
_sp       ::= [ \t]+ ;
_s        ::= [ \t]* ;
)bnf";
}

inline const morphe::formal::Bnf& grammar() {
  static const morphe::formal::Bnf bnf = morphe::formal::compile_bnf(grammar_text());
  return bnf;
}

struct Line {
  std::string text;
  std::size_t number;
};

inline std::vector<Line> lines_of(std::string_view script) {
  std::vector<Line> out;
  std::string cur;
  std::size_t n = 1;
  auto flush = [&] {
    std::string l = cur;
    if (const auto h = l.find('#'); h != std::string::npos) l.erase(h);
    const auto a = l.find_first_not_of(" \t"), b = l.find_last_not_of(" \t");
    if (a != std::string::npos) out.push_back({l.substr(a, b - a + 1), n});
    cur.clear();
  };
  for (char c : script) {
    if (c == '\r') continue;
    if (c == '\n') {
      flush();
      n++;
    } else {
      cur += c;
    }
  }
  flush();
  return out;
}

template <class F>
inline void each_statement(std::string_view script, std::vector<Fault>& faults, F f) {
  for (const auto& l : lines_of(script)) {
    const auto parsed = grammar().parse(l.text);
    if (!parsed.accepted) {
      faults.push_back({l.number, parsed.error});
      continue;
    }
    const morphe::formal::ParseNode& top = parsed.tree;
    f(top.children.empty() ? top : top.children.front(), l.number);
  }
}

inline std::string text_of(const morphe::formal::ParseNode& n, const char* child) {
  const auto* c = n.child(child);
  return c ? c->text : std::string();
}

}  // namespace detail

struct Reading {
  Ontology ontology;
  std::vector<Fault> faults;  // syntax only
  bool ok() const { return faults.empty(); }
};

// Reads an ontology. Every statement is parsed; a wrong one is reported with its line and what could have come next.
inline Reading read(std::string_view script) {
  Reading r;
  Ontology& o = r.ontology;
  bool header = false;
  detail::each_statement(script, r.faults, [&](const morphe::formal::ParseNode& n, std::size_t line) {
    const std::string& what = n.name;
    auto text = [&](const char* c) { return detail::text_of(n, c); };
    if (what == "header") {
      if (header) r.faults.push_back({line, "a second ontology statement"});
      header = true;
      o.name = text("oname"), o.version = std::stol(text("over"));
    } else if (what == "registers") {
      if (!o.registers.empty()) r.faults.push_back({line, "a second registers statement"});
      for (const auto* x : n.all("rname")) o.registers.push_back(x->text);
    } else if (what == "kind") {
      o.kinds.push_back({text("kname"), text("kparent"), text("regname"), false});
    } else if (what == "axis" || what == "oaxis") {
      Axis a;
      a.name = text("xname"), a.ordered = what == "oaxis";
      for (const auto* x : n.all("value")) a.values.push_back(x->text);
      o.axes.push_back(std::move(a));
    } else if (what == "applies" || what == "requires") {
      Axis* a = nullptr;
      for (auto& x : o.axes)
        if (x.name == text("xname")) a = &x;
      if (!a) {
        r.faults.push_back({line, "the axis " + text("xname") + " is used before it is declared"});
        return;
      }
      for (const auto* x : n.all("kref")) (what == "applies" ? a->applies : a->requires_).push_back(x->text);
    } else if (what == "relation") {
      o.relations.push_back({text("lname"), text("lfrom"), text("lto")});
    } else {
      r.faults.push_back({line, "a statement of a revision in an ontology: " + what});
    }
  });
  if (!header && r.faults.empty()) r.faults.push_back({1, "no ontology statement (ontology <Name> version <Integer>)"});
  return r;
}

// Prints an ontology as a script that reads back to it.
inline std::string print(const Ontology& o) {
  std::string s = "ontology " + o.name + " version " + std::to_string(o.version) + "\n";
  if (!o.registers.empty()) {
    s += "registers";
    for (std::size_t i = 0; i < o.registers.size(); i++) s += (i ? " < " : " ") + o.registers[i];
    s += "\n";
  }
  for (const auto& k : o.kinds) s += "kind " + k.name + (k.parent.empty() ? "" : " under " + k.parent) + (k.register_.empty() ? "" : " in " + k.register_) + "\n";
  for (const auto& a : o.axes) {
    s += std::string(a.ordered ? "ordered axis " : "axis ") + a.name + " :";
    for (std::size_t i = 0; i < a.values.size(); i++) s += (i ? (a.ordered ? " < " : ", ") : " ") + a.values[i];
    s += "\n";
  }
  for (const auto& a : o.axes) {
    if (!a.applies.empty()) {
      s += "applies " + a.name + " to";
      for (std::size_t i = 0; i < a.applies.size(); i++) s += (i ? ", " : " ") + a.applies[i];
      s += "\n";
    }
    if (!a.requires_.empty()) {
      s += "requires " + a.name + " for";
      for (std::size_t i = 0; i < a.requires_.size(); i++) s += (i ? ", " : " ") + a.requires_[i];
      s += "\n";
    }
  }
  for (const auto& r : o.relations) s += "relation " + r.name + " : " + r.from + " -> " + r.to + "\n";
  return s;
}

// ---- Admission of an ontology -----------------------------------------------------------------------------------------------------

namespace detail {

inline Check check(std::string name, Verdict v, std::string kind = "constraint") { return {std::move(name), std::move(v), std::move(kind)}; }

inline std::string duplicated(const std::vector<std::string>& names) {
  std::set<std::string> seen;
  for (const auto& n : names)
    if (!seen.insert(n).second) return n;
  return {};
}

template <class Outcomes>
inline Verdict session_verdict(const Outcomes& outcomes, const std::string& claim) {
  for (const auto& o : outcomes.items()) {
    const auto* status = o.get("status");
    if (!status) continue;
    const std::string st = status->text();
    const auto* reason = o.get("reason");
    const auto* cl = o.get("claim");
    const std::string why = (cl ? cl->text() : std::string("a claim")) + (reason ? ": " + reason->text() : std::string());
    if (st == "fails") return Verdict::no(claim, why);
    if (st != "holds") return Verdict::unknown(claim, why);
  }
  return Verdict::yes(claim);
}

inline std::string categories_of(const Ontology& o, const std::string& prefix) {
  std::string s = "category " + prefix + "Kinds {\n  objects";
  for (std::size_t i = 0; i < o.kinds.size(); i++) s += (i ? ", " : " ") + o.kinds[i].name;
  s += "\n";
  for (std::size_t i = 0; i < o.kinds.size(); i++)
    if (!o.kinds[i].parent.empty()) s += "  k" + std::to_string(i) + " : " + o.kinds[i].name + " -> " + o.kinds[i].parent + "\n";
  s += "}\n";
  return s;
}

}  // namespace detail

// The script of an ontology's categorical claims, in kategoria's language.
inline std::string categorical_script(const Ontology& o) {
  std::string s = "# The categorical claims of the ontology " + o.name + ", version " + std::to_string(o.version) + ", generated.\n";
  s += "category Registers {\n  objects";
  for (std::size_t i = 0; i < o.registers.size(); i++) s += (i ? ", " : " ") + o.registers[i];
  s += "\n";
  for (std::size_t i = 0; i + 1 < o.registers.size(); i++) s += "  r" + std::to_string(i) + " : " + o.registers[i] + " -> " + o.registers[i + 1] + "\n";
  s += "}\ncheck Registers is thin\n";
  s += detail::categories_of(o, "");
  s += "check Kinds is thin\nfind terminal in Kinds\n";
  s += "functor InRegister : Kinds -> Registers {\n";
  for (const auto& k : o.kinds) s += "  " + k.name + " -> " + o.register_of(k.name) + "\n";
  s += "}\n";
  return s;
}

struct Verification {
  std::vector<Check> checks;
  Verdict verdict;
  bool admitted() const { return verdict.holds(); }
};

inline std::vector<Check> constraints(const Ontology& o) {
  std::vector<Check> out;
  {
    std::vector<std::string> names;
    for (const auto& k : o.kinds) names.push_back(k.name);
    std::string dup = detail::duplicated(names);
    if (dup.empty()) {
      names.clear();
      for (const auto& a : o.axes) names.push_back(a.name);
      dup = detail::duplicated(names);
    }
    if (dup.empty()) {
      names.clear();
      for (const auto& r : o.relations) names.push_back(r.name);
      dup = detail::duplicated(names);
    }
    if (dup.empty()) dup = detail::duplicated(o.registers);
    for (const auto& a : o.axes)
      if (dup.empty() && !detail::duplicated(a.values).empty()) dup = a.name + "'s value " + detail::duplicated(a.values);
    out.push_back(detail::check("names are distinct", dup.empty() ? Verdict::yes("names are distinct") : Verdict::no("names are distinct", dup + " is declared twice")));
  }
  {
    const std::string c = "there are registers, and every kind is in one, inherited or stated";
    std::string bad;
    if (o.registers.empty()) bad = "no registers are declared";
    for (const auto& k : o.kinds) {
      if (!bad.empty()) break;
      if (!k.register_.empty() && o.register_index(k.register_) < 0) bad = k.name + " is in " + k.register_ + ", which is not declared";
      else if (o.register_of(k.name).empty()) bad = k.name + " is in no register";
    }
    out.push_back(detail::check(c, bad.empty() ? Verdict::yes(c) : Verdict::no(c, bad)));
  }
  {
    const std::string c = "kinds form a forest without cycles";
    std::string bad;
    for (const auto& k : o.kinds) {
      if (!bad.empty()) break;
      if (!k.parent.empty() && !o.kind(k.parent)) bad = k.name + " is a kind of " + k.parent + ", which is not declared";
      std::string at = k.name;
      for (std::size_t steps = 0; bad.empty() && steps <= o.kinds.size() + 1; steps++) {
        const Kind* x = o.kind(at);
        if (!x || x->parent.empty()) break;
        at = x->parent;
        if (at == k.name) bad = k.name + " is beneath itself";
      }
    }
    out.push_back(detail::check(c, bad.empty() ? Verdict::yes(c) : Verdict::no(c, bad)));
  }
  {
    const std::string c = "a kind is never more about what obtains than the kind it is a kind of";
    std::string bad;
    for (const auto& k : o.kinds)
      if (bad.empty() && !k.parent.empty() && o.kind(k.parent) && o.register_index(o.register_of(k.name)) > o.register_index(o.register_of(k.parent)))
        bad = k.name + " (" + o.register_of(k.name) + ") is a kind of " + k.parent + " (" + o.register_of(k.parent) + ")";
    out.push_back(detail::check(c, bad.empty() ? Verdict::yes(c) : Verdict::no(c, bad)));
  }
  {
    const std::string c = "every axis has values and applies to declared kinds, required only where it applies";
    std::string bad;
    for (const auto& a : o.axes) {
      if (!bad.empty()) break;
      if (a.values.empty()) bad = a.name + " has no values";
      for (const auto& k : a.applies)
        if (bad.empty() && !o.kind(k)) bad = a.name + " applies to " + k + ", which is not declared";
      for (const auto& k : a.requires_) {
        if (!bad.empty()) break;
        if (!o.kind(k)) bad = a.name + " is required for " + k + ", which is not declared";
        else if (!o.applies(a, k)) bad = a.name + " is required for " + k + ", to which it does not apply";
      }
    }
    out.push_back(detail::check(c, bad.empty() ? Verdict::yes(c) : Verdict::no(c, bad)));
  }
  {
    const std::string c = "relations join declared kinds";
    std::string bad;
    for (const auto& r : o.relations)
      if (bad.empty() && (!o.kind(r.from) || !o.kind(r.to))) bad = r.name + " joins " + r.from + " to " + r.to + ", not both declared";
    out.push_back(detail::check(c, bad.empty() ? Verdict::yes(c) : Verdict::no(c, bad)));
  }
  return out;
}

// Every check of an ontology, and whether it is admitted. The categorical claims are asked only of a declaration whose constraints
// hold, so that kategoria is never asked about a forest that is not one.
inline Verification verify(const Ontology& o) {
  Verification v;
  v.checks = constraints(o);
  const bool sound = std::all_of(v.checks.begin(), v.checks.end(), [](const Check& c) { return c.verdict.holds(); });
  if (sound) {
    kategoria::script::Session session;
    const auto outcomes = session.run(categorical_script(o));
    v.checks.push_back(detail::check("the registers and the kinds are orders, the kinds have one root, and the register of a kind is a functor of their orders",
                                     detail::session_verdict(outcomes, "the categorical claims of the ontology"), "category"));
  } else {
    v.checks.push_back(detail::check("the categorical claims", Verdict::unknown("the categorical claims", "they need a declaration whose constraints hold"), "category"));
  }
  v.verdict = kernel::all_of(v.checks, "the ontology " + o.name + " (version " + std::to_string(o.version) + ") is admitted");
  return v;
}

// ---- Records of facts ---------------------------------------------------------------------------------------------------------------

struct Fact {
  std::string id, kind, text;
  std::map<std::string, std::string> axes;
  bool operator==(const Fact&) const = default;
};

struct Link {
  std::string relation, from, to;
  bool operator==(const Link&) const = default;
};

struct Record {
  std::vector<Fact> facts;
  std::vector<Link> links;
};

struct Problem {
  std::string where, what;
};

// Every way a record fails its ontology, each named: an unknown kind, an axis that does not apply or has no such value, a required
// axis missing, a link whose relation or ends do not fit.
inline std::vector<Problem> problems(const Ontology& o, const Record& r) {
  std::vector<Problem> out;
  std::map<std::string, const Fact*> by_id;
  for (const auto& f : r.facts) {
    if (f.id.empty()) out.push_back({"a fact", "it has no id"});
    else if (by_id.count(f.id)) out.push_back({"fact " + f.id, "its id is given twice"});
    by_id[f.id] = &f;
    if (!o.kind(f.kind)) {
      out.push_back({"fact " + f.id, "its kind " + f.kind + " is not in the ontology"});
      continue;
    }
    for (const auto& [axis, value] : f.axes) {
      const Axis* a = o.axis(axis);
      if (!a) out.push_back({"fact " + f.id, "it is described on " + axis + ", which is no axis of the ontology"});
      else if (!o.applies(*a, f.kind)) out.push_back({"fact " + f.id, axis + " does not apply to " + f.kind});
      else if (std::find(a->values.begin(), a->values.end(), value) == a->values.end()) out.push_back({"fact " + f.id, value + " is no value of " + axis});
    }
    for (const auto& a : o.axes)
      if (o.required(a, f.kind) && !f.axes.count(a.name)) out.push_back({"fact " + f.id, a.name + " is required for " + f.kind + " and not given"});
  }
  for (const auto& l : r.links) {
    const std::string where = "link " + l.from + " " + l.relation + " " + l.to;
    const Relation* rel = o.relation(l.relation);
    if (!rel) {
      out.push_back({where, l.relation + " is no relation of the ontology"});
      continue;
    }
    const auto a = by_id.find(l.from), b = by_id.find(l.to);
    if (a == by_id.end() || b == by_id.end()) {
      out.push_back({where, "it joins a fact the record does not hold"});
      continue;
    }
    if (!o.beneath(a->second->kind, rel->from)) out.push_back({where, a->second->kind + " is not a kind of " + rel->from + ", where " + l.relation + " begins"});
    if (!o.beneath(b->second->kind, rel->to)) out.push_back({where, b->second->kind + " is not a kind of " + rel->to + ", where " + l.relation + " ends"});
  }
  return out;
}

// Where a value stands on an ordered axis (0 lowest), or -1.
inline long rank(const Axis& a, const std::string& value) {
  for (std::size_t i = 0; i < a.values.size(); i++)
    if (a.values[i] == value) return static_cast<long>(i);
  return -1;
}

// ---- Revisions -----------------------------------------------------------------------------------------------------------------------

struct Mapping {
  std::string name;
  long from = 0, to = 0;
  std::map<std::string, std::string> kinds;                              // old -> new, where the name changes
  std::map<std::string, std::map<std::string, std::string>> values;     // axis -> old -> new
  std::map<std::string, std::string> relations;                          // old -> new
  std::set<std::string> dropped;                                         // relations dropped
};

struct MappingReading {
  Mapping mapping;
  std::vector<Fault> faults;
  bool ok() const { return faults.empty(); }
};

inline MappingReading read_mapping(std::string_view script) {
  MappingReading r;
  Mapping& m = r.mapping;
  bool header = false;
  detail::each_statement(script, r.faults, [&](const morphe::formal::ParseNode& n, std::size_t line) {
    auto text = [&](const char* c) { return detail::text_of(n, c); };
    if (n.name == "revision") {
      header = true;
      m.name = text("oname"), m.from = std::stol(text("vfrom")), m.to = std::stol(text("vto"));
    } else if (n.name == "mapkind") {
      m.kinds[text("old")] = text("new");
    } else if (n.name == "mapvalue") {
      m.values[text("xname")][text("old")] = text("new");
    } else if (n.name == "maprel") {
      m.relations[text("old")] = text("new");
    } else if (n.name == "droprel") {
      m.dropped.insert(text("old"));
    } else {
      r.faults.push_back({line, "a statement of an ontology in a revision: " + n.name});
    }
  });
  if (!header && r.faults.empty()) r.faults.push_back({1, "no revision statement (revision <Name> from <Integer> to <Integer>)"});
  return r;
}

inline std::string image_kind(const Mapping& m, const std::string& k) {
  const auto it = m.kinds.find(k);
  return it == m.kinds.end() ? k : it->second;
}

inline std::string image_value(const Mapping& m, const std::string& axis, const std::string& v) {
  const auto a = m.values.find(axis);
  if (a == m.values.end()) return v;
  const auto it = a->second.find(v);
  return it == a->second.end() ? v : it->second;
}

inline std::string image_relation(const Mapping& m, const std::string& r) {
  if (m.dropped.count(r)) return {};
  const auto it = m.relations.find(r);
  return it == m.relations.end() ? r : it->second;
}

struct Admission {
  std::vector<Check> checks;
  std::vector<std::string> changes;  // what the revision changes, by name
  Verdict verdict;
  bool admitted() const { return verdict.holds(); }
};

// Whether a revision of an ontology is admitted with its mapping, and what it changes.
inline Admission admit(const Ontology& from, const Ontology& to, const Mapping& m) {
  Admission a;
  const auto vf = verify(from), vt = verify(to);
  a.checks.push_back(detail::check("the ontology revised is admitted", vf.verdict.holds() ? Verdict::yes("the ontology revised is admitted") : Verdict::no("the ontology revised is admitted", vf.verdict.reason)));
  a.checks.push_back(detail::check("the revision is admitted as an ontology", vt.verdict.holds() ? Verdict::yes("the revision is admitted as an ontology") : Verdict::no("the revision is admitted as an ontology", vt.verdict.reason)));
  {
    const std::string c = "the mapping is between the two versions";
    const bool ok = from.name == to.name && m.from == from.version && m.to == to.version && to.version > from.version;
    a.checks.push_back(detail::check(c, ok ? Verdict::yes(c) : Verdict::no(c, "it maps " + std::to_string(m.from) + " to " + std::to_string(m.to) + "; the ontologies are " + from.name + " " +
                                                                                       std::to_string(from.version) + " and " + to.name + " " + std::to_string(to.version))));
  }
  {
    const std::string c = "every kind has an image, which keeps its register";
    std::string bad;
    for (const auto& k : from.kinds) {
      if (!bad.empty()) break;
      const std::string img = image_kind(m, k.name);
      if (!to.kind(img)) bad = k.name + " has no image (" + img + " is not in the revision)";
      else if (from.register_of(k.name) != to.register_of(img)) bad = k.name + " (" + from.register_of(k.name) + ") would become " + img + " (" + to.register_of(img) + ")";
    }
    a.checks.push_back(detail::check(c, bad.empty() ? Verdict::yes(c) : Verdict::no(c, bad)));
  }
  {
    const std::string c = "every value of an axis kept has an image, and an ordered axis keeps its order";
    std::string bad;
    for (const auto& ax : from.axes) {
      const Axis* nx = to.axis(ax.name);
      if (!nx || !bad.empty()) continue;
      long last = -1;
      for (const auto& v : ax.values) {
        const std::string img = image_value(m, ax.name, v);
        const long r = rank(*nx, img);
        if (r < 0) {
          bad = ax.name + "'s value " + v + " has no image (" + img + " is no value of it in the revision)";
          break;
        }
        if (ax.ordered && nx->ordered && r < last) {
          bad = ax.name + "'s order is not kept: " + v + " would fall below a value it stood above";
          break;
        }
        last = std::max(last, r);
      }
    }
    a.checks.push_back(detail::check(c, bad.empty() ? Verdict::yes(c) : Verdict::no(c, bad)));
  }
  {
    const std::string c = "every relation has an image whose ends contain the images of its own, or is dropped";
    std::string bad;
    for (const auto& r : from.relations) {
      if (!bad.empty()) break;
      const std::string img = image_relation(m, r.name);
      if (img.empty()) continue;
      const Relation* nr = to.relation(img);
      if (!nr) bad = r.name + " has no image (" + img + " is not in the revision)";
      else if (!to.beneath(image_kind(m, r.from), nr->from) || !to.beneath(image_kind(m, r.to), nr->to))
        bad = r.name + " (" + r.from + " -> " + r.to + ") would become " + img + " (" + nr->from + " -> " + nr->to + "), which does not contain it";
    }
    a.checks.push_back(detail::check(c, bad.empty() ? Verdict::yes(c) : Verdict::no(c, bad)));
  }
  const bool ready = std::all_of(a.checks.begin(), a.checks.end(), [](const Check& c) { return c.verdict.holds(); });
  if (ready) {
    // The map of kinds as a functor between the kinds' orders, decided by kategoria.
    std::string s = detail::categories_of(from, "Old") + detail::categories_of(to, "New") + "functor Revision : OldKinds -> NewKinds {\n";
    for (const auto& k : from.kinds) s += "  " + k.name + " -> " + image_kind(m, k.name) + "\n";
    s += "}\n";
    kategoria::script::Session session;
    a.checks.push_back(detail::check("the map of kinds is a functor of their orders", detail::session_verdict(session.run(s), "the map of kinds is a functor of their orders"), "category"));
  } else {
    a.checks.push_back(detail::check("the map of kinds is a functor of their orders", Verdict::unknown("the map of kinds is a functor of their orders", "it needs both ontologies admitted and every kind mapped"), "category"));
  }
  // What changes, by name.
  for (const auto& k : from.kinds)
    if (image_kind(m, k.name) != k.name) a.changes.push_back("the kind " + k.name + " becomes " + image_kind(m, k.name));
  for (const auto& k : to.kinds) {
    bool hit = false;
    for (const auto& x : from.kinds) hit = hit || image_kind(m, x.name) == k.name;
    if (!hit) a.changes.push_back("the kind " + k.name + " is added" + (k.parent.empty() ? "" : ", under " + k.parent));
  }
  for (const auto& ax : from.axes)
    if (!to.axis(ax.name)) a.changes.push_back("the axis " + ax.name + " is dropped");
  for (const auto& ax : to.axes)
    if (!from.axis(ax.name)) a.changes.push_back("the axis " + ax.name + " is added");
  for (const auto& [axis, vals] : m.values)
    for (const auto& [o, n] : vals) a.changes.push_back("on " + axis + ", " + o + " becomes " + n);
  for (const auto& r : m.dropped) a.changes.push_back("the relation " + r + " is dropped");
  for (const auto& [o, n] : m.relations) a.changes.push_back("the relation " + o + " becomes " + n);
  for (const auto& r : to.relations)
    if (!from.relation(r.name)) {
      bool hit = false;
      for (const auto& [o, n] : m.relations) hit = hit || n == r.name;
      if (!hit) a.changes.push_back("the relation " + r.name + " is added (" + r.from + " -> " + r.to + ")");
    }
  a.verdict = kernel::all_of(a.checks, "the revision of " + from.name + " from version " + std::to_string(from.version) + " to " + std::to_string(to.version) + " is admitted");
  return a;
}

struct Migrated {
  Record record;
  std::vector<std::string> kept, lost;  // what was carried changed, and what did not survive, by name
  std::vector<Problem> problems;        // what the carried record lacks under the revision
};

// A record carried along an admitted revision.
inline Migrated migrate(const Record& r, const Ontology& from, const Ontology& to, const Mapping& m) {
  Migrated out;
  for (const auto& f : r.facts) {
    Fact g;
    g.id = f.id, g.text = f.text, g.kind = from.kind(f.kind) ? image_kind(m, f.kind) : f.kind;
    if (g.kind != f.kind) out.kept.push_back("fact " + f.id + ": " + f.kind + " is now " + g.kind);
    for (const auto& [axis, value] : f.axes) {
      const Axis* nx = to.axis(axis);
      if (!nx) {
        out.lost.push_back("fact " + f.id + ": " + axis + " = " + value + " (the revision has no axis " + axis + ")");
        continue;
      }
      if (!to.applies(*nx, g.kind)) {
        out.lost.push_back("fact " + f.id + ": " + axis + " = " + value + " (the revision does not describe " + g.kind + " on it)");
        continue;
      }
      const std::string img = image_value(m, axis, value);
      if (img != value) out.kept.push_back("fact " + f.id + ": " + axis + " " + value + " is now " + img);
      g.axes[axis] = img;
    }
    out.record.facts.push_back(std::move(g));
  }
  for (const auto& l : r.links) {
    const std::string img = image_relation(m, l.relation);
    if (img.empty()) {
      out.lost.push_back("link " + l.from + " " + l.relation + " " + l.to + " (the relation is dropped)");
      continue;
    }
    if (img != l.relation) out.kept.push_back("link " + l.from + " " + l.relation + " " + l.to + " is now " + img);
    out.record.links.push_back({img, l.from, l.to});
  }
  out.problems = problems(to, out.record);
  return out;
}

}  // namespace kybernesis::ontology
