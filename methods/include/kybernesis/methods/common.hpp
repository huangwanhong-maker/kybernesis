// Methods common to any architecture: constants for boundaries, identities and rules declared by a person, and the world's
// own methods for referred blocks in a simulation (decision 0003).
//
// Each is an adapter in the sense of blocks/method.hpp: it states its block's ports by taking them from the architecture, and
// maps values by name, so that it fills any block whose ports have the names and types it reads.
#pragma once

#include <kybernesis/blocks/method.hpp>
#include <kybernesis/blocks/values.hpp>

#include <map>
#include <string>
#include <vector>

namespace kybernesis::methods {

using blocks::Instance;
using blocks::Method;
using blocks::Value;
using kernel::Distribution;
using kernel::Family;

// Every output a fixed label (a boundary's declared content: the constitution's terms, a set of proposals).
inline Method constant(const architecture::Architecture& a, const Instance& in, const std::string& block, const std::map<std::string, std::string>& labels) {
  const architecture::Block* b = a.block(block);
  std::vector<std::size_t> idx;
  for (const auto& p : b->outputs) idx.push_back(in.index(p.type, labels.at(p.name)));
  return blocks::for_block(a, block, "constant", Family::declared, "kybernesis::methods::constant", [idx](const std::vector<Value>&) {
    std::vector<Value> out;
    for (std::size_t i : idx) out.push_back(Value::certain(i));
    return out;
  });
}

// Each output the input of the same name (an archive keeping what it receives; a policy passed on unchanged).
inline Method identity(const architecture::Architecture& a, const std::string& block, const std::map<std::string, std::string>& from_input, Family family = Family::declared) {
  const architecture::Block* b = a.block(block);
  std::vector<std::size_t> idx;
  for (const auto& p : b->outputs) idx.push_back(blocks::port(b->inputs, from_input.at(p.name)));
  return blocks::for_block(a, block, "identity", family, "kybernesis::methods::identity", [idx](const std::vector<Value>& in) {
    std::vector<Value> out;
    for (std::size_t i : idx) out.push_back(in[i]);
    return out;
  });
}

// A rule declared by a person: an output read from one input's label through a table (other outputs passed on by name).
inline Method table(const architecture::Architecture& a, const Instance& in, const std::string& block, const std::string& name, const std::string& input, const std::string& output,
                    const std::map<std::string, std::string>& rows, const std::map<std::string, std::string>& passed = {}) {
  const architecture::Block* b = a.block(block);
  const std::size_t ip = blocks::port(b->inputs, input), op = blocks::port(b->outputs, output);
  const std::string ity = b->inputs[ip].type, oty = b->outputs[op].type;
  std::vector<std::size_t> map(in.labels(ity).size(), 0);
  for (std::size_t i = 0; i < map.size(); i++) map[i] = in.index(oty, rows.at(in.labels(ity)[i]));
  std::vector<long> pass(b->outputs.size(), -1);
  for (const auto& [o, i] : passed) pass[blocks::port(b->outputs, o)] = static_cast<long>(blocks::port(b->inputs, i));
  return blocks::for_block(a, block, name, Family::declared, "kybernesis::methods::table", [=](const std::vector<Value>& v) {
    std::vector<Value> out(pass.size());
    for (std::size_t k = 0; k < pass.size(); k++) out[k] = pass[k] >= 0 ? v[static_cast<std::size_t>(pass[k])] : Value::certain(0);
    out[op] = Value::certain(map[v[ip].label]);
    return out;
  });
}

// The world's transition for a simulation: the next state drawn from a law given the state and an action.
// law(s, a) is a law over the states.
inline Method transition(const architecture::Architecture& a, const std::string& block, const std::string& state_in, const std::string& action_in,
                         std::function<Distribution(std::size_t, std::size_t)> law, std::string wraps = "the world's transition law") {
  const architecture::Block* b = a.block(block);
  const std::size_t s = blocks::port(b->inputs, state_in), ac = blocks::port(b->inputs, action_in);
  return blocks::for_block(a, block, "transition", Family::world, std::move(wraps), [=](const std::vector<Value>& v) { return std::vector<Value>{Value::drawn(law(v[s].label, v[ac].label))}; });
}

// An observation mechanism: traces drawn from a law given the state (a design's mechanism, declared with it).
inline Method mechanism(const architecture::Architecture& a, const std::string& block, const std::string& state_in, std::function<Distribution(std::size_t)> law,
                        std::string wraps = "the design's observation law") {
  const architecture::Block* b = a.block(block);
  const std::size_t s = blocks::port(b->inputs, state_in);
  return blocks::for_block(a, block, "mechanism", Family::declared, std::move(wraps), [=](const std::vector<Value>& v) { return std::vector<Value>{Value::drawn(law(v[s].label))}; });
}

}  // namespace kybernesis::methods
