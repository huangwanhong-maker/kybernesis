// An invented example: a district by a river, steered by the standard architecture (decision 0003).
//
// Everything here is an illustration, said to be: the states, laws, readings and thresholds are invented to exercise the
// architecture, its admission and its loop, and describe no river.
//
//   State        dry, shallow (water in the streets), deep
//   Traces       calm, alarm                         Words     gauge (a gauge's reading), report (residents' reports)
//   Readings     strict ("flooded" means deep), loose (shallow or deep), both
//   Action       wait, protect                       Inquiry   routine, survey
//   Evaluations  none, concern                       PolicyVersion  cautious, relaxed
//
// The POMDP (tyche::decision): waiting, a dry district may take water (0.2), shallow water may deepen (0.3) or drain (0.2),
// deep water may fall (0.3); protecting keeps a dry district dry and drains faster. An alarm sounds falsely over a dry district
// (0.1), often over shallow water (0.6), always over deep. Rewards are costs of damage and of protecting. The world's relations
// for soundness are the supports of these laws.
#pragma once

#include <kybernesis/architecture/model.hpp>
#include <kybernesis/blocks/admission.hpp>
#include <kybernesis/methods/common.hpp>
#include <kybernesis/methods/estimation.hpp>
#include <kybernesis/methods/evaluation.hpp>
#include <kybernesis/methods/interpretation.hpp>
#include <kybernesis/methods/policy.hpp>

#include <tyche/decision/pomdp.hpp>

#include <map>
#include <string>
#include <vector>

