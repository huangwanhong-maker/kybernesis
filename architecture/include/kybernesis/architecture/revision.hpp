// Revisions of an architecture: what changed between two versions, and whether the new one is admitted (decision 0002).
//
// The architecture is meant to change: blocks added, removed, split or merged; laws added; wires moved; kinds and types
// renamed. A revision is the pair (before, after). It is admitted exactly when the after verifies (checks.hpp) and its
// version is greater than the before's; what it changes is reported by name, so that a reader sees the revision as a
// difference and the methods filling the blocks can be carried over or refused (blocks/admission.hpp, carry).
//
// Records made under the before move to the after by kategoria's data migration (the pullback and the left Kan extension
// along a map of schemas, as the law domain's records do); that step needs the record schema of both versions and is the
// constitution's (planned), not this header's.
#pragma once

#include <kybernesis/architecture/checks.hpp>

#include <string>
#include <vector>

namespace kybernesis::architecture {

struct Difference {
  std::vector<std::string> blocks_added, blocks_removed, blocks_changed;
  std::vector<std::string> wires_added, wires_removed;
  std::vector<std::string> kinds_added, kinds_removed, types_added, types_removed;
  bool empty() const {
    return blocks_added.empty() && blocks_removed.empty() && blocks_changed.empty() && wires_added.empty() && wires_removed.empty() && kinds_added.empty() &&
           kinds_removed.empty() && types_added.empty() && types_removed.empty();
  }
};

inline Difference difference(const Architecture& before, const Architecture& after) {
  Difference d;
  for (const auto& b : after.blocks) {
    const Block* old = before.block(b.name);
    if (!old) d.blocks_added.push_back(b.name);
    else if (!(*old == b)) d.blocks_changed.push_back(b.name);
  }
  for (const auto& b : before.blocks)
    if (!after.block(b.name)) d.blocks_removed.push_back(b.name);
  auto has_wire = [](const Architecture& a, const Wire& w) {
    for (const auto& x : a.wires)
      if (x == w) return true;
    return false;
  };
  for (const auto& w : after.wires)
    if (!has_wire(before, w)) d.wires_added.push_back(w.words());
  for (const auto& w : before.wires)
    if (!has_wire(after, w)) d.wires_removed.push_back(w.words());
  for (const auto& k : after.kinds)
    if (!before.kind(k.name)) d.kinds_added.push_back(k.name);
  for (const auto& k : before.kinds)
    if (!after.kind(k.name)) d.kinds_removed.push_back(k.name);
  for (const auto& t : after.types)
    if (!before.type(t.name)) d.types_added.push_back(t.name);
  for (const auto& t : before.types)
    if (!after.type(t.name)) d.types_removed.push_back(t.name);
  return d;
}

inline std::string words(const Difference& d) {
  std::string s;
  auto list = [&](const char* what, const std::vector<std::string>& xs) {
    if (xs.empty()) return;
    s += (s.empty() ? "" : "; ") + std::string(what) + ": ";
    for (std::size_t i = 0; i < xs.size(); i++) s += (i ? ", " : "") + xs[i];
  };
  list("blocks added", d.blocks_added);
  list("blocks removed", d.blocks_removed);
  list("blocks changed", d.blocks_changed);
  list("wires added", d.wires_added);
  list("wires removed", d.wires_removed);
  list("kinds added", d.kinds_added);
  list("kinds removed", d.kinds_removed);
  list("types added", d.types_added);
  list("types removed", d.types_removed);
  return s.empty() ? "nothing changed" : s;
}

struct Revision {
  Difference difference;
  Verification after;
  Verdict verdict;
};

inline Revision revise(const Architecture& before, const Architecture& after) {
  Revision r;
  r.difference = difference(before, after);
  r.after = verify(after);
  const std::string claim = "the revision of " + before.name + " from version " + std::to_string(before.version) + " to version " + std::to_string(after.version) + " is admitted";
  if (after.version <= before.version) r.verdict = Verdict::no(claim, "the version does not increase");
  else if (r.difference.empty()) r.verdict = Verdict::no(claim, "nothing changed");
  else if (!r.after.admitted()) r.verdict = Verdict::no(claim, "the revised architecture is not admitted: " + r.after.verdict.reason).with("changes", words(r.difference));
  else r.verdict = Verdict::yes(claim, words(r.difference));
  return r;
}

}  // namespace kybernesis::architecture
