// The record of a steered system: its trajectory, every value with how it came to be (decision 0004).
//
// A run of the loop (loop/run.hpp) fills every block with an admitted method. A real steering is not like that: most of what
// it knows is entered by people (a workshop's needs, an engineer's estimate, officials' levels on a scorecard), what the world
// does is observed and never computed, and computed values are amended, accepted or refused by those with the authority to.
// The record keeps all of it, so that every value says which block it belongs to, at which turn and on which branch, whether a
// method made it or a person, from what, why, with which files, and under which version of the architecture.
//
//   entries      a value of a block at a turn, made by one of five acts:
//                  method    an admitted method applied to the block's inputs;
//                  observe   taken in from outside the loop, at a boundary or at a block the world feeds (Collection); the
//                            world itself (a referred block) never has an entry: it is observed, not computed;
//                  fill      entered by a person where no method runs;
//                  amend     a method's value changed by a person, the computed value kept beside it;
//                  review    a value accepted or refused before it passes on; a proposal passes only through a review.
//   checks       what a person enters is held to what can be held: its type (a check per type, given with the trajectory);
//                a block's bound where one is given (an estimate that excludes every state the evidence allows is refused,
//                one that excludes some is entered with the remark); and its authority: an entry counts as the record's only
//                where the rules empower the person's role for that act at that block, and is otherwise kept as a proposal.
//   files        documents, tables, images and recordings named by the SHA-256 of their bytes; the bytes are the work's to keep.
//   branches     an alternative taken from any turn (another reading, another option, another standard) is a branch: it shares
//                the turns before it with its parent and holds its own from there, never written over the main line.
//   sealing      every act (and the opening, each initial value, each file, each branch, each revision) is sealed in archeion's
//                hash-chained journal; verify() recomputes every digest and the chain, and names the first fault.
//   revisiting   the value at a turn; the difference between two turns or branches; what rests on an entry, a file or an agent;
//                which entries are stale (an input replaced since); replay (computed values recomputed from their recorded
//                inputs); lineage as W3C PROV (archeion's provenance graph).
//
// Values are text: each type's payload is a canonical text, kept once under its digest (content addressing). The finite
// values of an instance are one such text (finite.hpp); a work may record whatever its blocks carry. The record keeps no
// clock: every act is given its instant, ISO 8601 in UTC, and instants never go back (archeion's rule).
#pragma once

#include <kybernesis/architecture/language.hpp>
#include <kybernesis/architecture/model.hpp>
#include <kybernesis/kernel/verdicts.hpp>

#include <archeion/fixity/journal.hpp>
#include <archeion/fixity/sha256.hpp>
#include <archeion/provenance/provenance.hpp>

#include <algorithm>
#include <cstdint>
#include <exception>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace kybernesis::record {

using kernel::Family;
using kernel::Verdict;

enum class Act { method, observe, fill, amend, review };

inline const char* name(Act a) {
  switch (a) {
    case Act::method: return "method";
    case Act::observe: return "observe";
    case Act::fill: return "fill";
    case Act::amend: return "amend";
    default: return "review";
  }
}

inline Act act_of(const std::string& s) {
  for (Act a : {Act::method, Act::observe, Act::fill, Act::amend, Act::review})
    if (s == name(a)) return a;
  throw std::invalid_argument("kybernesis::record: no act named " + s);
}

struct File {
  std::string name, digest, kind;  // digest: the SHA-256 of its bytes, hexadecimal; kind: a media type or a word
  std::uint64_t bytes = 0;
};

struct Entry {
  std::size_t id = 0;              // its place in the record, from 0
  std::string branch = "main";
  std::size_t turn = 0;
  std::string block;
  Act act = Act::method;
  std::string by, role;            // the method's name, or the person or source; a person's role
  Family family = Family::declared;
  std::string reason;
  std::vector<long> fed_by;        // per input port: the entry that supplied it; -1 for an initial value or none
  std::vector<std::string> inputs; // per input port: the payload's digest (empty when not available to a person)
  std::vector<std::string> outputs;// per output port: the payload's digest (none for a refusing review)
  std::vector<std::string> computed;// amend: the outputs the person changed
  long target = -1;                // amend, review: the entry acted on
  bool accepted = true;            // review: accepted or refused
  std::vector<std::string> files;  // digests
  std::string architecture;        // the digest of the architecture's printed script when it was made
  std::string when;
  bool counts = true;              // the record's; false: a proposal, its author not empowered, awaiting a review
  std::vector<std::string> remarks;
  std::string digest;              // of the canonical text of all the above
};

struct Grant {
  std::string block;  // a block's name, or "*" for every block
  Act act = Act::fill;
  std::string role;   // a role, or "*" for every role
};

struct Rules {
  std::vector<Grant> grants;
  bool allows(const std::string& block, Act act, const std::string& role) const {
    for (const auto& g : grants)
      if ((g.block == block || g.block == "*") && g.act == act && (g.role == role || g.role == "*")) return true;
    return false;
  }
};

// A branch is an overlay on its parent as the parent stood when the branch was taken: it reads every turn from its parent's
// entries made before it (`since`), and holds entries of its own from turn `from` on, which take precedence. A change in a
// branch makes what was inherited after it stale there, and a turn run on the branch recomputes it there, never in the parent.
struct Branch {
  std::string name, parent;  // the main line has no parent
  std::size_t from = 0;      // the first turn it may hold entries of its own
  std::size_t since = 0;     // the number of entries the record held when it was taken: the parent as it then stood
  std::string reason, by, when;
};

