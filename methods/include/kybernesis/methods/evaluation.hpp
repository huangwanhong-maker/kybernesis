// Methods of evaluation: an operator applied to the belief, under the admissible readings of its open term and a standard
// (decision 0003; the idea's §3.4).
//
// exceedance: the probability, under the belief, that the evaluated term holds of the state, read under each admissible
// reading (a reading is the set of states the term covers under it), judged by a standard (a threshold on the probability).
// It answers with a dual verdict:
//
//   standing   clear when the standard is met under every admissible reading, untenable when under none, arguable otherwise
//              (the class of a finding over its admissible readings, tekmerion's construal; apodeixis's classes);
//   measure    the interval of the probabilities over the admissible readings (a single probability when one reading stands).
//
// Its output label is the concern when the standard is met under some admissible reading, and none otherwise; the standing
// says how firmly. It is deterministic: the same belief, readings and standard give the same answer, as the block's law asks.
// The example kinds this operator yields (a feature, a risk) are the declaring domain's names for it, not the record's.
#pragma once

#include <kybernesis/methods/common.hpp>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace kybernesis::methods {

struct ExceedanceSpec {
  std::string belief_in, readings_in, out;
  std::map<std::string, std::vector<std::string>> admissible;  // readings label -> the readings it admits (e.g. both -> {strict, loose})
  std::map<std::string, std::vector<std::string>> covers;      // reading -> the states the term covers under it
  double threshold = 0.5;                                      // the standard: the probability to exceed
  std::string concern, none;                                   // the output labels
};

inline Method exceedance(const architecture::Architecture& a, const Instance& in, const std::string& block, const ExceedanceSpec& spec) {
  const architecture::Block* b = a.block(block);
  const std::size_t bi = blocks::port(b->inputs, spec.belief_in), ri = blocks::port(b->inputs, spec.readings_in), op = blocks::port(b->outputs, spec.out);
  const std::string bty = b->inputs[bi].type, rty = b->inputs[ri].type, oty = b->outputs[op].type;
  const std::size_t states = in.labels(bty).size();
  // per readings label, the sets of states covered under each admissible reading
  std::vector<std::vector<kernel::Possible>> sets(in.labels(rty).size());
  for (std::size_t r = 0; r < sets.size(); r++)
    for (const auto& reading : spec.admissible.at(in.labels(rty)[r])) {
      kernel::Possible s(states, 0);
      for (const auto& st : spec.covers.at(reading)) s[in.index(bty, st)] = 1;
      sets[r].push_back(s);
    }
  const std::size_t concern = in.index(oty, spec.concern), none = in.index(oty, spec.none);
  const double t = spec.threshold;
  return blocks::for_block(a, block, "exceedance", Family::statistical, "kybernesis::kernel::probability, judged over the admissible readings", [=](const std::vector<Value>& v) {
    double lo = 1, hi = 0;
    std::size_t met = 0;
    const auto& readings = sets[v[ri].label];
    for (const auto& s : readings) {
      const double p = kernel::probability(v[bi].law, s);
      lo = std::min(lo, p), hi = std::max(hi, p);
      met += p > t;
    }
    std::vector<Value> out(b->outputs.size(), Value::certain(0));
    Value e = Value::certain(met > 0 ? concern : none);
    e.standing = met == readings.size() ? kernel::Standing::clear : met == 0 ? kernel::Standing::untenable : kernel::Standing::arguable;
    e.measure = readings.size() == 1 ? kernel::Measure::probability(lo) : kernel::Measure::interval(lo, hi);
    out[op] = e;
    return out;
  });
}

}  // namespace kybernesis::methods
