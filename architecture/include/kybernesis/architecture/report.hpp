// The report of an architecture's checks as JSON, as the visualization layer's computational graph reads it (decision 0002):
//
//   { "architecture": "standard", "version": 1,
//     "checks":  [{"check": ..., "kind": "constraint" | "category" | "law" | "contract", "status": "holds" | "fails" | "undecided", "reason": ...}],
//     "methods": [{"block": ..., "method": ..., "family": ..., "wraps": ..., "status": ..., "reason": ...}] }
//
// The methods are added by the blocks module (blocks/admission.hpp); this header writes the architecture's part.
#pragma once

#include <kybernesis/architecture/checks.hpp>

#include <string>
#include <vector>

namespace kybernesis::architecture {

inline std::string json_string(const std::string& s) {
  std::string out = "\"";
  for (unsigned char c : s) {
    switch (c) {
      case '"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\n': out += "\\n"; break;
      case '\t': out += "\\t"; break;
      case '\r': out += "\\r"; break;
      default:
        if (c < 0x20) {
          char buf[8];
          std::snprintf(buf, sizeof buf, "\\u%04x", c);
          out += buf;
        } else {
          out += static_cast<char>(c);
        }
    }
  }
  return out + "\"";
}

inline std::string json_check(const Check& c) {
  return "{\"check\": " + json_string(c.name) + ", \"kind\": " + json_string(c.kind) + ", \"status\": " + json_string(std::string(kernel::word(c.verdict.status))) +
         ", \"reason\": " + json_string(c.verdict.reason) + "}";
}

// The report with the architecture's checks and, given, the methods' entries already written as JSON objects.
inline std::string report_json(const Architecture& a, const std::vector<Check>& checks, const std::vector<std::string>& method_entries = {}) {
  std::string s = "{\"architecture\": " + json_string(a.name) + ", \"version\": " + std::to_string(a.version) + ",\n \"checks\": [";
  for (std::size_t i = 0; i < checks.size(); i++) s += (i ? ",\n  " : "\n  ") + json_check(checks[i]);
  s += "],\n \"methods\": [";
  for (std::size_t i = 0; i < method_entries.size(); i++) s += (i ? ",\n  " : "\n  ") + method_entries[i];
  s += "]}\n";
  return s;
}

}  // namespace kybernesis::architecture