// A type's well-formedness; a block's bound on what a person enters (given the inputs when they are available, and the
// outputs); a method as the record sees it, on payloads.
using Check = std::function<Verdict(const std::string& payload)>;
using Bound = std::function<Verdict(const std::vector<std::string>& inputs, const std::vector<std::string>& outputs)>;
using Step = std::function<std::vector<std::string>(const std::vector<std::string>&)>;

struct Method {
  std::string name, wraps;
  Family family = Family::declared;
  Step apply;
};

struct Trajectory {
  architecture::Architecture arch;
  std::string arch_digest;
  Rules rules;
  std::map<std::string, Check> types;          // by type name; a type without a check takes any text
  std::map<std::string, Bound> bounds;         // by block name
  std::map<std::string, std::string> initial;  // a next wire's words -> its first value's digest
  std::vector<Branch> branches;                // the main line first
  std::vector<Entry> entries;                  // only ever appended to
  std::map<std::string, std::string> values;   // digest -> payload
  std::map<std::string, File> files;           // digest -> file
  archeion::fixity::Journal journal;
};

// What an act answered: its verdict and, when something was recorded, the entry's place.
struct Outcome {
  Verdict verdict;
  long entry = -1;
};

namespace detail {

inline void plain(const std::string& field, const std::string& what) {
  for (char c : field)
    if (c == '\x1e' || c == '\x1f') throw std::invalid_argument("kybernesis::record: " + what + " may not hold a separator (0x1E or 0x1F)");
}

inline std::string join(const std::vector<std::string>& xs) {
  std::string s;
  for (std::size_t i = 0; i < xs.size(); i++) s += (i ? "\x1e" : "") + xs[i];
  return s;
}

inline std::string family_word(Family f) { return kernel::name(f); }

}  // namespace detail

inline std::string digest_of(std::string_view payload) { return archeion::fixity::sha256_hex(payload); }

// The canonical text of an entry: its fields in a fixed order, each followed by the unit separator, lists joined by the
// record separator. Its digest is the entry's.
inline std::string canonical(const Entry& e) {
  std::vector<std::string> fed;
  for (long k : e.fed_by) fed.push_back(std::to_string(k));
  const std::vector<std::string> fields{std::to_string(e.id), e.branch, std::to_string(e.turn), e.block, name(e.act), e.by, e.role, detail::family_word(e.family),
                                        e.reason, detail::join(fed), detail::join(e.inputs), detail::join(e.outputs), detail::join(e.computed), std::to_string(e.target),
                                        e.accepted ? "accepted" : "refused", detail::join(e.files), e.architecture, e.when, e.counts ? "counts" : "proposal", detail::join(e.remarks)};
  std::string s;
  for (const auto& f : fields) s += f + '\x1f';
  return s;
}

inline std::string canonical(const Branch& b) {
  return b.name + '\x1f' + b.parent + '\x1f' + std::to_string(b.from) + '\x1f' + std::to_string(b.since) + '\x1f' + b.reason + '\x1f' + b.by + '\x1f' + b.when + '\x1f';
}

inline std::string canonical(const File& f) { return f.name + '\x1f' + f.digest + '\x1f' + f.kind + '\x1f' + std::to_string(f.bytes) + '\x1f'; }

namespace detail {

inline void seal(Trajectory& t, const std::string& when, const std::string& who, const std::string& what, const std::string& about, const std::string& note) {
  if (!t.journal.entries.empty() && when < t.journal.entries.back().when)
    throw std::invalid_argument("an act at " + when + " is earlier than the record's last (" + t.journal.entries.back().when + ")");
  t.journal.append({when, who, what, about, note, {}});
}

}  // namespace detail

// A trajectory opened on an architecture, under rules of authority, with the checks of its types and the bounds of its blocks.
inline Trajectory open(const architecture::Architecture& a, Rules rules, std::map<std::string, Check> types, std::map<std::string, Bound> bounds, const std::string& by,
                       const std::string& when) {
  Trajectory t;
  t.arch = a;
  t.arch_digest = digest_of(architecture::print(a));
  t.rules = std::move(rules);
  t.types = std::move(types);
  t.bounds = std::move(bounds);
  t.branches.push_back({"main", "", 0, 0, "the main line", by, when});
  detail::plain(by, "an agent's name");
  detail::seal(t, when, by, "open", a.name + " version " + std::to_string(a.version), t.arch_digest);
  return t;
}

// A payload kept under its digest (once, however often it recurs).
inline std::string keep(Trajectory& t, std::string payload) {
  std::string d = digest_of(payload);
  t.values.emplace(d, std::move(payload));
  return d;
}

inline const std::string& payload(const Trajectory& t, const std::string& digest) {
  const auto it = t.values.find(digest);
  if (it == t.values.end()) throw std::invalid_argument("kybernesis::record: no value kept under " + digest);
  return it->second;
}

// The first value of a wire carried to the next turn.
inline Verdict initial(Trajectory& t, const std::string& wire_words, std::string payload, const std::string& by, const std::string& when) {
  const std::string claim = "the first value of " + wire_words + " is recorded";
  const architecture::Wire* wire = nullptr;
  for (const auto& w : t.arch.wires)
    if (w.next && w.words() == wire_words) wire = &w;
  if (!wire) return Verdict::no(claim, "the architecture has no wire carried to the next turn named " + wire_words);
  const architecture::Block* from = t.arch.block(wire->from);
  std::string type;
  for (const auto& p : from->outputs)
    if (p.name == wire->out) type = p.type;
  if (const auto it = t.types.find(type); it != t.types.end())
    if (const auto v = it->second(payload); v.fails()) return Verdict::no(claim, "it is no value of " + type + ": " + v.reason);
  try {
    const std::string d = keep(t, std::move(payload));
    detail::seal(t, when, by, "initial", wire_words, d);
    t.initial[wire_words] = d;
    return Verdict::yes(claim, d);
  } catch (const std::exception& e) {
    return Verdict::no(claim, e.what());
  }
}

