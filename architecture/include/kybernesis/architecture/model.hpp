// An architecture as data: registers, stages, kinds, types, blocks, boundaries and wires (decision 0002).
//
// The framework's architecture is not code. It is a declaration, read from a script (language.hpp) and printed back, that
// can be revised by anyone: a block added, removed or changed, a law added, a wire moved. A revision is admitted when its
// checks pass (checks.hpp): constraints on the declaration, and categorical claims answered by kategoria.
//
// - Registers are ordered by aboutness (the Generative Relational framework: what is manifested is relational, what is
//   relational concerns what obtains), the first the lowest.
// - Stages are the moments of one turn of the loop, earliest first, each in a register.
// - Kinds of fact form an order (a kind under the kind it is a kind of), each in a register, inherited unless stated.
//   Example kinds are declared with the operators that yield them and are never yielded by a block.
// - Types travel on wires, each in a register.
// - A block yields a kind of fact at a stage, obeys laws, and has typed input and output ports. A boundary is a block with
//   outputs only: what enters the loop from outside it.
// - A wire joins an output to an input. A wire marked next carries its value to the next turn (a delay).
#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace kybernesis::architecture {

struct Stage {
  std::string name, register_;
  bool operator==(const Stage&) const = default;
};

struct Kind {
  std::string name, parent, register_;  // parent empty for a root; register empty when inherited
  bool example = false;
  bool operator==(const Kind&) const = default;
};

struct Type {
  std::string name, register_;
  bool operator==(const Type&) const = default;
};

struct Port {
  std::string name, type;
  bool operator==(const Port&) const = default;
};

struct Block {
  std::string name, yields, stage;
  bool boundary = false;
  std::vector<std::string> laws;
  std::vector<Port> inputs, outputs;
  bool operator==(const Block&) const = default;
  bool has_law(const std::string& l) const {
    for (const auto& x : laws)
      if (x == l) return true;
    return false;
  }
};

struct Wire {
  std::string from, out, to, in;
  bool next = false;
  bool operator==(const Wire&) const = default;
  std::string words() const { return from + "." + out + " -> " + to + "." + in + (next ? " next" : ""); }
};

struct Architecture {
  std::string name;
  long version = 0;
  std::vector<std::string> registers;  // lowest first, in the order of aboutness
  std::vector<std::string> stage_order;
  std::vector<Stage> stages;
  std::vector<Kind> kinds;
  std::vector<Type> types;
  std::vector<Block> blocks;
  std::vector<Wire> wires;

  const Block* block(const std::string& n) const {
    for (const auto& b : blocks)
      if (b.name == n) return &b;
    return nullptr;
  }
  Block* block(const std::string& n) {
    for (auto& b : blocks)
      if (b.name == n) return &b;
    return nullptr;
  }
  const Kind* kind(const std::string& n) const {
    for (const auto& k : kinds)
      if (k.name == n) return &k;
    return nullptr;
  }
  const Type* type(const std::string& n) const {
    for (const auto& t : types)
      if (t.name == n) return &t;
    return nullptr;
  }
  const Stage* stage(const std::string& n) const {
    for (const auto& s : stages)
      if (s.name == n) return &s;
    return nullptr;
  }
  std::optional<std::size_t> stage_index(const std::string& n) const {
    for (std::size_t i = 0; i < stage_order.size(); i++)
      if (stage_order[i] == n) return i;
    return std::nullopt;
  }
  std::optional<std::size_t> register_index(const std::string& n) const {
    for (std::size_t i = 0; i < registers.size(); i++)
      if (registers[i] == n) return i;
    return std::nullopt;
  }
  // A kind's register, inherited along its parents; empty when none is declared on the way (or the order has a cycle).
  std::string register_of(const std::string& kind_name) const {
    std::string at = kind_name;
    for (std::size_t steps = 0; steps <= kinds.size(); steps++) {
      const Kind* k = kind(at);
      if (!k) return {};
      if (!k->register_.empty()) return k->register_;
      if (k->parent.empty()) return {};
      at = k->parent;
    }
    return {};
  }
  // The type of a block's port; empty when there is no such port.
  std::string port_type(const std::string& block_name, const std::string& port, bool input) const {
    const Block* b = block(block_name);
    if (!b) return {};
    for (const auto& p : input ? b->inputs : b->outputs)
      if (p.name == port) return p.type;
    return {};
  }
  bool operator==(const Architecture&) const = default;
};

}  // namespace kybernesis::architecture
