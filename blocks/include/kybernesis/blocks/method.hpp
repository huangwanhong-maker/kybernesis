// Methods: what fills a block (decision 0003).
//
// A method is an implementation of a block's contract: the block's input and output ports, with their types, and the
// block's laws. Any algorithm can be a method; one written for another purpose is wrapped by an adapter that reads the
// block's inputs into the algorithm's terms and writes its answer back as values (methods/). A method says:
//
//   name, wraps    what it is, and the library function it wraps (its provenance);
//   family         declared, formal, statistical, proposal, or world (only a referred block takes a method of the world);
//   ports          the block's inputs and outputs, in the block's order;
//   apply          the input values (one per input port) to the output values (one per output port): a finite output may be
//                  a law still to be drawn, a belief output is the law itself; each value may carry a standing and a measure.
//
// Whether a method may fill a block is decided by admission (admission.hpp), never assumed.
#pragma once

#include <kybernesis/architecture/model.hpp>
#include <kybernesis/blocks/values.hpp>
#include <kybernesis/kernel/verdicts.hpp>

#include <functional>
#include <string>
#include <vector>

namespace kybernesis::blocks {

using architecture::Port;
using kernel::Family;

struct Method {
  std::string name, block, wraps;
  Family family = Family::declared;
  std::vector<Port> inputs, outputs;
  std::function<std::vector<Value>(const std::vector<Value>&)> apply;
};

// A method's ports taken from the block it is for, so that an adapter states them once.
inline Method for_block(const architecture::Architecture& a, const std::string& block, std::string name, Family family, std::string wraps,
                        std::function<std::vector<Value>(const std::vector<Value>&)> apply) {
  const architecture::Block* b = a.block(block);
  if (!b) throw std::invalid_argument("kybernesis::blocks: the architecture " + a.name + " has no block " + block);
  return {std::move(name), block, std::move(wraps), family, b->inputs, b->outputs, std::move(apply)};
}

// The index of an input or output port of a method by name.
inline std::size_t port(const std::vector<Port>& ports, const std::string& name) {
  for (std::size_t i = 0; i < ports.size(); i++)
    if (ports[i].name == name) return i;
  throw std::invalid_argument("kybernesis::blocks: no port named " + name);
}

}  // namespace kybernesis::blocks