// A file named by its digest (the work keeps its bytes); the digest is computed from the bytes when they are given.
inline Verdict attach(Trajectory& t, File f, const std::string& by, const std::string& when) {
  const std::string claim = "the file " + f.name + " is recorded";
  try {
    detail::plain(f.name, "a file's name");
    detail::plain(f.kind, "a file's kind");
    if (f.digest.size() != 64) return Verdict::no(claim, "a file is named by the 64 hexadecimal digits of its SHA-256");
    if (t.files.count(f.digest)) return Verdict::yes(claim, "already recorded");
    detail::seal(t, when, by, "file", f.digest, digest_of(canonical(f)));
    t.files[f.digest] = f;
    return Verdict::yes(claim, f.digest);
  } catch (const std::exception& e) {
    return Verdict::no(claim, e.what());
  }
}

inline File file_of(const std::string& name, std::string_view bytes, const std::string& kind = {}) {
  return {name, archeion::fixity::sha256_hex(bytes), kind, static_cast<std::uint64_t>(bytes.size())};
}

// ---- Reading the record ------------------------------------------------------------------------------------------------------

inline const Branch* branch_named(const Trajectory& t, const std::string& name) {
  for (const auto& b : t.branches)
    if (b.name == name) return &b;
  return nullptr;
}

// Whether a branch may hold entries of its own at a turn.
inline bool holds_turn(const Trajectory& t, const std::string& branch, std::size_t turn) {
  const Branch* b = branch_named(t, branch);
  if (!b) throw std::invalid_argument("kybernesis::record: no branch named " + branch);
  return b->parent.empty() || turn >= b->from;
}

// Calls f(branch, limit) for a branch and its ancestors, each with the number of entries visible from it (all for the branch
// read, and for each ancestor the entries made before the branch below it was taken), until f answers true.
template <class F>
inline void lineage_of_branches(const Trajectory& t, std::string branch, F f) {
  std::size_t limit = t.entries.size();
  for (std::size_t steps = 0; steps <= t.branches.size(); steps++) {
    const Branch* b = branch_named(t, branch);
    if (!b) throw std::invalid_argument("kybernesis::record: no branch named " + branch);
    if (f(*b, limit)) return;
    if (b->parent.empty()) return;
    limit = std::min(limit, b->since);
    branch = b->parent;
  }
  throw std::invalid_argument("kybernesis::record: the branches form a cycle");
}

// The entry that holds a block's value at a turn, read from a branch: the latest that counts in the branch, else what it
// inherits. A refusing review holds the block: it is current, and has no value.
inline const Entry* current(const Trajectory& t, const std::string& branch, std::size_t turn, const std::string& block) {
  const Entry* found = nullptr;
  lineage_of_branches(t, branch, [&](const Branch& b, std::size_t limit) {
    if (!(b.parent.empty() || turn >= b.from)) return false;
    for (std::size_t k = limit; k-- > 0;) {
      const Entry& e = t.entries[k];
      if (e.counts && e.turn == turn && e.block == block && e.branch == b.name) {
        found = &e;
        return true;
      }
    }
    return false;
  });
  return found;
}

inline bool holds_value(const Entry* e) { return e && !(e->act == Act::review && !e->accepted); }

// The last turn with an entry, read from a branch.
inline std::optional<std::size_t> last_turn(const Trajectory& t, const std::string& branch) {
  std::optional<std::size_t> out;
  lineage_of_branches(t, branch, [&](const Branch& b, std::size_t limit) {
    for (std::size_t k = 0; k < limit; k++) {
      const Entry& e = t.entries[k];
      if (e.branch == b.name && (b.parent.empty() || e.turn >= b.from) && (!out || e.turn > *out)) out = e.turn;
    }
    return false;
  });
  return out;
}

inline std::size_t port_index(const std::vector<architecture::Port>& ports, const std::string& n) {
  for (std::size_t i = 0; i < ports.size(); i++)
    if (ports[i].name == n) return i;
  throw std::invalid_argument("kybernesis::record: no port named " + n);
}

inline bool referred(const architecture::Block& b) { return b.has_law("referred"); }

// The inputs of a block at a turn: their digests and the entries that supplied them, or why they are not available.
struct Inputs {
  std::vector<std::string> digests;
  std::vector<long> fed_by;
  std::string missing;       // empty when every input is available
  bool from_world = false;   // some input is fed by a referred block: the block is observed, not computed
};

inline Inputs inputs_for(const Trajectory& t, const std::string& branch, std::size_t turn, const std::string& block) {
  Inputs in;
  const architecture::Block* b = t.arch.block(block);
  if (!b) throw std::invalid_argument("kybernesis::record: no block named " + block);
  for (const auto& p : b->inputs) {
    const architecture::Wire* feed = nullptr;
    for (const auto& w : t.arch.wires)
      if (w.to == block && w.in == p.name) feed = &w;
    std::string d;
    long by = -1;
    if (!feed) {
      if (in.missing.empty()) in.missing = "nothing feeds the input " + p.name;
    } else if (referred(*t.arch.block(feed->from))) {
      in.from_world = true;
      if (in.missing.empty()) in.missing = "its input " + p.name + " is fed by " + feed->from + ", the world, which the record observes and never computes";
    } else if (feed->next && turn == 0) {
      if (const auto it = t.initial.find(feed->words()); it != t.initial.end()) d = it->second;
      else if (in.missing.empty()) in.missing = "no first value of " + feed->words();
    } else {
      const Entry* e = current(t, branch, feed->next ? turn - 1 : turn, feed->from);
      if (holds_value(e)) {
        d = e->outputs.at(port_index(t.arch.block(feed->from)->outputs, feed->out));
        by = static_cast<long>(e->id);
      } else if (in.missing.empty()) {
        in.missing = std::string(e ? "the value of " + feed->from + " is held by a review that refused it" : "no value of " + feed->from) + (feed->next ? " at the turn before" : "");
      }
    }
    in.digests.push_back(d);
    in.fed_by.push_back(by);
  }
  return in;
}

