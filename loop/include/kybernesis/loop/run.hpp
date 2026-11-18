// The loop run: an admitted architecture, a method in each block, turned on an instance (decision 0003).
//
// The executor knows no block by name. It takes any architecture that verifies (architecture/checks.hpp) and an assignment of
// a method to every block whose admission did not fail (blocks/admission.hpp), so that a revised architecture runs as soon as
// it is admitted and its methods carried over.
//
// One turn: the blocks in an order of the wires not marked next (a topological order, ties in the order of declaration); each
// block's inputs read from the wires that feed them, from this turn, or from the last turn for a wire marked next (the first
// turn reads the initial values given for those wires); its method applied; a finite output that is a law drawn from a seeded
// stream (calx::random), so that a run is the same wherever it is repeated. Every value is recorded, with its standing and
// measure, wire by wire and turn by turn.
#pragma once

#include <kybernesis/architecture/checks.hpp>
#include <kybernesis/blocks/admission.hpp>
#include <kybernesis/blocks/method.hpp>

#include <calx/random/stream.hpp>

#include <map>
#include <string>
#include <vector>

namespace kybernesis::loop {

using blocks::Instance;
using blocks::Method;
using blocks::Value;
using kernel::Verdict;

struct Turn {
  std::map<std::string, std::vector<Value>> outputs;  // block -> its outputs, in its order
};

struct Run {
  std::vector<std::string> order;  // the blocks in the order applied
  std::vector<Turn> turns;
  Verdict verdict;                 // holds when every turn was run

  // The value an output port of a block took at a turn.
  const Value& value(std::size_t turn, const architecture::Architecture& a, const std::string& block, const std::string& port) const {
    return turns.at(turn).outputs.at(block).at(blocks::port(a.block(block)->outputs, port));
  }
};

namespace detail {

inline std::size_t draw(const kernel::Distribution& p, double u) {
  double c = 0;
  for (std::size_t i = 0; i < p.size(); i++) {
    c += p[i];
    if (u < c) return i;
  }
  for (std::size_t i = p.size(); i-- > 0;)
    if (p[i] > 0) return i;
  return 0;
}

inline std::vector<std::string> topological(const architecture::Architecture& a) {
  std::map<std::string, std::size_t> indegree;
  for (const auto& b : a.blocks) indegree[b.name] = 0;
  for (const auto& w : a.wires)
    if (!w.next && w.from != w.to) indegree[w.to]++;
  std::vector<std::string> order;
  std::vector<char> done(a.blocks.size(), 0);
  while (order.size() < a.blocks.size()) {
    bool moved = false;
    for (std::size_t i = 0; i < a.blocks.size(); i++) {
      if (done[i] || indegree[a.blocks[i].name] != 0) continue;
      done[i] = 1, moved = true;
      order.push_back(a.blocks[i].name);
      for (const auto& w : a.wires)
        if (!w.next && w.from == a.blocks[i].name && w.from != w.to) indegree[w.to]--;
      break;
    }
    if (!moved) break;  // a cycle within a turn: the architecture would not have verified
  }
  return order;
}

}  // namespace detail

// Runs `turns` turns. `initial` gives each wire marked next its value for the first turn, by the wire's words (Wire::words()).
inline Run run(const architecture::Architecture& a, const std::map<std::string, Method>& methods, const Instance& in, const std::map<std::string, Value>& initial, std::size_t turns,
               std::uint64_t seed) {
  Run r;
  const std::string claim = "the loop of " + a.name + " is run for " + std::to_string(turns) + " turns";
  if (const auto v = architecture::verify(a); !v.admitted()) {
    r.verdict = Verdict::no(claim, "the architecture is not admitted: " + v.verdict.reason);
    return r;
  }
  for (const auto& b : a.blocks)
    if (!methods.count(b.name)) {
      r.verdict = Verdict::no(claim, "no method fills the block " + b.name);
      return r;
    }
  for (const auto& w : a.wires)
    if (w.next && !initial.count(w.words())) {
      r.verdict = Verdict::no(claim, "no initial value for the wire " + w.words());
      return r;
    }
  r.order = detail::topological(a);
  calx::random::Stream rng(seed, 7, 0, 0);
  for (std::size_t t = 0; t < turns; t++) {
    Turn turn;
    for (const auto& name : r.order) {
      const architecture::Block& b = *a.block(name);
      std::vector<Value> args;
      for (const auto& p : b.inputs) {
        const architecture::Wire* feed = nullptr;
        for (const auto& w : a.wires)
          if (w.to == name && w.in == p.name) feed = &w;
        if (feed->next) {
          args.push_back(t == 0 ? initial.at(feed->words()) : r.turns.back().outputs.at(feed->from).at(blocks::port(a.block(feed->from)->outputs, feed->out)));
        } else {
          args.push_back(turn.outputs.at(feed->from).at(blocks::port(a.block(feed->from)->outputs, feed->out)));
        }
      }
      std::vector<Value> out = methods.at(name).apply(args);
      for (std::size_t k = 0; k < out.size(); k++)
        if (!in.belief(b.outputs[k].type) && !out[k].law.empty()) {
          // drawn once: from here on the value is certain, and the probability it had is its measure
          out[k].label = detail::draw(out[k].law, rng.uniform());
          if (out[k].measure.kind == kernel::Measure::Kind::none) out[k].measure = kernel::Measure::probability(out[k].law[out[k].label]);
          out[k].law.clear();
        }
      turn.outputs[name] = std::move(out);
    }
    r.turns.push_back(std::move(turn));
  }
  r.verdict = Verdict::yes(claim, "every block applied " + std::to_string(turns) + " times, in the order " + [&] {
    std::string s;
    for (std::size_t i = 0; i < r.order.size(); i++) s += (i ? ", " : "") + r.order[i];
    return s;
  }());
  return r;
}

}  // namespace kybernesis::loop