namespace kybernesis::methods::example {

inline Instance riverside() {
  Instance in;
  in.name = "riverside";
  in.types = {{"State", {{"dry", "shallow", "deep"}, ""}},
              {"Belief", {{}, "State"}},
              {"Traces", {{"calm", "alarm"}, ""}},
              {"Words", {{"gauge", "report"}, ""}},
              {"Terms", {{"flooded"}, ""}},
              {"Readings", {{"strict", "loose", "both"}, ""}},
              {"Operators", {{"exposure"}, ""}},
              {"Standards", {{"half"}, ""}},
              {"Rules", {{"council"}, ""}},
              {"Proposals", {{"none"}, ""}},
              {"Practice", {{"none"}, ""}},
              {"Evaluations", {{"none", "concern"}, ""}},
              {"PolicyVersion", {{"cautious", "relaxed"}, ""}},
              {"Action", {{"wait", "protect"}, ""}},
              {"Inquiry", {{"routine", "survey"}, ""}}};
  return in;
}

inline tyche::decision::Pomdp pomdp() {
  tyche::decision::Pomdp P(3, 2, 2, 0.95);
  const double wait[3][3] = {{0.8, 0.2, 0.0}, {0.2, 0.5, 0.3}, {0.0, 0.3, 0.7}};
  const double protect[3][3] = {{1.0, 0.0, 0.0}, {0.6, 0.4, 0.0}, {0.0, 0.5, 0.5}};
  const double alarm[3] = {0.1, 0.6, 1.0};
  const double cost_wait[3] = {0, -2, -10}, cost_protect[3] = {-1, -3, -6};
  for (std::size_t s = 0; s < 3; s++) {
    for (std::size_t s2 = 0; s2 < 3; s2++) P.transition(0, s, s2) = wait[s][s2], P.transition(1, s, s2) = protect[s][s2];
    for (std::size_t a = 0; a < 2; a++) P.observe(a, s, 0) = 1 - alarm[s], P.observe(a, s, 1) = alarm[s];
    P.reward(s, 0) = cost_wait[s], P.reward(s, 1) = cost_protect[s];
  }
  P.initial = {1.0 / 3, 1.0 / 3, 1.0 / 3};
  return P;
}

// The world as relations: the supports of the POMDP's laws.
inline blocks::World world() {
  const auto P = pomdp();
  blocks::World w;
  w.state = "State", w.belief = "Belief", w.traces = "Traces", w.action = "Action";
  w.dynamics = [P](std::size_t s, std::size_t a) {
    kernel::Possible next(3, 0);
    for (std::size_t s2 = 0; s2 < 3; s2++) next[s2] = P.transition(a, s, s2) > 0;
    return next;
  };
  w.observation = [P](std::size_t s) {
    kernel::Possible o(2, 0);
    for (std::size_t k = 0; k < 2; k++) o[k] = P.observe(0, s, k) > 0;
    return o;
  };
  return w;
}

// The predefined methods for every block of the standard architecture, the Bayes filter and the threshold rule among them.
// `estimator` and `policy` choose among the alternatives: "Bayes filter", "possibility filter", "face value"; "threshold rule", "HSVI".
inline std::map<std::string, blocks::Method> assignment(const architecture::Architecture& a, const std::string& estimator = "Bayes filter", const std::string& policy = "threshold rule",
                                                        const std::string& interpretation = "declared readings") {
  const Instance in = riverside();
  const auto P = pomdp();
  const auto w = world();
  std::map<std::string, blocks::Method> m;
  m["Constitution"] = constant(a, in, "Constitution", {{"terms", "flooded"}, {"operators", "exposure"}, {"standards", "half"}, {"rules", "council"}});
  m["Proposals"] = constant(a, in, "Proposals", {{"proposals", "none"}});
  m["Practice"] = constant(a, in, "Practice", {{"practice", "none"}});
  m["System"] = identity(a, "System", {{"state", "state"}}, Family::world);
  m["Collection"] = mechanism(a, "Collection", "state", [P](std::size_t s) { return Distribution{P.observe(0, s, 0), P.observe(0, s, 1)}; });
  {
    Method e = table(a, in, "Evidence", "archive", "traces", "words", {{"calm", "gauge"}, {"alarm", "report"}}, {{"traces", "traces"}});
    e.wraps = "a trace kept, with the words of its source";
    m["Evidence"] = e;
  }
  if (interpretation == "construal")
    m["Interpretation"] = construal_readings(a, in, "Interpretation", "flooded", "readings",
                                             {{"strict", tekmerion::construal::Source::authority}, {"loose", tekmerion::construal::Source::convention}}, "both");
  else if (interpretation == "precedents")
    m["Interpretation"] = precedent_readings(a, in, "Interpretation", "flooded", "readings", {"strict", "loose"}, {3, 7});
  else
    m["Interpretation"] = declared_readings(a, in, "Interpretation", "words", "readings", {{"gauge", "strict"}, {"report", "loose"}});
  if (estimator == "possibility filter") m["Estimation"] = possibility_filter(a, "Estimation", w, 3);
  else if (estimator == "face value") m["Estimation"] = face_value(a, in, "Estimation", w, {{"calm", "dry"}, {"alarm", "deep"}});
  else m["Estimation"] = bayes_filter(a, "Estimation", w, P);
  ExceedanceSpec spec;
  spec.belief_in = "belief", spec.readings_in = "readings", spec.out = "evaluations";
  spec.admissible = {{"strict", {"strict"}}, {"loose", {"loose"}}, {"both", {"strict", "loose"}}};
  spec.covers = {{"strict", {"deep"}}, {"loose", {"shallow", "deep"}}};
  spec.threshold = 0.5, spec.concern = "concern", spec.none = "none";
  m["Evaluation"] = exceedance(a, in, "Evaluation", spec);
  m["PolicyMaking"] = table(a, in, "PolicyMaking", "react to concern", "evaluations", "policy", {{"none", "relaxed"}, {"concern", "cautious"}});
  if (policy == "HSVI") m["Policy"] = hsvi_policy(a, "Policy", P);
  else m["Policy"] = threshold_policy(a, in, "Policy", {"shallow", "deep"}, {{"cautious", 0.2}, {"relaxed", 0.6}}, "protect", "wait");
  m["Decision"] = table(a, in, "Decision", "inquire when protecting", "action", "inquiry", {{"wait", "routine"}, {"protect", "survey"}}, {{"intervention", "action"}});
  m["Realisation"] = transition(a, "Realisation", "state", "action", [P](std::size_t s, std::size_t act) {
    Distribution d(3);
    for (std::size_t s2 = 0; s2 < 3; s2++) d[s2] = P.transition(act, s, s2);
    return d;
  });
  return m;
}

}  // namespace kybernesis::methods::example