// Whether an entry, read from a branch (its own by default), rests on inputs that have since been replaced, and by what.
inline std::optional<std::string> stale(const Trajectory& t, const Entry& e, const std::string& branch = {}) {
  const std::string from = branch.empty() ? e.branch : branch;
  for (std::size_t k = 0; k < e.fed_by.size(); k++) {
    if (e.fed_by[k] < 0) continue;
    const Entry& src = t.entries.at(static_cast<std::size_t>(e.fed_by[k]));
    const Entry* now = current(t, from, src.turn, src.block);
    if (!now || now->id != src.id)
      return "its input from " + src.block + " (entry " + std::to_string(src.id) + ") has since been " +
             (now ? (holds_value(now) ? "replaced by entry " + std::to_string(now->id) : "refused by entry " + std::to_string(now->id)) : "withdrawn");
  }
  return std::nullopt;
}

// ---- Acting on the record ---------------------------------------------------------------------------------------------------

namespace detail {

inline Verdict typed(const Trajectory& t, const std::vector<architecture::Port>& ports, const std::vector<std::string>& payloads) {
  if (payloads.size() != ports.size()) return Verdict::no("typed", std::to_string(payloads.size()) + " values for " + std::to_string(ports.size()) + " outputs");
  for (std::size_t k = 0; k < ports.size(); k++)
    if (const auto it = t.types.find(ports[k].type); it != t.types.end())
      if (const auto v = it->second(payloads[k]); v.fails()) return Verdict::no("typed", "the output " + ports[k].name + " is no value of " + ports[k].type + ": " + v.reason);
  return Verdict::yes("typed");
}

inline Outcome append(Trajectory& t, Entry e, const std::vector<std::string>& outputs, const std::string& claim) {
  try {
    for (const auto* f : {&e.by, &e.role, &e.reason, &e.when, &e.branch, &e.block}) plain(*f, "a field of an entry");
    for (const auto& r : e.remarks) plain(r, "a remark");
    for (const auto& f : e.files)
      if (!t.files.count(f)) return {Verdict::no(claim, "the file " + f + " is not recorded: attach it first"), -1};
    e.outputs.clear();
    for (const auto& p : outputs) e.outputs.push_back(keep(t, p));
    e.id = t.entries.size();
    e.architecture = t.arch_digest;
    e.digest = digest_of(canonical(e));
    seal(t, e.when, e.by, name(e.act), e.branch + "/" + std::to_string(e.turn) + "/" + e.block, e.digest);
    t.entries.push_back(e);
    std::string why = "entry " + std::to_string(e.id);
    for (const auto& r : e.remarks) why += "; " + r;
    return {e.counts ? Verdict::yes(claim, why) : Verdict::unknown(claim, why), static_cast<long>(e.id)};
  } catch (const std::exception& x) {
    return {Verdict::no(claim, x.what()), -1};
  }
}

inline std::vector<std::string> payloads(const Trajectory& t, const std::vector<std::string>& digests) {
  std::vector<std::string> out;
  for (const auto& d : digests) out.push_back(d.empty() ? std::string() : payload(t, d));
  return out;
}

// The person's entry held to the block's bound, when one is given and the inputs are available.
inline Verdict bounded(const Trajectory& t, const std::string& block, const Inputs& in, const std::vector<std::string>& outputs) {
  const auto it = t.bounds.find(block);
  if (it == t.bounds.end() || !in.missing.empty()) return Verdict::yes("bounded");
  return it->second(payloads(t, in.digests), outputs);
}

}  // namespace detail

// A method applied to a block's inputs at a turn.
inline Outcome apply(Trajectory& t, const std::string& branch, std::size_t turn, const std::string& block, const Method& m, const std::string& when) {
  const std::string claim = m.name + " fills " + block + " at turn " + std::to_string(turn) + " of " + branch;
  const architecture::Block* b = t.arch.block(block);
  if (!b) return {Verdict::no(claim, "the architecture has no block " + block), -1};
  if (referred(*b)) return {Verdict::no(claim, block + " is the world's: it is never filled from the record"), -1};
  if (!branch_named(t, branch)) return {Verdict::no(claim, "no branch named " + branch), -1};
  if (!holds_turn(t, branch, turn)) return {Verdict::no(claim, "turn " + std::to_string(turn) + " comes before the branch was taken: take a branch from an earlier turn"), -1};
  const Inputs in = inputs_for(t, branch, turn, block);
  if (!in.missing.empty()) return {Verdict::no(claim, in.missing), -1};
  std::vector<std::string> out;
  try {
    out = m.apply(detail::payloads(t, in.digests));
  } catch (const std::exception& x) {
    return {Verdict::no(claim, std::string("the method throws: ") + x.what()), -1};
  }
  if (const auto v = detail::typed(t, b->outputs, out); v.fails()) return {Verdict::no(claim, v.reason), -1};
  Entry e;
  e.branch = branch, e.turn = turn, e.block = block, e.act = Act::method, e.by = m.name, e.family = m.family, e.when = when;
  e.reason = m.wraps.empty() ? "" : "wraps " + m.wraps;
  e.fed_by = in.fed_by, e.inputs = in.digests;
  return detail::append(t, e, out, claim);
}

