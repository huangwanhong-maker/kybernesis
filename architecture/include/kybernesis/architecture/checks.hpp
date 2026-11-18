// The checks of an architecture: the constraints on its declaration, and the claims about it that kategoria answers
// (decision 0002). An architecture, or a revision of one, is admitted exactly when every check holds.
//
// Constraints (decided here, each with the place where it fails):
//   - it is named and versioned; names are distinct; everything referred to is declared;
//   - every stage has a declared register, and the stages declared are those ordered;
//   - kinds form a forest without cycles, each with a register (its own or inherited);
//   - every block yields a declared kind that is no example, at a declared stage, and in that stage's register;
//   - a boundary has outputs and no inputs; any other block has both (what it emerged from, and what it acts on);
//   - every output port's type lies in its block's register;
//   - wires join an output to an input of the same type; every input is fed by exactly one wire, and every output feeds one
//     at least (efficacy: nothing is computed that nothing uses);
//   - a wire going forward within a turn reaches the next stage, or a later stage of its own register: in the standard
//     stages, nothing in the record is computed from the system except through what is observed (no wire from before to
//     recorded), and the system is reached only through what is decided (no wire from observed to after). This is a property
//     of paths, not of functors: a functor into the stages cannot forbid a wire that skips a stage, since the composite arrow
//     it would be sent to exists;
//   - the wires not marked next form no cycle (causality within a turn: every cycle passes through the next turn);
//   - laws are among those known (the blocks module decides them of methods); a referred block lies in the highest register.
//
// Categorical claims (a script generated from the architecture and answered by kategoria's session):
//   - Registers, the order of aboutness, is a thin category;
//   - Kinds is a thin category with a terminal object, and InRegister, each kind sent to its register, is a functor
//     Kinds → Registers (a kind is never in a register below its parent's in the order of aboutness... read as arrows: a kind's
//     register has an arrow to its parent's);
//   - Stages, the order of a turn, is a thin category;
//   - Step, the free category on the blocks and the wires not marked next, has StageOf, each block sent to its stage, as a
//     functor Step → Stages: every wire within a turn goes forward or stays (no wire runs back against the order of a turn).
//
// The categorical checks run only on a declaration whose constraints of form hold (names declared, kinds acyclic, wires
// acyclic within a turn); otherwise they are undecided, with the reason.
#pragma once

#include <kybernesis/architecture/model.hpp>
#include <kybernesis/kernel/verdicts.hpp>

#include <kategoria/script/session.hpp>

#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace kybernesis::architecture {

using kernel::Check;
using kernel::Verdict;

// The laws the blocks module knows how to decide of a method. A revision may add a law here and its checker there.
inline const std::vector<std::string>& known_laws() {
  static const std::vector<std::string> laws{"total", "deterministic", "sound", "referred"};
  return laws;
}

namespace detail {

inline Check check(std::string name, Verdict v, std::string kind = "constraint") { return {std::move(name), std::move(v), std::move(kind)}; }

template <class F>
Check rule(const std::string& name, F&& find_fault) {
  const std::string fault = find_fault();
  return check(name, fault.empty() ? Verdict::yes(name) : Verdict::no(name, fault));
}

inline std::string duplicated(const std::vector<std::string>& names) {
  std::set<std::string> seen;
  for (const auto& n : names)
    if (!seen.insert(n).second) return n;
  return {};
}

// Whether the wires not marked next form a cycle; the first block found on one.
inline std::string cycle_within_turn(const Architecture& a) {
  std::map<std::string, std::vector<std::string>> next;
  for (const auto& w : a.wires)
    if (!w.next) next[w.from].push_back(w.to);
  std::map<std::string, int> state;  // 0 unseen, 1 on the stack, 2 done
  std::string found;
  std::function<bool(const std::string&)> visit = [&](const std::string& v) {
    state[v] = 1;
    for (const auto& u : next[v]) {
      if (state[u] == 1) return found = u, true;
      if (state[u] == 0 && visit(u)) return true;
    }
    state[v] = 2;
    return false;
  };
  for (const auto& b : a.blocks)
    if (state[b.name] == 0 && visit(b.name)) return found;
  return {};
}

inline std::string kind_cycle(const Architecture& a) {
  for (const auto& k : a.kinds) {
    std::string at = k.parent;
    for (std::size_t steps = 0; !at.empty(); steps++) {
      if (at == k.name || steps > a.kinds.size()) return k.name;
      const Kind* p = a.kind(at);
      if (!p) break;
      at = p->parent;
    }
  }
  return {};
}

}  // namespace detail

