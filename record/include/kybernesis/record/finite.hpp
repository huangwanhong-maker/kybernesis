// Finite values in the record (decision 0004): an instance's values written as canonical text, the checks of its types, a
// block's admitted method seen by the record, and a bound that holds a person's estimate to what the evidence allows.
//
// A value is written as four fields, `label=…;law=…;standing=…;measure=…`, numbers with seventeen significant digits so that
// writing and reading again is exact, and "nan" for a measure's missing parts. A finite value's law is kept when its method
// answered one (a value still to be drawn): the record keeps what the method said, and a draw, when one is made, is a
// person's or an instrument's act of its own.
#pragma once

#include <kybernesis/blocks/method.hpp>
#include <kybernesis/blocks/values.hpp>
#include <kybernesis/record/record.hpp>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace kybernesis::record {

namespace detail {

inline std::string number17(double x) {
  if (std::isnan(x)) return "nan";
  char buf[40];
  std::snprintf(buf, sizeof buf, "%.17g", x);
  return buf;
}

inline std::optional<double> read_number(const std::string& s) {
  if (s == "nan") return std::nan("");
  char* end = nullptr;
  const double x = std::strtod(s.c_str(), &end);
  if (s.empty() || end != s.c_str() + s.size()) return std::nullopt;
  return x;
}

inline std::vector<std::string> split(const std::string& s, char sep) {
  std::vector<std::string> out;
  std::string cur;
  for (char c : s) {
    if (c == sep) out.push_back(cur), cur.clear();
    else cur += c;
  }
  out.push_back(cur);
  return out;
}

inline const char* measure_word(kernel::Measure::Kind k) {
  switch (k) {
    case kernel::Measure::Kind::probability: return "probability";
    case kernel::Measure::Kind::interval: return "interval";
    case kernel::Measure::Kind::estimate: return "estimate";
    default: return "none";
  }
}

}  // namespace detail

inline std::string write(const blocks::Value& v) {
  std::string law;
  for (std::size_t i = 0; i < v.law.size(); i++) law += (i ? "," : "") + detail::number17(v.law[i]);
  const auto& m = v.measure;
  return "label=" + std::to_string(v.label) + ";law=" + law + ";standing=" + kernel::name(v.standing) + ";measure=" + detail::measure_word(m.kind) + ":" + detail::number17(m.value) +
         ":" + detail::number17(m.lower) + ":" + detail::number17(m.upper) + ":" + detail::number17(m.error);
}

inline std::optional<blocks::Value> parse(const std::string& text) {
  const auto fields = detail::split(text, ';');
  if (fields.size() != 4 || fields[0].rfind("label=", 0) != 0 || fields[1].rfind("law=", 0) != 0 || fields[2].rfind("standing=", 0) != 0 || fields[3].rfind("measure=", 0) != 0)
    return std::nullopt;
  blocks::Value v;
  const std::string label = fields[0].substr(6);
  if (label.empty() || label.find_first_not_of("0123456789") != std::string::npos) return std::nullopt;
  v.label = static_cast<std::size_t>(std::stoull(label));
  const std::string law = fields[1].substr(4);
  if (!law.empty())
    for (const auto& p : detail::split(law, ',')) {
      const auto x = detail::read_number(p);
      if (!x) return std::nullopt;
      v.law.push_back(*x);
    }
  const std::string st = fields[2].substr(9);
  bool known = false;
  for (auto s : {kernel::Standing::clear, kernel::Standing::arguable, kernel::Standing::untenable, kernel::Standing::undecided})
    if (st == kernel::name(s)) v.standing = s, known = true;
  if (!known) return std::nullopt;
  const auto m = detail::split(fields[3].substr(8), ':');
  if (m.size() != 5) return std::nullopt;
  bool kind = false;
  for (auto k : {kernel::Measure::Kind::none, kernel::Measure::Kind::probability, kernel::Measure::Kind::interval, kernel::Measure::Kind::estimate})
    if (m[0] == detail::measure_word(k)) v.measure.kind = k, kind = true;
  if (!kind) return std::nullopt;
  for (std::size_t i = 1; i < 5; i++) {
    const auto x = detail::read_number(m[i]);
    if (!x) return std::nullopt;
    (i == 1 ? v.measure.value : i == 2 ? v.measure.lower : i == 3 ? v.measure.upper : v.measure.error) = *x;
  }
  return v;
}