namespace detail {

inline Outcome personal(Trajectory& t, Act act, const std::string& branch, std::size_t turn, const std::string& block, const std::string& by, const std::string& role,
                        const std::vector<std::string>& outputs, const std::string& reason, const std::vector<std::string>& files, const std::string& when, long target,
                        std::vector<std::string> computed, const std::string& claim) {
  const architecture::Block* b = t.arch.block(block);
  if (!b) return {Verdict::no(claim, "the architecture has no block " + block), -1};
  if (referred(*b)) return {Verdict::no(claim, block + " is the world's: it is observed where it feeds the loop, never entered"), -1};
  if (!branch_named(t, branch)) return {Verdict::no(claim, "no branch named " + branch), -1};
  if (!holds_turn(t, branch, turn)) return {Verdict::no(claim, "turn " + std::to_string(turn) + " comes before the branch was taken: take a branch from an earlier turn"), -1};
  if (by.empty()) return {Verdict::no(claim, "an act is made by someone: name them"), -1};
  if (const auto v = typed(t, b->outputs, outputs); v.fails()) return {Verdict::no(claim, v.reason), -1};
  const Inputs in = inputs_for(t, branch, turn, block);
  if (act == Act::observe && !(b->boundary || in.from_world))
    return {Verdict::no(claim, "only what enters at a boundary, or what the world feeds, is observed; " + block + " is neither"), -1};
  Entry e;
  e.branch = branch, e.turn = turn, e.block = block, e.act = act, e.by = by, e.role = role, e.reason = reason, e.files = files, e.when = when, e.target = target;
  e.computed = std::move(computed);
  if (in.missing.empty()) e.fed_by = in.fed_by, e.inputs = in.digests;
  if (act != Act::observe) {
    const Verdict bound = bounded(t, block, in, outputs);
    if (bound.fails()) return {Verdict::no(claim, "the block's bound refuses it: " + bound.reason), -1};
    if (bound.undecided()) e.remarks.push_back(bound.reason);
  }
  e.counts = t.rules.allows(block, act, role);
  e.family = e.counts ? Family::declared : Family::proposal;
  if (!e.counts) e.remarks.push_back("a proposal: the role " + (role.empty() ? std::string("(none)") : role) + " may not " + name(act) + " " + block + "; it awaits a review");
  return append(t, e, outputs, claim);
}

}  // namespace detail

// Values taken in from outside the loop: at a boundary, or at a block the world feeds.
inline Outcome observe(Trajectory& t, const std::string& branch, std::size_t turn, const std::string& block, const std::string& by, const std::string& role,
                       const std::vector<std::string>& outputs, const std::string& reason, const std::vector<std::string>& files, const std::string& when) {
  return detail::personal(t, Act::observe, branch, turn, block, by, role, outputs, reason, files, when, -1, {},
                          by + " observes " + block + " at turn " + std::to_string(turn) + " of " + branch);
}

// A block's value entered by a person.
inline Outcome fill(Trajectory& t, const std::string& branch, std::size_t turn, const std::string& block, const std::string& by, const std::string& role,
                    const std::vector<std::string>& outputs, const std::string& reason, const std::vector<std::string>& files, const std::string& when) {
  return detail::personal(t, Act::fill, branch, turn, block, by, role, outputs, reason, files, when, -1, {},
                          by + " fills " + block + " at turn " + std::to_string(turn) + " of " + branch);
}

// A current value, read from a branch (the entry's own by default), changed by a person in that branch; the value it replaces
// is kept beside it.
inline Outcome amend(Trajectory& t, std::size_t target, const std::string& by, const std::string& role, const std::vector<std::string>& outputs, const std::string& reason,
                     const std::vector<std::string>& files, const std::string& when, const std::string& in_branch = {}) {
  const std::string claim = by + " amends entry " + std::to_string(target);
  if (target >= t.entries.size()) return {Verdict::no(claim, "no entry " + std::to_string(target)), -1};
  const Entry e = t.entries[target];
  const std::string branch = in_branch.empty() ? e.branch : in_branch;
  if (!branch_named(t, branch)) return {Verdict::no(claim, "no branch named " + branch), -1};
  const Entry* now = current(t, branch, e.turn, e.block);
  if (!now || now->id != e.id || !holds_value(now)) return {Verdict::no(claim, "entry " + std::to_string(target) + " is not the current value of " + e.block + " in " + branch), -1};
  return detail::personal(t, Act::amend, branch, e.turn, e.block, by, role, outputs, reason, files, when, static_cast<long>(target), e.outputs, claim);
}

// A value accepted or refused, in a branch (the entry's own by default). The current value refused holds its block; a
// proposal accepted becomes the record's.
inline Outcome review(Trajectory& t, std::size_t target, const std::string& by, const std::string& role, bool accept, const std::string& reason, const std::string& when,
                      const std::string& in_branch = {}) {
  const std::string claim = by + (accept ? " accepts" : " refuses") + " entry " + std::to_string(target);
  if (target >= t.entries.size()) return {Verdict::no(claim, "no entry " + std::to_string(target)), -1};
  const Entry e = t.entries[target];
  const std::string branch = in_branch.empty() ? e.branch : in_branch;
  if (!branch_named(t, branch)) return {Verdict::no(claim, "no branch named " + branch), -1};
  if (!holds_turn(t, branch, e.turn)) return {Verdict::no(claim, "turn " + std::to_string(e.turn) + " comes before the branch " + branch + " was taken"), -1};
  if (!t.rules.allows(e.block, Act::review, role)) return {Verdict::no(claim, "the role " + role + " may not review " + e.block), -1};
  const Entry* now = current(t, branch, e.turn, e.block);
  const bool proposal = !e.counts;
  if (!proposal && (!now || now->id != e.id || !holds_value(now))) return {Verdict::no(claim, "entry " + std::to_string(target) + " is neither the current value of " + e.block + " nor a proposal"), -1};
  if (e.act == Act::review) return {Verdict::no(claim, "a review is not reviewed"), -1};
  Entry r;
  r.branch = branch, r.turn = e.turn, r.block = e.block, r.act = Act::review, r.by = by, r.role = role, r.reason = reason, r.when = when;
  r.target = static_cast<long>(target), r.accepted = accept, r.family = Family::declared;
  r.fed_by = e.fed_by, r.inputs = e.inputs;
  r.counts = accept || !proposal;  // refusing a proposal leaves the current value as it was
  if (accept && proposal)
    if (const auto st = stale(t, e, branch)) r.remarks.push_back("the proposal " + *st);
  return detail::append(t, r, accept ? detail::payloads(t, e.outputs) : std::vector<std::string>{}, claim);
}

