// Methods of policy: an action from the belief, under the policy in force (decision 0003; the idea's §3.8).
//
// The block's ports are found by name: the policy in force in, the belief in; the action out, and the policy passed on as
// the one in force (for policy making's next turn).
//
//   declared     a threshold rule: act when the probability of a set of states exceeds the threshold the version in force
//                states (signposts and triggers, as adaptive pathways set them out);
//   statistical  the POMDP solved by HSVI (tyche::decision::hsvi; Smith and Simmons 2004): the action of the lower bound's
//                maximising vector at the belief, a policy on beliefs rather than on states. The bounds' gap at the initial
//                belief is reported as the method's measure.
#pragma once

#include <kybernesis/methods/common.hpp>

#include <tyche/decision/point_based.hpp>
#include <tyche/decision/pomdp.hpp>

#include <map>
#include <string>
#include <vector>

namespace kybernesis::methods {

struct PolicyPorts {
  std::string policy_in = "policy", belief_in = "belief", action_out = "action", current_out = "current";
};

// Act (the action named `act`) when P(set) exceeds the version's threshold; otherwise the action named `rest`.
inline Method threshold_policy(const architecture::Architecture& a, const Instance& in, const std::string& block, const std::vector<std::string>& set,
                               const std::map<std::string, double>& threshold_by_version, const std::string& act, const std::string& rest, PolicyPorts ports = {}) {
  const architecture::Block* b = a.block(block);
  const std::size_t pi = blocks::port(b->inputs, ports.policy_in), bi = blocks::port(b->inputs, ports.belief_in);
  const std::size_t ao = blocks::port(b->outputs, ports.action_out), co = blocks::port(b->outputs, ports.current_out);
  const std::string pty = b->inputs[pi].type, bty = b->inputs[bi].type, aty = b->outputs[ao].type;
  kernel::Possible s(in.labels(bty).size(), 0);
  for (const auto& st : set) s[in.index(bty, st)] = 1;
  std::vector<double> thr(in.labels(pty).size());
  for (std::size_t k = 0; k < thr.size(); k++) thr[k] = threshold_by_version.at(in.labels(pty)[k]);
  const std::size_t A = in.index(aty, act), R = in.index(aty, rest);
  return blocks::for_block(a, block, "threshold rule", Family::declared, "kybernesis::methods::threshold_policy", [=](const std::vector<Value>& v) {
    const double p = kernel::probability(v[bi].law, s);
    std::vector<Value> out(b->outputs.size());
    out[ao] = Value::certain(p > thr[v[pi].label] ? A : R);
    out[ao].measure = kernel::Measure::probability(p);
    out[co] = v[pi];
    return out;
  });
}

// The POMDP solved by HSVI; the action of the lower bound at the belief.
inline Method hsvi_policy(const architecture::Architecture& a, const std::string& block, const tyche::decision::Pomdp& P, double epsilon = 1e-3, PolicyPorts ports = {}) {
  const architecture::Block* b = a.block(block);
  const std::size_t pi = blocks::port(b->inputs, ports.policy_in), bi = blocks::port(b->inputs, ports.belief_in);
  const std::size_t ao = blocks::port(b->outputs, ports.action_out), co = blocks::port(b->outputs, ports.current_out);
  tyche::decision::HsviOptions opt;
  opt.epsilon = epsilon;
  const auto solved = tyche::decision::hsvi(P, opt);
  const auto lower = solved.lower;
  const double gap = solved.upper_value - solved.lower_value;
  return blocks::for_block(a, block, "HSVI", Family::statistical, "tyche::decision::hsvi", [=](const std::vector<Value>& v) {
    std::vector<Value> out(b->outputs.size());
    out[ao] = Value::certain(lower.action(v[bi].law));
    out[ao].measure = kernel::Measure::estimate(lower.value(v[bi].law), gap);
    out[co] = v[pi];
    return out;
  });
}

}  // namespace kybernesis::methods