// The constraints on the declaration.
inline std::vector<Check> constraints(const Architecture& a) {
  using detail::rule;
  std::vector<Check> out;
  out.push_back(rule("the architecture is named and versioned", [&]() -> std::string {
    if (a.name.empty()) return "it has no name";
    if (a.version < 1) return "its version is not a positive integer";
    return {};
  }));
  out.push_back(rule("names are distinct", [&]() -> std::string {
    if (auto d = detail::duplicated(a.registers); !d.empty()) return "the register " + d + " is declared twice";
    if (auto d = detail::duplicated(a.stage_order); !d.empty()) return "the stage " + d + " is ordered twice";
    std::vector<std::string> names;
    for (const auto& s : a.stages) names.push_back(s.name);
    if (auto d = detail::duplicated(names); !d.empty()) return "the stage " + d + " is declared twice";
    names.clear();
    for (const auto& k : a.kinds) names.push_back(k.name);
    if (auto d = detail::duplicated(names); !d.empty()) return "the kind " + d + " is declared twice";
    names.clear();
    for (const auto& t : a.types) names.push_back(t.name);
    if (auto d = detail::duplicated(names); !d.empty()) return "the type " + d + " is declared twice";
    names.clear();
    for (const auto& b : a.blocks) names.push_back(b.name);
    if (auto d = detail::duplicated(names); !d.empty()) return "the block " + d + " is declared twice";
    for (const auto& b : a.blocks) {
      std::vector<std::string> in, out_;
      for (const auto& p : b.inputs) in.push_back(p.name);
      for (const auto& p : b.outputs) out_.push_back(p.name);
      if (auto d = detail::duplicated(in); !d.empty()) return "the block " + b.name + " has two inputs named " + d;
      if (auto d = detail::duplicated(out_); !d.empty()) return "the block " + b.name + " has two outputs named " + d;
    }
    return {};
  }));
  out.push_back(rule("there are registers and stages, each stage ordered and in a declared register", [&]() -> std::string {
    if (a.registers.empty()) return "no registers are declared";
    if (a.stage_order.empty()) return "no stages are ordered";
    for (const auto& s : a.stage_order)
      if (!a.stage(s)) return "the stage " + s + " is ordered but has no register (stage " + s + " in <Register>)";
    for (const auto& s : a.stages) {
      if (!a.stage_index(s.name)) return "the stage " + s.name + " is declared but not ordered";
      if (!a.register_index(s.register_)) return "the stage " + s.name + " lies in " + s.register_ + ", which is no declared register";
    }
    return {};
  }));
  out.push_back(rule("kinds form a forest without cycles, each in a declared register", [&]() -> std::string {
    if (a.kinds.empty()) return "no kinds are declared";
    for (const auto& k : a.kinds) {
      if (!k.parent.empty() && !a.kind(k.parent)) return "the kind " + k.name + " is under " + k.parent + ", which is not declared";
      if (!k.register_.empty() && !a.register_index(k.register_)) return "the kind " + k.name + " lies in " + k.register_ + ", which is no declared register";
      if (k.example && k.parent.empty()) return "the example " + k.name + " is under no kind";
      if (const Kind* p = a.kind(k.parent); p && p->example) return "the kind " + k.name + " is under the example " + p->name + ": examples are leaves, declared with their operators";
    }
    if (auto c = detail::kind_cycle(a); !c.empty()) return "the kind " + c + " lies under itself";
    for (const auto& k : a.kinds)
      if (a.register_of(k.name).empty()) return "the kind " + k.name + " has no register, its own or inherited";
    return {};
  }));
  out.push_back(rule("every type lies in a declared register", [&]() -> std::string {
    for (const auto& t : a.types)
      if (!a.register_index(t.register_)) return "the type " + t.name + " lies in " + t.register_ + ", which is no declared register";
    return {};
  }));
  out.push_back(rule("every block yields a declared kind that is no example, at a declared stage", [&]() -> std::string {
    if (a.blocks.empty()) return "no blocks are declared";
    for (const auto& b : a.blocks) {
      const Kind* k = a.kind(b.yields);
      if (!k) return "the block " + b.name + " yields " + b.yields + ", which is not declared";
      if (k->example) return "the block " + b.name + " yields the example " + b.yields + ": examples are what operators yield, not elements of the loop";
      if (!a.stage(b.stage)) return "the block " + b.name + " is at " + b.stage + ", which is no declared stage";
    }
    return {};
  }));
  out.push_back(rule("every block yields a kind in its stage's register", [&]() -> std::string {
    for (const auto& b : a.blocks) {
      const Stage* s = a.stage(b.stage);
      if (!s || !a.kind(b.yields)) continue;  // reported above
      const std::string r = a.register_of(b.yields);
      if (r != s->register_) return "the block " + b.name + " yields " + b.yields + " (" + r + ") at " + b.stage + " (" + s->register_ + ")";
    }
    return {};
  }));
  out.push_back(rule("every port has a declared type, and every output's type lies in its block's register", [&]() -> std::string {
    for (const auto& b : a.blocks) {
      for (const auto& p : b.inputs)
        if (!a.type(p.type)) return "the input " + b.name + "." + p.name + " has the type " + p.type + ", which is not declared";
      const Stage* s = a.stage(b.stage);
      for (const auto& p : b.outputs) {
        const Type* t = a.type(p.type);
        if (!t) return "the output " + b.name + "." + p.name + " has the type " + p.type + ", which is not declared";
        if (s && t->register_ != s->register_) return "the output " + b.name + "." + p.name + " is of " + p.type + " (" + t->register_ + "), and its block is in " + s->register_;
      }
    }
    return {};
  }));
  out.push_back(rule("a boundary has outputs and no inputs; every other block has inputs and outputs", [&]() -> std::string {
    for (const auto& b : a.blocks) {
      if (b.boundary && !b.inputs.empty()) return "the boundary " + b.name + " has inputs: what enters the loop from outside it takes nothing from the loop";
      if (b.outputs.empty()) return "the block " + b.name + " has no output: it acts on nothing";
      if (!b.boundary && b.inputs.empty()) return "the block " + b.name + " has no input: it emerges from nothing (declare it a boundary if it enters from outside the loop)";
    }
    return {};
  }));
  out.push_back(rule("wires join an output to an input of the same type", [&]() -> std::string {
    for (const auto& w : a.wires) {
      if (!a.block(w.from)) return "the wire " + w.words() + " leaves " + w.from + ", which is no block";
      if (!a.block(w.to)) return "the wire " + w.words() + " enters " + w.to + ", which is no block";
      const std::string t1 = a.port_type(w.from, w.out, false), t2 = a.port_type(w.to, w.in, true);
      if (t1.empty()) return "the wire " + w.words() + " leaves " + w.from + "." + w.out + ", which is no output";
      if (t2.empty()) return "the wire " + w.words() + " enters " + w.to + "." + w.in + ", which is no input";
      if (t1 != t2) return "the wire " + w.words() + " carries " + t1 + " into an input of " + t2;
    }
    return {};
  }));
  out.push_back(rule("every input is fed by exactly one wire", [&]() -> std::string {
    for (const auto& b : a.blocks)
      for (const auto& p : b.inputs) {
        std::size_t n = 0;
        for (const auto& w : a.wires) n += w.to == b.name && w.in == p.name;
        if (n == 0) return "the input " + b.name + "." + p.name + " is fed by no wire";
        if (n > 1) return "the input " + b.name + "." + p.name + " is fed by " + std::to_string(n) + " wires";
      }
    return {};
  }));
  out.push_back(rule("every output feeds a wire", [&]() -> std::string {
    for (const auto& b : a.blocks)
      for (const auto& p : b.outputs) {
        bool used = false;
        for (const auto& w : a.wires) used = used || (w.from == b.name && w.out == p.name);
        if (!used) return "the output " + b.name + "." + p.name + " feeds nothing";
      }
    return {};
  }));
  out.push_back(rule("a wire going forward within a turn reaches the next stage, or a later stage of its own register", [&]() -> std::string {
    for (const auto& w : a.wires) {
      if (w.next) continue;
      const Block *from = a.block(w.from), *to = a.block(w.to);
      if (!from || !to) continue;  // reported above
      const auto i = a.stage_index(from->stage), j = a.stage_index(to->stage);
      const Stage *si = a.stage(from->stage), *sj = a.stage(to->stage);
      if (!i || !j || !si || !sj || *j <= *i + 1) continue;  // backwards, the same stage or the next: StageOf and the order of a turn judge those
      if (si->register_ != sj->register_)
        return "the wire " + w.words() + " passes from " + from->stage + " (" + si->register_ + ") to " + to->stage + " (" + sj->register_ + "), over the stages between";
    }
    return {};
  }));
  out.push_back(rule("the wires within a turn form no cycle", [&]() -> std::string {
    if (auto c = detail::cycle_within_turn(a); !c.empty()) return "a cycle through " + c + " does not pass through the next turn (mark one of its wires next)";
    return {};
  }));
  out.push_back(rule("every law is known, and a referred block lies in the highest register", [&]() -> std::string {
    for (const auto& b : a.blocks)
      for (const auto& l : b.laws) {
        if (std::find(known_laws().begin(), known_laws().end(), l) == known_laws().end()) return "the block " + b.name + " obeys the law " + l + ", which no checker knows";
        if (l == "referred") {
          const Stage* s = a.stage(b.stage);
          if (s && !a.registers.empty() && s->register_ != a.registers.back())
            return "the block " + b.name + " is referred to, and lies in " + s->register_ + ", not in " + a.registers.back();
        }
      }
    return {};
  }));
  return out;
}