// A branch taken from `from` at a turn: it reads the turns before from its parent and holds its own from there.
inline Verdict branch(Trajectory& t, const std::string& from, std::size_t at, const std::string& name, const std::string& reason, const std::string& by, const std::string& when) {
  const std::string claim = "the branch " + name + " is taken from " + from + " at turn " + std::to_string(at);
  if (!branch_named(t, from)) return Verdict::no(claim, "no branch named " + from);
  if (branch_named(t, name) || name.empty()) return Verdict::no(claim, name.empty() ? "a branch is named" : "a branch named " + name + " exists");
  const auto last = last_turn(t, from);
  if (at > (last ? *last + 1 : 0)) return Verdict::no(claim, "the branch " + from + " has no turn before " + std::to_string(at));
  Branch b{name, from, at, t.entries.size(), reason, by, when};
  try {
    detail::plain(name, "a branch's name");
    detail::plain(reason, "a branch's reason");
    detail::seal(t, when, by, "branch", name, digest_of(canonical(b)));
  } catch (const std::exception& x) {
    return Verdict::no(claim, x.what());
  }
  t.branches.push_back(b);
  return Verdict::yes(claim);
}

// The architecture revised (the constitutional loop): entries made after carry the new digest. Admission of the revision is
// the architecture's own (architecture::verify); the record keeps that it happened, by whom and why.
inline Verdict revise(Trajectory& t, const architecture::Architecture& a, const std::string& by, const std::string& reason, const std::string& when) {
  const std::string claim = "the architecture is revised to " + a.name + " version " + std::to_string(a.version);
  const std::string d = digest_of(architecture::print(a));
  try {
    detail::plain(reason, "a revision's reason");
    detail::seal(t, when, by, "revise", a.name + " version " + std::to_string(a.version), d);
  } catch (const std::exception& x) {
    return Verdict::no(claim, x.what());
  }
  t.arch = a;
  t.arch_digest = d;
  return Verdict::yes(claim, reason);
}

// ---- Running a turn ----------------------------------------------------------------------------------------------------------

// The blocks in an order in which every block comes after those that feed it within a turn.
inline std::vector<std::string> order(const architecture::Architecture& a) {
  std::map<std::string, std::size_t> indegree;
  for (const auto& b : a.blocks) indegree[b.name] = 0;
  for (const auto& w : a.wires)
    if (!w.next && w.from != w.to) indegree[w.to]++;
  std::vector<std::string> out;
  std::vector<char> done(a.blocks.size(), 0);
  while (out.size() < a.blocks.size()) {
    bool moved = false;
    for (std::size_t i = 0; i < a.blocks.size(); i++) {
      if (done[i] || indegree[a.blocks[i].name] != 0) continue;
      done[i] = 1, moved = true;
      out.push_back(a.blocks[i].name);
      for (const auto& w : a.wires)
        if (!w.next && w.from == a.blocks[i].name && w.from != w.to) indegree[w.to]--;
      break;
    }
    if (!moved) break;
  }
  return out;
}

struct Progress {
  std::vector<std::size_t> made;                          // entries appended
  std::vector<std::pair<std::string, std::string>> waiting;  // blocks without a value, and why
  std::vector<std::pair<std::size_t, std::string>> stale;    // persons' entries resting on replaced inputs, and why
};

// One turn of a branch: every block without a current value, or whose current value a method made from inputs since replaced,
// is filled by its method where one is given and its inputs are available. A person's entry is never recomputed: when its
// inputs have changed, it is reported stale, for its author to look at again.
inline Progress run_turn(Trajectory& t, const std::string& branch, std::size_t turn, const std::map<std::string, Method>& methods, const std::string& when) {
  Progress p;
  for (const auto& name : order(t.arch)) {
    const architecture::Block& b = *t.arch.block(name);
    if (referred(b)) continue;
    const Entry* c = current(t, branch, turn, name);
    const auto st = c ? stale(t, *c, branch) : std::nullopt;
    if (c && !st) continue;
    if (c && c->act != Act::method) {
      p.stale.push_back({c->id, *st});
      continue;
    }
    const auto m = methods.find(name);
    const Inputs in = inputs_for(t, branch, turn, name);
    if (m == methods.end()) {
      p.waiting.push_back({name, in.from_world ? in.missing : "no method fills it and no one has entered it"});
      continue;
    }
    if (!in.missing.empty()) {
      p.waiting.push_back({name, in.missing});
      continue;
    }
    const Outcome o = apply(t, branch, turn, name, m->second, when);
    if (o.entry >= 0) p.made.push_back(static_cast<std::size_t>(o.entry));
    else p.waiting.push_back({name, o.verdict.reason});
  }
  return p;
}

