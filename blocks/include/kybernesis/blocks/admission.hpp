// Admission: whether a method may fill a block (decision 0003).
//
// A method is put into a block when it meets the block's contract and the block's laws. That is decided, not assumed, on a
// finite instance of the architecture's types:
//
//   contract   the block is declared; the method's ports are the block's (names, types and order); a referred block takes a
//              method of the world and no other block does; the instance gives every type of its ports; and on every
//              representative input the method answers, with one value per output port, each a value of its type.
//   laws       each law the block declares, decided by kategoria in a model in relations (kategoria::monoidal, boolean
//              matrices) of the method's tabulation over representatives of its inputs' shadows (values.hpp):
//                total           f ; delete = delete: every input has an output;
//                deterministic   f ; copy = copy ; (f ⊗ f): every input has exactly one output (Fritz 2020, Definition 10.1);
//                sound           member ; step = member ; step ; member, for an estimator in a world given as relations: one
//                                step from a belief that holds the true state ends in a belief that holds it (the possibilistic
//                                reading of a filter's correctness; the idea's §5.4);
//                referred        the method is the world's: a referred block is never filled from the record.
//
// A law that cannot be decided on the instance (sound with no world given, or for an estimator whose ports are not those of a
// filter) is undecided, with the reason; an admission with an undecided law and no failure is undecided, and the loop accepts
// it while the report says so. A failure refuses the method, naming the law, the input and the output where it fails.
//
// carry re-admits the methods of a block assignment against a revised architecture: a method whose block was removed, or
// whose contract changed beyond it, is refused, so that a revision of the architecture says which methods survive it.
#pragma once

#include <kybernesis/architecture/model.hpp>
#include <kybernesis/architecture/report.hpp>
#include <kybernesis/blocks/method.hpp>
#include <kybernesis/blocks/values.hpp>
#include <kybernesis/kernel/verdicts.hpp>

#include <kategoria/monoidal/monoidal.hpp>

#include <cmath>
#include <exception>
#include <functional>
#include <string>
#include <vector>

