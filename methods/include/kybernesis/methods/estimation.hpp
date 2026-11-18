// Methods of estimation: a belief about the system from the traces, the prior belief and the last action (decision 0003).
//
// The block's ports are found by type: one input of the belief type (the prior), one of the traces type, one of the action
// type, other inputs (readings) taken and not used by these three; one output of the belief type.
//
//   formal       the possibilistic filter: the states still possible after the evidence, from the world's relations of
//                transition and observation: {s' : some s in the prior's support, s' possible after s and a, o possible in s'};
//                its belief is the uniform law on them (the doctrine of possibilities carries supports only; the law says so);
//   statistical  Bayes's rule on a POMDP, tyche::decision::update: τ(b, a, o)(s') ∝ Z(o | s', a) Σ_s T(s' | s, a) b(s). An
//                observation of probability zero under the belief is a contradiction of the record: the belief predicted is
//                kept and the finding marked untenable, so that the method answers on every input and says what happened;
//   counterexample  the face-value estimator: the state a trace names, believed outright. It is a lawful method of the block's
//                ports, and it is not sound: the admission refuses it (blocks/admission.hpp), which is the formal statement of
//                why a single reading taken at its word is never the record's estimate.
#pragma once

#include <kybernesis/blocks/admission.hpp>
#include <kybernesis/methods/common.hpp>

#include <tyche/decision/pomdp.hpp>

#include <map>
#include <string>
#include <vector>

namespace kybernesis::methods {

struct FilterPorts {
  std::size_t prior = 0, traces = 0, action = 0;
};

inline FilterPorts filter_ports(const architecture::Block& b, const blocks::World& w) {
  FilterPorts f;
  bool p = false, t = false, ac = false;
  for (std::size_t k = 0; k < b.inputs.size(); k++) {
    if (b.inputs[k].type == w.belief && !p) f.prior = k, p = true;
    else if (b.inputs[k].type == w.traces && !t) f.traces = k, t = true;
    else if (b.inputs[k].type == w.action && !ac) f.action = k, ac = true;
  }
  if (!p || !t || !ac || b.outputs.size() != 1 || b.outputs[0].type != w.belief)
    throw std::invalid_argument("kybernesis::methods: the block " + b.name + " has not the ports of a filter (a prior, traces and an action in, a belief out)");
  return f;
}

// The possibilistic filter in a world given as relations.
inline Method possibility_filter(const architecture::Architecture& a, const std::string& block, const blocks::World& w, std::size_t states) {
  const FilterPorts f = filter_ports(*a.block(block), w);
  return blocks::for_block(a, block, "possibility filter", Family::formal, "kybernesis::methods::possibility_filter", [=](const std::vector<Value>& v) {
    kernel::Possible next(states, 0);
    for (std::size_t s = 0; s < states; s++) {
      if (!(v[f.prior].law[s] > 0)) continue;
      const auto after = w.dynamics(s, v[f.action].label);
      for (std::size_t s2 = 0; s2 < states; s2++)
        if (after[s2] && w.observation(s2)[v[f.traces].label]) next[s2] = 1;
    }
    std::size_t n = 0;
    for (char c : next) n += c;
    Distribution law(states, 0.0);
    Value out;
    if (n == 0) {  // the evidence contradicts every state the prior allowed: keep the prior's support, untenable
      std::size_t m = 0;
      for (std::size_t s = 0; s < states; s++) m += v[f.prior].law[s] > 0;
      for (std::size_t s = 0; s < states; s++) law[s] = v[f.prior].law[s] > 0 ? 1.0 / static_cast<double>(m) : 0.0;
      out = Value::belief(law);
      out.standing = kernel::Standing::untenable;
    } else {
      for (std::size_t s = 0; s < states; s++) law[s] = next[s] ? 1.0 / static_cast<double>(n) : 0.0;
      out = Value::belief(law);
      out.standing = kernel::Standing::clear;
    }
    return std::vector<Value>{out};
  });
}

// Bayes's rule on a POMDP (tyche::decision).
inline Method bayes_filter(const architecture::Architecture& a, const std::string& block, const blocks::World& w, tyche::decision::Pomdp P) {
  const FilterPorts f = filter_ports(*a.block(block), w);
  return blocks::for_block(a, block, "Bayes filter", Family::statistical, "tyche::decision::update", [=](const std::vector<Value>& v) {
    const auto& b = v[f.prior].law;
    const std::size_t act = v[f.action].label, o = v[f.traces].label;
    const auto p = tyche::decision::observation_law(P, b, act);
    Value out;
    if (p[o] > 0) {
      out = Value::belief(tyche::decision::update(P, b, act, o).belief);
      out.standing = kernel::Standing::clear;
      out.measure = kernel::Measure::probability(p[o]);  // the probability the evidence had under the belief
    } else {
      out = Value::belief(tyche::decision::predict(P, b, act));
      out.standing = kernel::Standing::untenable;
      out.measure = kernel::Measure::probability(0);
    }
    return std::vector<Value>{out};
  });
}

// The face-value estimator: each trace names a state, believed outright.
inline Method face_value(const architecture::Architecture& a, const Instance& in, const std::string& block, const blocks::World& w, const std::map<std::string, std::string>& names) {
  const FilterPorts f = filter_ports(*a.block(block), w);
  const std::size_t states = in.labels(w.state).size();
  std::vector<std::size_t> named(in.labels(w.traces).size());
  for (std::size_t o = 0; o < named.size(); o++) named[o] = in.index(w.state, names.at(in.labels(w.traces)[o]));
  return blocks::for_block(a, block, "face value", Family::declared, "kybernesis::methods::face_value", [=](const std::vector<Value>& v) {
    Value out = Value::belief(kernel::dirac(states, named[v[f.traces].label]));
    out.standing = kernel::Standing::clear;
    return std::vector<Value>{out};
  });
}

}  // namespace kybernesis::methods