namespace detail {

inline std::string quoted(const std::string& s) { return "\"" + s + "\""; }

}  // namespace detail

// The script of the categorical claims, as kategoria's language writes them.
inline std::string categorical_script(const Architecture& a) {
  std::string s = "# The categorical claims of the architecture " + a.name + ", version " + std::to_string(a.version) + ", generated.\n";
  s += "category Registers {\n  objects";
  for (std::size_t i = 0; i < a.registers.size(); i++) s += (i ? ", " : " ") + a.registers[i];
  s += "\n";
  for (std::size_t i = 0; i + 1 < a.registers.size(); i++) s += "  r" + std::to_string(i) + " : " + a.registers[i] + " -> " + a.registers[i + 1] + "\n";
  s += "}\ncheck Registers is thin\n";

  s += "category Kinds {\n  objects";
  for (std::size_t i = 0; i < a.kinds.size(); i++) s += (i ? ", " : " ") + a.kinds[i].name;
  s += "\n";
  for (std::size_t i = 0; i < a.kinds.size(); i++)
    if (!a.kinds[i].parent.empty()) s += "  k" + std::to_string(i) + " : " + a.kinds[i].name + " -> " + a.kinds[i].parent + "\n";
  s += "}\ncheck Kinds is thin\nfind terminal in Kinds\n";
  s += "functor InRegister : Kinds -> Registers {\n";
  for (const auto& k : a.kinds) s += "  " + k.name + " -> " + a.register_of(k.name) + "\n";
  s += "}\n";

  s += "category Stages {\n  objects";
  for (std::size_t i = 0; i < a.stage_order.size(); i++) s += (i ? ", " : " ") + a.stage_order[i];
  s += "\n";
  for (std::size_t i = 0; i + 1 < a.stage_order.size(); i++) s += "  s" + std::to_string(i) + " : " + a.stage_order[i] + " -> " + a.stage_order[i + 1] + "\n";
  s += "}\ncheck Stages is thin\n";

  s += "category Step {\n  objects";
  for (std::size_t i = 0; i < a.blocks.size(); i++) s += (i ? ", " : " ") + a.blocks[i].name;
  s += "\n";
  for (std::size_t i = 0; i < a.wires.size(); i++) {
    const auto& w = a.wires[i];
    if (w.next || w.from == w.to) continue;
    s += "  w" + std::to_string(i) + " : " + w.from + " -> " + w.to + "\n";
  }
  s += "}\n";
  s += "functor StageOf : Step -> Stages {\n";
  for (const auto& b : a.blocks) s += "  " + b.name + " -> " + b.stage + "\n";
  s += "}\n";
  return s;
}