// Whether a text is a value of a type of the instance: a label among its labels, or a law over them (a belief, or a finite
// value still to be drawn).
inline Check check_of(const blocks::Instance& in, const std::string& type) {
  return [in, type](const std::string& text) -> Verdict {
    const std::string claim = "a value of " + type;
    const auto v = parse(text);
    if (!v) return Verdict::no(claim, "it is not written as a value (label=…;law=…;standing=…;measure=…)");
    const std::size_t n = in.labels(type).size();
    if (in.belief(type) || !v->law.empty()) {
      if (v->law.size() != n) return Verdict::no(claim, "its law has " + std::to_string(v->law.size()) + " parts for " + std::to_string(n) + " values");
      double sum = 0;
      for (double p : v->law) {
        if (!(p >= 0) || !std::isfinite(p)) return Verdict::no(claim, "its law has a part that is no probability");
        sum += p;
      }
      if (std::fabs(sum - 1) > 1e-9) return Verdict::no(claim, "its law sums to " + detail::number17(sum));
      return Verdict::yes(claim);
    }
    if (v->label >= n) return Verdict::no(claim, "its label " + std::to_string(v->label) + " is not among the " + std::to_string(n) + " values");
    return Verdict::yes(claim);
  };
}

// The checks of every type the instance gives.
inline std::map<std::string, Check> checks(const blocks::Instance& in) {
  std::map<std::string, Check> out;
  for (const auto& [name, spec] : in.types)
    if (in.has(name)) out[name] = check_of(in, name);
  return out;
}

// A block's method (blocks/method.hpp), seen by the record: its inputs read, its outputs written.
inline Method method_of(const blocks::Method& m) {
  return {m.name, m.wraps, m.family, [m](const std::vector<std::string>& texts) {
            std::vector<blocks::Value> args;
            for (const auto& t : texts) {
              const auto v = parse(t);
              if (!v) throw std::invalid_argument("an input is not written as a value");
              args.push_back(*v);
            }
            std::vector<std::string> out;
            for (const auto& v : m.apply(args)) out.push_back(write(v));
            return out;
          }};
}

inline std::map<std::string, Method> methods_of(const std::map<std::string, blocks::Method>& ms) {
  std::map<std::string, Method> out;
  for (const auto& [block, m] : ms) out[block] = method_of(m);
  return out;
}

// A bound on what a person enters as an estimate: given the block's inputs, `allowed` answers the states the evidence allows
// (for instance a possibilistic filter's support); an estimate whose support meets none of them is refused, and one that
// leaves some of them out is entered with the remark, naming them. `port` is the estimate's output port.
inline Bound support_bound(const blocks::Instance& in, const std::string& belief_type, std::size_t port,
                           std::function<kernel::Possible(const std::vector<blocks::Value>& inputs)> allowed) {
  return [in, belief_type, port, allowed](const std::vector<std::string>& inputs, const std::vector<std::string>& outputs) -> Verdict {
    const std::string claim = "the estimate keeps to what the evidence allows";
    std::vector<blocks::Value> args;
    for (const auto& t : inputs) {
      const auto v = parse(t);
      if (!v) return Verdict::unknown(claim, "an input is not a value, so the evidence cannot be read");
      args.push_back(*v);
    }
    const auto b = parse(outputs.at(port));
    if (!b) return Verdict::no(claim, "the estimate is not a value");
    const kernel::Possible ok = allowed(args);
    const auto& labels = in.labels(belief_type);
    bool meets = false;
    std::string left_out;
    for (std::size_t s = 0; s < ok.size() && s < b->law.size(); s++) {
      if (ok[s] && b->law[s] > 0) meets = true;
      if (ok[s] && !(b->law[s] > 0)) left_out += (left_out.empty() ? "" : ", ") + labels[s];
    }
    if (!meets) return Verdict::no(claim, "it excludes every state the evidence allows");
    if (!left_out.empty()) return Verdict::unknown(claim, "it leaves out states the evidence allows: " + left_out + " (a judgement beyond the evidence)");
    return Verdict::yes(claim);
  };
}

}  // namespace kybernesis::record
