// Methods of interpretation: which readings of an open term are admissible, from the words of the evidence and the terms the
// constitution declares (decision 0003).
//
// The block's output is a set of admissible readings of one open term, as a label of the instance (one label per reading,
// and one for both or all when more than one stands). Three families:
//
//   declared     a table from the evidence's words to a reading, set down by a person or a body;
//   formal       tekmerion's construal: each reading with its source (authority, convention, analyst, data, proposal),
//                argued for by an argument of its source's kind, rival readings attacking one another unless the attacker
//                ranks below; the admissible readings are those that stand (apodeixis's interpretive arguments);
//   statistical  tekmerion's precedents: the law of the next reading after the readings of earlier cases, under a Dirichlet
//                prior (Bernardo and Smith 1994, §5.2); a law over the readings, to be drawn.
#pragma once

#include <kybernesis/methods/common.hpp>

#include <tekmerion/construal/construal.hpp>
#include <tekmerion/interpretation/interpreters.hpp>

#include <map>
#include <string>
#include <vector>

namespace kybernesis::methods {

// The words of the evidence read through a declared table: words label -> readings label.
inline Method declared_readings(const architecture::Architecture& a, const Instance& in, const std::string& block, const std::string& words_in, const std::string& readings_out,
                                const std::map<std::string, std::string>& rows) {
  Method m = table(a, in, block, "declared readings", words_in, readings_out, rows);
  m.wraps = "a table set down by a person or a body";
  return m;
}

// One reading of an open term with its source, for construal.
struct SourcedReading {
  std::string reading;  // a label of the readings type
  tekmerion::construal::Source source = tekmerion::construal::Source::analyst;
};

// tekmerion's construal: the admissible readings among those declared, whatever the words. `all` is the label standing for
// every reading at once (used when more than one is admissible).
inline Method construal_readings(const architecture::Architecture& a, const Instance& in, const std::string& block, const std::string& term, const std::string& readings_out,
                                 const std::vector<SourcedReading>& readings, const std::string& all) {
  namespace tc = tekmerion::construal;
  tc::Finding f;
  f.claim = "the term " + term + " is read so";
  tc::Point p;
  p.name = term;
  for (const auto& r : readings) p.readings.push_back({r.reading, r.source, 'A', "", "", false});
  f.points = {p};
  f.judge = [](const apodeixis::interpretation::Interpretation&) { return apodeixis::kernel::Verdict::yes("the reading is taken"); };
  const tc::Construal c = tc::construe(f);
  std::vector<std::string> standing;
  for (std::size_t i = 0; i < c.admissible.size() && i < readings.size(); i++)
    if (c.admissible[i]) standing.push_back(readings[i].reading);
  const architecture::Block* b = a.block(block);
  const std::size_t op = blocks::port(b->outputs, readings_out);
  const std::string oty = b->outputs[op].type;
  const std::size_t label = in.index(oty, standing.size() == 1 ? standing[0] : all);
  return blocks::for_block(a, block, "construal", Family::formal, "tekmerion::construal::construe", [=](const std::vector<Value>&) {
    std::vector<Value> out(b->outputs.size(), Value::certain(0));
    out[op] = Value::certain(label);
    out[op].standing = standing.size() == 1 ? kernel::Standing::clear : kernel::Standing::arguable;
    return out;
  });
}

// tekmerion's precedents: a law over the readings after counts of earlier cases (one count per reading label, in order).
inline Method precedent_readings(const architecture::Architecture& a, const Instance& in, const std::string& block, const std::string& term, const std::string& readings_out,
                                 const std::vector<std::string>& readings, const std::vector<double>& counts, double alpha = 1) {
  namespace ti = tekmerion::interpretation;
  apodeixis::interpretation::OpenPoint point{term, readings, 0};
  const ti::Reading r = ti::read_by_precedents({point}, {counts}, alpha);
  const architecture::Block* b = a.block(block);
  const std::size_t op = blocks::port(b->outputs, readings_out);
  const std::string oty = b->outputs[op].type;
  Distribution law(in.labels(oty).size(), 0.0);
  for (std::size_t i = 0; i < readings.size(); i++) law[in.index(oty, readings[i])] = r.law[i];
  return blocks::for_block(a, block, "precedents", Family::statistical, "tekmerion::interpretation::read_by_precedents", [=](const std::vector<Value>&) {
    std::vector<Value> out(b->outputs.size(), Value::certain(0));
    out[op] = Value::drawn(law);
    return out;
  });
}

}  // namespace kybernesis::methods