// Every turn of a branch from `from` on, run again, so that a change reaches all that a method made after it.
inline Progress propagate(Trajectory& t, const std::string& branch, std::size_t from, const std::map<std::string, Method>& methods, const std::string& when) {
  Progress all;
  const auto last = last_turn(t, branch);
  if (!last) return all;
  for (std::size_t turn = from; turn <= *last; turn++) {
    if (!holds_turn(t, branch, turn)) continue;
    Progress p = run_turn(t, branch, turn, methods, when);
    all.made.insert(all.made.end(), p.made.begin(), p.made.end());
    all.waiting.insert(all.waiting.end(), p.waiting.begin(), p.waiting.end());
    all.stale.insert(all.stale.end(), p.stale.begin(), p.stale.end());
  }
  return all;
}

// ---- Verifying, replaying, revisiting ---------------------------------------------------------------------------------------

// Every digest recomputed (entries, values, files, branches), the journal's chain verified, and the journal's acts matched to
// the record's entries in order. Fails at the first fault, named.
inline Verdict verify(const Trajectory& t) {
  const std::string claim = "the record verifies";
  if (const auto v = archeion::fixity::verify(t.journal); v.fails()) return Verdict::no(claim, "the journal: " + v.reason);
  for (const auto& [d, p] : t.values)
    if (digest_of(p) != d) return Verdict::no(claim, "the value kept under " + d.substr(0, 12) + "… is not the value of that digest");
  for (const auto& [d, f] : t.files)
    if (f.digest != d) return Verdict::no(claim, "the file " + f.name + " is kept under another digest");
  std::size_t next_entry = 0, next_branch = 1;
  for (const auto& j : t.journal.entries) {
    if (j.what == "open" || j.what == "initial" || j.what == "file" || j.what == "revise") {
      if (j.what == "initial" && !t.values.count(j.note)) return Verdict::no(claim, "the first value of " + j.about + " is not kept");
      if (j.what == "file" && !t.files.count(j.about)) return Verdict::no(claim, "the file " + j.about.substr(0, 12) + "… sealed in the journal is not recorded");
      continue;
    }
    if (j.what == "branch") {
      if (next_branch >= t.branches.size()) return Verdict::no(claim, "the journal seals a branch the record does not hold: " + j.about);
      const Branch& b = t.branches[next_branch++];
      if (b.name != j.about || digest_of(canonical(b)) != j.note) return Verdict::no(claim, "the branch " + b.name + " is not the one the journal sealed");
      continue;
    }
    if (next_entry >= t.entries.size()) return Verdict::no(claim, "the journal seals an act the record does not hold (" + j.what + " " + j.about + ")");
    const Entry& e = t.entries[next_entry];
    if (e.id != next_entry) return Verdict::no(claim, "entry " + std::to_string(next_entry) + " is out of its place");
    if (digest_of(canonical(e)) != e.digest) return Verdict::no(claim, "entry " + std::to_string(e.id) + " has been altered since it was made");
    if (e.digest != j.note || j.what != name(e.act)) return Verdict::no(claim, "entry " + std::to_string(e.id) + " is not the act the journal sealed in its place");
    for (const auto& d : e.inputs)
      if (!d.empty() && !t.values.count(d)) return Verdict::no(claim, "an input of entry " + std::to_string(e.id) + " is not kept");
    for (const auto& d : e.outputs)
      if (!t.values.count(d)) return Verdict::no(claim, "an output of entry " + std::to_string(e.id) + " is not kept");
    for (long k : e.fed_by)
      if (k >= static_cast<long>(e.id)) return Verdict::no(claim, "entry " + std::to_string(e.id) + " is fed by an entry made after it");
    for (const auto& f : e.files)
      if (!t.files.count(f)) return Verdict::no(claim, "a file of entry " + std::to_string(e.id) + " is not recorded");
    next_entry++;
  }
  if (next_entry != t.entries.size()) return Verdict::no(claim, "entry " + std::to_string(next_entry) + " was never sealed in the journal");
  if (next_branch != t.branches.size()) return Verdict::no(claim, "the branch " + t.branches[next_branch].name + " was never sealed in the journal");
  return Verdict::yes(claim, std::to_string(t.entries.size()) + " entries, " + std::to_string(t.journal.entries.size()) + " acts sealed, head " + t.journal.head().substr(0, 12) + "…");
}

struct Replayed {
  std::size_t entry;
  Verdict verdict;
};

// Every value a method made, recomputed from its recorded inputs by the method of that name: holds when every output is the
// same, fails naming the first output that differs, undecided when no method of that name is given.
inline std::vector<Replayed> replay(const Trajectory& t, const std::map<std::string, Method>& methods) {
  std::vector<Replayed> out;
  for (const auto& e : t.entries) {
    if (e.act != Act::method) continue;
    const std::string claim = "entry " + std::to_string(e.id) + " (" + e.block + " by " + e.by + ") replays";
    const auto m = methods.find(e.block);
    if (m == methods.end() || m->second.name != e.by) {
      out.push_back({e.id, Verdict::unknown(claim, "no method " + e.by + " is given for " + e.block)});
      continue;
    }
    std::vector<std::string> again;
    try {
      again = m->second.apply(detail::payloads(t, e.inputs));
    } catch (const std::exception& x) {
      out.push_back({e.id, Verdict::no(claim, std::string("the method now throws: ") + x.what())});
      continue;
    }
    const auto& ports = t.arch.block(e.block) ? t.arch.block(e.block)->outputs : std::vector<architecture::Port>{};
    std::string differs;
    if (again.size() != e.outputs.size()) differs = "it now answers " + std::to_string(again.size()) + " values";
    for (std::size_t k = 0; differs.empty() && k < again.size(); k++)
      if (digest_of(again[k]) != e.outputs[k]) differs = "its output " + (k < ports.size() ? ports[k].name : std::to_string(k)) + " is no longer the value recorded";
    out.push_back({e.id, differs.empty() ? Verdict::yes(claim) : Verdict::no(claim, differs)});
  }
  return out;
}