namespace kybernesis::blocks {

using kernel::Check;
using kernel::Verdict;
namespace km = kategoria::monoidal;

// A world, given as relations, against which an estimator's soundness is decided.
struct World {
  std::string state, belief, traces, action;  // the instance's type names: belief is the beliefs over state
  std::function<kernel::Possible(std::size_t state, std::size_t action)> dynamics;
  std::function<kernel::Possible(std::size_t state)> observation;
};

struct Admission {
  std::string block, method, wraps;
  kernel::Family family = kernel::Family::declared;
  std::vector<Check> checks;
  Verdict verdict;
  bool usable() const { return !verdict.fails(); }
};

namespace detail {

inline Verdict from(const kategoria::kernel::Verdict& v) {
  const auto st = v.status == kategoria::kernel::Status::holds ? kernel::Status::holds : v.status == kategoria::kernel::Status::fails ? kernel::Status::fails : kernel::Status::undecided;
  return {st, v.claim, v.reason, {}, v.examined};
}

// The representative inputs of a method: per port, the representatives of every shadow, in order; and their product.
struct Tabulation {
  std::vector<std::vector<Value>> per_port;           // representatives of each input port
  std::vector<std::vector<std::size_t>> port_shadow;  // the shadow each representative stands for
  std::vector<std::size_t> output_sizes;              // shadows of each output port
  std::size_t inputs = 1, outputs = 1;                // the product sizes
  std::vector<std::vector<char>> relation;            // relation[x][y]: output tuple y possible on input tuple x
  std::string fault;                                  // the first input on which the method gives no proper answer
};

inline std::vector<std::size_t> digits(std::size_t x, const std::vector<std::size_t>& sizes) {
  std::vector<std::size_t> d(sizes.size(), 0);
  for (std::size_t k = sizes.size(); k-- > 0;) d[k] = x % sizes[k], x /= sizes[k];
  return d;
}

inline std::size_t number(const std::vector<std::size_t>& d, const std::vector<std::size_t>& sizes) {
  std::size_t x = 0;
  for (std::size_t k = 0; k < sizes.size(); k++) x = x * sizes[k] + d[k];
  return x;
}

inline std::string describe(const Instance& in, const std::vector<Port>& ports, const std::vector<Value>& vs) {
  std::string s;
  for (std::size_t k = 0; k < ports.size(); k++) {
    s += (k ? ", " : "") + ports[k].name + " = ";
    if (in.belief(ports[k].type)) {
      s += "a law on {";
      const auto& ls = in.labels(ports[k].type);
      bool first = true;
      for (std::size_t i = 0; i < vs[k].law.size(); i++)
        if (vs[k].law[i] > 0) s += (first ? "" : ", ") + ls[i], first = false;
      s += "}";
    } else {
      s += in.labels(ports[k].type)[vs[k].label];
    }
  }
  return s;
}

inline Tabulation tabulate(const Method& m, const Instance& in, std::size_t limit = 20000) {
  Tabulation t;
  std::vector<std::size_t> rep_sizes;
  for (const auto& p : m.inputs) {
    std::vector<Value> reps;
    std::vector<std::size_t> shadow;
    for (std::size_t sh = 0; sh < in.shadows(p.type); sh++)
      for (auto& v : representatives(in, p.type, sh)) reps.push_back(v), shadow.push_back(sh);
    rep_sizes.push_back(reps.size());
    t.inputs *= reps.size();
    t.per_port.push_back(std::move(reps));
    t.port_shadow.push_back(std::move(shadow));
  }
  for (const auto& p : m.outputs) t.output_sizes.push_back(in.shadows(p.type)), t.outputs *= in.shadows(p.type);
  if (t.inputs > limit) {
    t.fault = "the instance gives " + std::to_string(t.inputs) + " representative inputs, more than " + std::to_string(limit);
    return t;
  }
  t.relation.assign(t.inputs, std::vector<char>(t.outputs, 0));
  for (std::size_t x = 0; x < t.inputs; x++) {
    const auto d = digits(x, rep_sizes);
    std::vector<Value> args;
    for (std::size_t k = 0; k < d.size(); k++) args.push_back(t.per_port[k][d[k]]);
    std::vector<Value> res;
    try {
      res = m.apply(args);
    } catch (const std::exception& e) {
      t.fault = "on " + describe(in, m.inputs, args) + " it throws: " + e.what();
      return t;
    }
    if (res.size() != m.outputs.size()) {
      t.fault = "on " + describe(in, m.inputs, args) + " it answers " + std::to_string(res.size()) + " values for " + std::to_string(m.outputs.size()) + " outputs";
      return t;
    }
    std::vector<std::vector<std::size_t>> options;
    for (std::size_t k = 0; k < res.size(); k++) {
      const std::string& ty = m.outputs[k].type;
      const std::size_t n = in.labels(ty).size();
      if (in.belief(ty)) {
        double sum = 0;
        bool ok = res[k].law.size() == n;
        for (double p : res[k].law) ok = ok && p >= -1e-12 && std::isfinite(p), sum += p;
        if (!ok || std::fabs(sum - 1) > 1e-9) {
          t.fault = "on " + describe(in, m.inputs, args) + " its output " + m.outputs[k].name + " is no law over " + std::to_string(n) + " values";
          return t;
        }
      } else if (res[k].law.empty() ? res[k].label >= n : res[k].law.size() != n) {
        t.fault = "on " + describe(in, m.inputs, args) + " its output " + m.outputs[k].name + " is no value of " + ty;
        return t;
      } else if (!res[k].law.empty()) {
        double sum = 0;
        bool ok = true;
        for (double p : res[k].law) ok = ok && p >= -1e-12 && std::isfinite(p), sum += p;
        if (!ok || std::fabs(sum - 1) > 1e-9) {
          t.fault = "on " + describe(in, m.inputs, args) + " its output " + m.outputs[k].name + " is drawn from no law";
          return t;
        }
      }
      options.push_back(shadows_of(in, ty, res[k]));
    }
    // every combination of the outputs' possible shadows
    std::vector<std::size_t> pick(options.size(), 0);
    for (;;) {
      std::vector<std::size_t> y(options.size());
      for (std::size_t k = 0; k < options.size(); k++) y[k] = options[k][pick[k]];
      t.relation[x][number(y, t.output_sizes)] = 1;
      std::size_t k = options.size();
      while (k > 0 && ++pick[k - 1] == options[k - 1].size()) pick[--k] = 0;
      if (k == 0) break;
    }
  }
  return t;
}

// A copy map as a matrix: x -> (x, x).
inline km::Matrix copy(std::size_t n) {
  km::Matrix M{n * n, n, std::vector<long long>(n * n * n, 0)};
  for (std::size_t x = 0; x < n; x++) M.at(x * n + x, x) = 1;
  return M;
}
inline km::Matrix erase(std::size_t n) { return {1, n, std::vector<long long>(n, 1)}; }

inline km::Term place(const std::vector<km::Index>& wires, std::size_t at, std::size_t width, km::Term t) {
  std::vector<km::Term> parts;
  if (at > 0) parts.push_back(km::id(std::vector<km::Index>(wires.begin(), wires.begin() + static_cast<long>(at))));
  parts.push_back(std::move(t));
  if (at + width < wires.size()) parts.push_back(km::id(std::vector<km::Index>(wires.begin() + static_cast<long>(at + width), wires.end())));
  return parts.size() == 1 ? parts[0] : km::par(std::move(parts));
}

// Adjacent swaps carrying the wires from one order to another (bubble sort); `wires` is updated.
inline std::vector<km::Term> permute(std::vector<km::Index>& wires, std::size_t from, const std::vector<km::Index>& target) {
  std::vector<km::Term> steps;
  for (std::size_t i = 0; i < target.size(); i++) {
    std::size_t j = from + i;
    while (j < wires.size() && wires[j] != target[i]) j++;
    if (j == wires.size()) throw std::invalid_argument("kybernesis::blocks: a wire to permute is missing");
    for (; j > from + i; j--) {
      steps.push_back(place(wires, j - 1, 2, km::swap(wires[j - 1], wires[j])));
      std::swap(wires[j - 1], wires[j]);
    }
  }
  return steps;
}

inline Check law_check(const std::string& name, Verdict v) { return {name, std::move(v), "law"}; }

// total and deterministic, in a model in relations of the tabulation.
inline Check total_or_deterministic(const Tabulation& t, bool deterministic, const std::string& method) {
  const std::string name = deterministic ? "the law deterministic holds of " + method : "the law total holds of " + method;
  const std::size_t X = t.inputs, Y = t.outputs;
  if (deterministic && static_cast<double>(X) * X * X > 6e7) {
    for (std::size_t x = 0; x < X; x++) {
      std::size_t n = 0;
      for (std::size_t y = 0; y < Y; y++) n += t.relation[x][y];
      if (n != 1) return law_check(name, Verdict::no(name, "input " + std::to_string(x) + " has " + std::to_string(n) + " outputs (decided on the relation: the instance is too large for a model)"));
    }
    return law_check(name, Verdict::yes(name, "every input has one output (decided on the relation: the instance is too large for a model)", X));
  }
  km::Signature S{"Method", {"X", "Y"}, {{"f", {0}, {1}}, {"cX", {0}, {0, 0}}, {"cY", {1}, {1, 1}}, {"dX", {0}, {}}, {"dY", {1}, {}}}};
  km::Model M;
  M.name = "the tabulation of " + method;
  M.semiring = km::Semiring::boolean;
  M.dimension = {X, Y};
  km::Matrix f{Y, X, std::vector<long long>(X * Y, 0)};
  for (std::size_t x = 0; x < X; x++)
    for (std::size_t y = 0; y < Y; y++) f.at(y, x) = t.relation[x][y];
  M.generators = {f, copy(X), copy(Y), erase(X), erase(Y)};
  const km::Term fT = km::gen(0);
  Verdict v;
  if (deterministic) v = from(km::holds_in(S, M, km::seq({fT, km::gen(2)}), km::seq({km::gen(1), km::par({fT, fT})}), name));
  else v = from(km::holds_in(S, M, km::seq({fT, km::gen(4)}), km::gen(3), name));
  return law_check(name, v);
}

}  // namespace detail

// Soundness of an estimator in a world given as relations.
inline Check soundness(const Method& m, const Instance& in, const World& w, const detail::Tabulation& t) {
  const std::string name = "the law sound holds of " + m.name;
  if (!w.dynamics || !w.observation) return detail::law_check(name, Verdict::unknown(name, "no world is given to decide soundness against"));
  // roles of the ports
  long prior = -1, traces = -1, action = -1, out = -1;
  for (std::size_t k = 0; k < m.inputs.size(); k++) {
    const auto& ty = m.inputs[k].type;
    if (ty == w.belief && prior < 0) prior = static_cast<long>(k);
    else if (ty == w.traces && traces < 0) traces = static_cast<long>(k);
    else if (ty == w.action && action < 0) action = static_cast<long>(k);
  }
  for (std::size_t k = 0; k < m.outputs.size(); k++)
    if (m.outputs[k].type == w.belief) out = static_cast<long>(k);
  if (prior < 0 || traces < 0 || action < 0 || out < 0 || m.outputs.size() != 1)
    return detail::law_check(name, Verdict::unknown(name, "soundness is decided for an estimator of a belief from a prior, traces and an action; " + m.name + " has other ports"));
  const std::size_t nS = in.labels(w.state).size(), nA = in.labels(w.action).size(), nO = in.labels(w.traces).size();
  // types: S, A, O, Bx (the prior's representatives), C_j (other inputs' representatives), Bs (the output's shadows)
  km::Signature S;
  S.name = "Filter";
  S.types = {"S", "A", "O", "Bx"};
  std::vector<km::Index> port_type(m.inputs.size());
  std::vector<std::size_t> dims{nS, nA, nO, t.per_port[static_cast<std::size_t>(prior)].size()};
  std::vector<km::Index> context;  // the other inputs' types, in order
  for (std::size_t k = 0; k < m.inputs.size(); k++) {
    if (static_cast<long>(k) == prior) port_type[k] = 3;
    else if (static_cast<long>(k) == traces) port_type[k] = 2;
    else if (static_cast<long>(k) == action) port_type[k] = 1;
    else {
      port_type[k] = S.types.size();
      context.push_back(port_type[k]);
      S.types.push_back("C" + std::to_string(k));
      dims.push_back(t.per_port[k].size());
    }
  }
  const km::Index Bs = S.types.size();
  S.types.push_back("Bs");
  dims.push_back(in.shadows(w.belief));
  // generators: dyn S A -> S; obs S -> O; est ports -> Bs; mem_in S Bx -> S Bx; mem_out S Bs -> S Bs; cA; cS
  S.generators = {{"dynamics", {0, 1}, {0}}, {"observe", {0}, {2}}, {"estimate", port_type, {Bs}}, {"member", {0, 3}, {0, 3}}, {"member'", {0, Bs}, {0, Bs}}, {"cA", {1}, {1, 1}}, {"cS", {0}, {0, 0}}};
  km::Model M;
  M.name = "the world and the tabulation of " + m.name;
  M.semiring = km::Semiring::boolean;
  M.dimension = dims;
  km::Matrix dyn{nS, nS * nA, std::vector<long long>(nS * nS * nA, 0)};
  for (std::size_t s = 0; s < nS; s++)
    for (std::size_t a = 0; a < nA; a++) {
      const auto next = w.dynamics(s, a);
      for (std::size_t s2 = 0; s2 < nS; s2++) dyn.at(s2, s * nA + a) = next[s2] ? 1 : 0;
    }
  km::Matrix obs{nO, nS, std::vector<long long>(nO * nS, 0)};
  for (std::size_t s = 0; s < nS; s++) {
    const auto o = w.observation(s);
    for (std::size_t k = 0; k < nO; k++) obs.at(k, s) = o[k] ? 1 : 0;
  }
  km::Matrix est{dims[Bs], t.inputs, std::vector<long long>(dims[Bs] * t.inputs, 0)};
  for (std::size_t x = 0; x < t.inputs; x++)
    for (std::size_t y = 0; y < t.outputs; y++) est.at(y, x) = t.relation[x][y];
  const std::size_t nBx = dims[3];
  km::Matrix mem_in{nS * nBx, nS * nBx, std::vector<long long>(nS * nBx * nS * nBx, 0)};
  for (std::size_t s = 0; s < nS; s++)
    for (std::size_t b = 0; b < nBx; b++)
      if (t.per_port[static_cast<std::size_t>(prior)][b].law[s] > 0) mem_in.at(s * nBx + b, s * nBx + b) = 1;
  const std::size_t nBs = dims[Bs];
  km::Matrix mem_out{nS * nBs, nS * nBs, std::vector<long long>(nS * nBs * nS * nBs, 0)};
  for (std::size_t s = 0; s < nS; s++)
    for (std::size_t b = 0; b < nBs; b++)
      if ((b >> s) & 1) mem_out.at(s * nBs + b, s * nBs + b) = 1;
  M.generators = {dyn, obs, est, mem_in, mem_out, detail::copy(nA), detail::copy(nS)};
  // the step, from S Bx A C... to S Bs
  std::vector<km::Index> wires{0, 3, 1};
  wires.insert(wires.end(), context.begin(), context.end());
  std::vector<km::Term> step;
  step.push_back(detail::place(wires, 2, 1, km::gen(5)));  // copy the action
  wires.insert(wires.begin() + 3, 1);
  step.push_back(detail::place(wires, 1, 2, km::swap(3, 1)));  // S A Bx A C...
  std::swap(wires[1], wires[2]);
  step.push_back(detail::place(wires, 0, 2, km::gen(0)));  // dynamics: S Bx A C...
  wires.erase(wires.begin() + 1);
  step.push_back(detail::place(wires, 0, 1, km::gen(6)));  // copy the state: S S Bx A C...
  wires.insert(wires.begin(), 0);
  step.push_back(detail::place(wires, 1, 1, km::gen(1)));  // observe: S O Bx A C...
  wires[1] = 2;
  for (auto& sw : detail::permute(wires, 1, port_type)) step.push_back(std::move(sw));
  step.push_back(detail::place(wires, 1, port_type.size(), km::gen(2)));  // estimate: S Bs
  const km::Term stepT = km::seq(step);
  std::vector<km::Index> start{0, 3, 1};
  start.insert(start.end(), context.begin(), context.end());
  const km::Term guard = detail::place(start, 0, 2, km::gen(3));
  const km::Term left = km::seq({guard, stepT}), right = km::seq({guard, stepT, km::gen(4)});
  return detail::law_check(name, detail::from(km::holds_in(S, M, left, right, name)));
}

// Whether a method may fill its block of the architecture, on an instance (and, for soundness, in a world).
inline Admission admit(const architecture::Architecture& a, const Method& m, const Instance& in, const World* world = nullptr) {
  Admission ad{m.block, m.name, m.wraps, m.family, {}, {}};
  auto contract = [&](const std::string& name, Verdict v) { ad.checks.push_back({name, std::move(v), "contract"}); };
  const architecture::Block* b = a.block(m.block);
  const std::string claim = m.name + " may fill the block " + m.block;
  if (!b) {
    contract("the block is declared", Verdict::no("the block is declared", "the architecture " + a.name + " has no block " + m.block));
    ad.verdict = kernel::all_of(ad.checks, claim);
    return ad;
  }
  contract("the block is declared", Verdict::yes("the block is declared"));
  {
    const std::string name = "the method's ports are the block's";
    std::string fault;
    if (m.inputs != b->inputs) fault = "its inputs differ from the block's";
    else if (m.outputs != b->outputs) fault = "its outputs differ from the block's";
    contract(name, fault.empty() ? Verdict::yes(name) : Verdict::no(name, fault));
  }
  {
    const std::string name = "a referred block takes a method of the world, and only a referred block does";
    const bool referred = b->has_law("referred"), world_method = m.family == kernel::Family::world;
    contract(name, referred == world_method ? Verdict::yes(name)
                                            : Verdict::no(name, referred ? "the block is referred to and the method is " + std::string(kernel::name(m.family)) : "a method of the world fills a block of the record"));
  }
  {
    const std::string name = "the instance gives every type of its ports";
    std::string fault;
    for (const auto& p : m.inputs)
      if (fault.empty() && !in.has(p.type)) fault = "the instance gives no " + p.type;
    for (const auto& p : m.outputs)
      if (fault.empty() && !in.has(p.type)) fault = "the instance gives no " + p.type;
    contract(name, fault.empty() ? Verdict::yes(name) : Verdict::no(name, fault));
  }
  if (kernel::all_of(ad.checks, claim).fails()) {
    ad.verdict = kernel::all_of(ad.checks, claim);
    return ad;
  }
  const detail::Tabulation t = detail::tabulate(m, in);
  {
    const std::string name = "the method answers every representative input with a value of each output's type";
    contract(name, t.fault.empty() ? Verdict::yes(name, std::to_string(t.inputs) + " representative inputs", t.inputs) : Verdict::no(name, t.fault));
  }
  if (t.fault.empty())
    for (const auto& law : b->laws) {
      if (law == "total") ad.checks.push_back(detail::total_or_deterministic(t, false, m.name));
      else if (law == "deterministic") ad.checks.push_back(detail::total_or_deterministic(t, true, m.name));
      else if (law == "sound") ad.checks.push_back(world ? soundness(m, in, *world, t) : detail::law_check("the law sound holds of " + m.name, Verdict::unknown("the law sound holds of " + m.name, "no world is given to decide soundness against")));
      else if (law == "referred") ad.checks.push_back(detail::law_check("the law referred holds of " + m.name, m.family == kernel::Family::world ? Verdict::yes("the law referred holds of " + m.name, "the method is the world's") : Verdict::no("the law referred holds of " + m.name, "the method is not the world's")));
      else ad.checks.push_back(detail::law_check("the law " + law + " holds of " + m.name, Verdict::unknown("the law " + law + " holds of " + m.name, "no checker knows the law " + law)));
    }
  ad.verdict = kernel::all_of(ad.checks, claim);
  return ad;
}

// The methods of an assignment re-admitted against a revised architecture.
inline std::vector<Admission> carry(const std::vector<Method>& methods, const architecture::Architecture& revised, const Instance& in, const World* world = nullptr) {
  std::vector<Admission> out;
  for (const auto& m : methods) out.push_back(admit(revised, m, in, world));
  return out;
}

// An admission as an entry of the report (architecture/report.hpp).
inline std::string json_entry(const Admission& ad) {
  using architecture::json_string;
  return "{\"block\": " + json_string(ad.block) + ", \"method\": " + json_string(ad.method) + ", \"family\": " + json_string(kernel::name(ad.family)) +
         ", \"wraps\": " + json_string(ad.wraps) + ", \"status\": " + json_string(std::string(kernel::word(ad.verdict.status))) + ", \"reason\": " + json_string(ad.verdict.reason) + "}";
}

}  // namespace kybernesis::blocks
