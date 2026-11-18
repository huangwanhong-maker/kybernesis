// Values on wires, and finite instances of an architecture's types (decision 0003).
//
// An architecture names types; an instance gives each a finite set of labelled values, or declares it the beliefs over
// another type. Methods are run, and their laws decided, on instances.
//
// - A value of a finite type is a label, drawn from a law when the method that made it is stochastic (the law is kept).
// - A value of a belief type is a law over the labels of its base type: the belief itself, a deterministic object.
// - Every value carries a dual verdict (kernel/verdicts.hpp): its standing and its measure, undecided and none by default.
//
// The shadow of a value is what the possibilistic doctrine sees of it: the label of a finite value, the support of a belief
// (a subset of the base type, numbered by its bits: label i is bit 2^i). Laws of methods are decided on representatives of
// the shadows: every label of a finite type, and for a belief each non-empty support, represented by the uniform law on it and,
// when it has two points or more, by a skewed law on it as well, so that a method whose answer depends on more than the
// support is seen to.
#pragma once

#include <kybernesis/kernel/doctrines.hpp>
#include <kybernesis/kernel/verdicts.hpp>

#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace kybernesis::blocks {

using kernel::Distribution;

struct TypeSpec {
  std::vector<std::string> labels;  // a finite type's values
  std::string beliefs_over;         // non-empty: the type's values are laws over this type's labels
};

struct Instance {
  std::string name;
  std::map<std::string, TypeSpec> types;

  const TypeSpec* spec(const std::string& t) const {
    const auto it = types.find(t);
    return it == types.end() ? nullptr : &it->second;
  }
  bool has(const std::string& t) const {
    const TypeSpec* s = spec(t);
    if (!s) return false;
    if (!s->beliefs_over.empty()) {
      const TypeSpec* b = spec(s->beliefs_over);
      return b && b->beliefs_over.empty() && !b->labels.empty() && b->labels.size() <= 16;
    }
    return !s->labels.empty();
  }
  bool belief(const std::string& t) const {
    const TypeSpec* s = spec(t);
    return s && !s->beliefs_over.empty();
  }
  // The labels of a finite type, or of a belief type's base.
  const std::vector<std::string>& labels(const std::string& t) const {
    const TypeSpec* s = spec(t);
    if (!s) throw std::invalid_argument("kybernesis::blocks: the instance " + name + " gives no type " + t);
    return s->beliefs_over.empty() ? s->labels : spec(s->beliefs_over)->labels;
  }
  // The number of shadows: labels of a finite type, subsets of a belief type's base.
  std::size_t shadows(const std::string& t) const {
    return belief(t) ? (std::size_t{1} << labels(t).size()) : labels(t).size();
  }
  std::size_t index(const std::string& t, const std::string& label) const {
    const auto& ls = labels(t);
    for (std::size_t i = 0; i < ls.size(); i++)
      if (ls[i] == label) return i;
    throw std::invalid_argument("kybernesis::blocks: the type " + t + " has no value " + label);
  }
};

struct Value {
  std::size_t label = 0;  // finite types
  Distribution law;       // finite types: the law the label was drawn from (empty when certain); belief types: the belief
  kernel::Standing standing = kernel::Standing::undecided;
  kernel::Measure measure;

  static Value certain(std::size_t i) {
    Value v;
    v.label = i;
    return v;
  }
  static Value drawn(Distribution p) {  // a finite value still to be drawn from p (a method's stochastic answer)
    Value v;
    v.law = std::move(p);
    return v;
  }
  static Value belief(Distribution p) {
    Value v;
    v.law = std::move(p);
    return v;
  }
};

// The shadows a value may have: one for a certain finite value or a belief; the support of the law for a finite value to be drawn.
inline std::vector<std::size_t> shadows_of(const Instance& in, const std::string& type, const Value& v) {
  if (in.belief(type)) {
    std::size_t mask = 0;
    for (std::size_t i = 0; i < v.law.size(); i++)
      if (v.law[i] > 0) mask |= std::size_t{1} << i;
    return {mask};
  }
  if (v.law.empty()) return {v.label};
  std::vector<std::size_t> out;
  for (std::size_t i = 0; i < v.law.size(); i++)
    if (v.law[i] > 0) out.push_back(i);
  return out;
}

// The set of a belief shadow, as a characteristic vector over the base.
inline kernel::Possible members(const Instance& in, const std::string& type, std::size_t shadow) {
  const std::size_t n = in.labels(type).size();
  kernel::Possible s(n, 0);
  for (std::size_t i = 0; i < n; i++) s[i] = (shadow >> i) & 1;
  return s;
}

// Representatives of a shadow: the label itself; or for a belief, the uniform law on the support and a skewed one.
inline std::vector<Value> representatives(const Instance& in, const std::string& type, std::size_t shadow) {
  if (!in.belief(type)) return {Value::certain(shadow)};
  const std::size_t n = in.labels(type).size();
  std::vector<std::size_t> on;
  for (std::size_t i = 0; i < n; i++)
    if ((shadow >> i) & 1) on.push_back(i);
  if (on.empty()) return {};  // no law has an empty support
  Distribution uniform(n, 0.0), skewed(n, 0.0);
  double total = 0;
  for (std::size_t k = 0; k < on.size(); k++) uniform[on[k]] = 1.0 / static_cast<double>(on.size()), skewed[on[k]] = static_cast<double>(k + 1), total += static_cast<double>(k + 1);
  for (auto& x : skewed) x /= total;
  if (on.size() == 1) return {Value::belief(uniform)};
  return {Value::belief(uniform), Value::belief(skewed)};
}

}  // namespace kybernesis::blocks