// The value of a block's output at a turn, read from a branch.
inline std::optional<std::string> value(const Trajectory& t, const std::string& branch, std::size_t turn, const std::string& block, const std::string& port) {
  const Entry* e = current(t, branch, turn, block);
  if (!holds_value(e)) return std::nullopt;
  return payload(t, e->outputs.at(port_index(t.arch.block(block)->outputs, port)));
}

struct Change {
  std::string block, port;
  std::optional<std::string> before, after;  // digests
  long before_entry = -1, after_entry = -1;
};

// What differs between a turn read from one branch and a turn read from another (or another turn of the same).
inline std::vector<Change> diff(const Trajectory& t, const std::string& branch_a, std::size_t turn_a, const std::string& branch_b, std::size_t turn_b) {
  std::vector<Change> out;
  for (const auto& b : t.arch.blocks) {
    if (referred(b)) continue;
    const Entry* ea = current(t, branch_a, turn_a, b.name);
    const Entry* eb = current(t, branch_b, turn_b, b.name);
    for (std::size_t k = 0; k < b.outputs.size(); k++) {
      std::optional<std::string> da, db;
      if (holds_value(ea)) da = ea->outputs[k];
      if (holds_value(eb)) db = eb->outputs[k];
      if (da != db) out.push_back({b.name, b.outputs[k].name, da, db, ea ? static_cast<long>(ea->id) : -1, eb ? static_cast<long>(eb->id) : -1});
    }
  }
  return out;
}

// Everything that rests on some entries: the entries fed by them, or acting on them, and so on.
inline std::set<std::size_t> resting_on(const Trajectory& t, const std::set<std::size_t>& start) {
  std::set<std::size_t> out = start;
  for (const auto& e : t.entries) {
    if (out.count(e.id)) continue;
    bool rests = e.target >= 0 && out.count(static_cast<std::size_t>(e.target));
    for (long k : e.fed_by) rests = rests || (k >= 0 && out.count(static_cast<std::size_t>(k)));
    if (rests) out.insert(e.id);
  }
  return out;
}

inline std::set<std::size_t> resting_on_file(const Trajectory& t, const std::string& digest) {
  std::set<std::size_t> s;
  for (const auto& e : t.entries)
    if (std::find(e.files.begin(), e.files.end(), digest) != e.files.end()) s.insert(e.id);
  return resting_on(t, s);
}

inline std::set<std::size_t> resting_on_agent(const Trajectory& t, const std::string& agent) {
  std::set<std::size_t> s;
  for (const auto& e : t.entries)
    if (e.by == agent) s.insert(e.id);
  return resting_on(t, s);
}

// The record as W3C PROV: each entry an activity, each of its outputs an entity it generated (named e<entry>.<port>), each
// file an entity, each first value an entity; an activity used what fed it, the files it names and, for an amendment or a
// review, the outputs of the entry it acted on; its agent is who made it.
inline archeion::provenance::Graph lineage(const Trajectory& t) {
  archeion::provenance::Graph g;
  for (const auto& [words, d] : t.initial) g.entities.push_back({"initial:" + words, {{"sha256", d}}});
  for (const auto& [d, f] : t.files) g.entities.push_back({"file:" + d, {{"sha256", d}}});
  auto entity_of = [&](std::size_t entry, std::size_t port) { return "e" + std::to_string(entry) + "." + std::to_string(port); };
  for (const auto& e : t.entries) {
    archeion::provenance::Activity a;
    a.id = "entry:" + std::to_string(e.id), a.kind = name(e.act), a.agent = e.by, a.when = e.when;
    const architecture::Block* b = t.arch.block(e.block);
    for (std::size_t k = 0; k < e.fed_by.size(); k++) {
      if (e.fed_by[k] >= 0) {
        const Entry& src = t.entries[static_cast<std::size_t>(e.fed_by[k])];
        // the output port of the feeding entry: the wire into this input
        std::size_t port = 0;
        if (b)
          for (const auto& w : t.arch.wires)
            if (w.to == e.block && k < b->inputs.size() && w.in == b->inputs[k].name && w.from == src.block && t.arch.block(src.block)) port = port_index(t.arch.block(src.block)->outputs, w.out);
        a.used.push_back(entity_of(src.id, port));
      } else if (!e.inputs.empty() && k < e.inputs.size() && !e.inputs[k].empty()) {
        for (const auto& [words, d] : t.initial)
          if (d == e.inputs[k] && b && k < b->inputs.size() && words.find("-> " + e.block + "." + b->inputs[k].name) != std::string::npos) a.used.push_back("initial:" + words);
      }
    }
    if (e.target >= 0)
      for (std::size_t k = 0; k < t.entries[static_cast<std::size_t>(e.target)].outputs.size(); k++) a.used.push_back(entity_of(static_cast<std::size_t>(e.target), k));
    for (const auto& f : e.files) a.used.push_back("file:" + f);
    for (std::size_t k = 0; k < e.outputs.size(); k++) {
      g.entities.push_back({entity_of(e.id, k), {{"sha256", e.outputs[k]}}});
      a.generated.push_back(entity_of(e.id, k));
    }
    std::sort(a.used.begin(), a.used.end());
    a.used.erase(std::unique(a.used.begin(), a.used.end()), a.used.end());
    g.activities.push_back(std::move(a));
  }
  return g;
}

}  // namespace kybernesis::record