// The categorical claims, answered by kategoria's session. Undecided when the declaration is not well formed enough for them.
inline std::vector<Check> categorical(const Architecture& a, const std::vector<Check>& constraint_checks) {
  static const std::vector<std::string> needed{"names are distinct", "there are registers and stages, each stage ordered and in a declared register",
                                               "kinds form a forest without cycles, each in a declared register",
                                               "every block yields a declared kind that is no example, at a declared stage", "wires join an output to an input of the same type",
                                               "the wires within a turn form no cycle"};
  for (const auto& c : constraint_checks)
    if (std::find(needed.begin(), needed.end(), c.name) != needed.end() && !c.verdict.holds())
      return {detail::check("the categorical claims", Verdict::unknown("the categorical claims", "they need a declaration whose names, kinds and wires are well formed: " + c.name + " does not hold"), "category")};
  kategoria::script::Session session;
  const auto outcomes = session.run(categorical_script(a));
  std::vector<Check> out;
  for (const auto& o : outcomes.items()) {
    const auto* status = o.get("status");
    const auto* claim = o.get("claim");
    const auto* reason = o.get("reason");
    const std::string st = status ? status->text() : "undecided", cl = claim ? claim->text() : o.get("text") ? o.get("text")->text() : "a statement";
    const std::string why = reason ? reason->text() : "";
    Verdict v = st == "holds" ? Verdict::yes(cl, why) : st == "fails" ? Verdict::no(cl, why) : Verdict::unknown(cl, why.empty() ? st : why);
    out.push_back(detail::check(cl, v, "category"));
  }
  return out;
}

struct Verification {
  std::vector<Check> checks;
  Verdict verdict;
  bool admitted() const { return verdict.holds(); }
};

// Every check of an architecture, and whether it is admitted.
inline Verification verify(const Architecture& a) {
  Verification v;
  v.checks = constraints(a);
  const auto cat = categorical(a, v.checks);
  v.checks.insert(v.checks.end(), cat.begin(), cat.end());
  v.verdict = kernel::all_of(v.checks, "the architecture " + a.name + " (version " + std::to_string(a.version) + ") is admitted");
  return v;
}

}  // namespace kybernesis::architecture
