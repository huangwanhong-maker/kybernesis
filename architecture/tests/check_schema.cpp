// Checks of the schema (schema/framework.cat), by kategoria, and of the architecture's kinds against it.
//
// - Every statement of the schema is answered as the script expects: "fails" where the line above says so, "holds" elsewhere.
//   So the framework's kinds extend the law domain's conservatively and keep their registers; examples change nothing and are
//   read as evaluations; one step of the loop has models among functions and relations, refusing a noisy feature, a noisy
//   evaluation and an estimator that loses the truth; and a record's nine rules each refuse what they should.
// - The kinds the standard architecture's blocks yield are kinds of the schema, ordered alike: the inclusion of the
//   architecture's Kinds into the schema's is a functor, full and faithful. A revision of the architecture that put a kind under
//   another than the schema does would fail here.

#include <kybernesis/architecture/checks.hpp>
#include <kybernesis/architecture/language.hpp>

#include <kategoria/script/session.hpp>

#include <calx/verification/check.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace kybernesis::architecture;
using kategoria::script::Session;
using kategoria::script::Value;

namespace {

std::string slurp(const std::string& path) {
  std::ifstream in(path);
  std::stringstream s;
  s << in.rdbuf();
  return s.str();
}

std::string field(const Value& o, const char* key) {
  const Value* v = o.get(key);
  return v ? v->text() : std::string();
}

// The block of a category of a script, from "category <name> {" to its closing brace.
std::string block(const std::string& script, const std::string& name) {
  const auto start = script.find("category " + name + " {");
  const auto end = script.find("\n}", start);
  return start == std::string::npos || end == std::string::npos ? std::string() : script.substr(start, end + 2 - start) + "\n";
}

}  // namespace

int main() {
  calx::verification::Suite s;
  const std::string script = slurp(std::string(KYBERNESIS_DATA) + "/schema/framework.cat");
  s.require("the schema's script is read", !script.empty());
  std::vector<std::string> lines;
  {
    std::istringstream in(script);
    for (std::string l; std::getline(in, l);) lines.push_back(l);
  }
  Session session;
  const Value outcomes = session.run(script);
  std::size_t wrong = 0, failing = 0;
  std::string first;
  for (const Value& o : outcomes.items()) {
    const auto line = static_cast<std::size_t>(o.get("line")->number());
    const bool expect_fails = line >= 2 && lines[line - 2].find("# expect fails") == 0;
    failing += expect_fails;
    if (field(o, "status") != (expect_fails ? "fails" : "holds")) {
      wrong++;
      if (first.empty()) first = "line " + std::to_string(line) + ": " + field(o, "status") + ", " + field(o, "reason");
    }
  }
  s.require("kategoria answers every statement of the schema as the script expects (" + std::to_string(outcomes.size()) + " statements, " + std::to_string(failing) + " meant to fail)",
            outcomes.size() >= 40 && failing >= 10 && wrong == 0, first);

  const Reading r = read(slurp(std::string(KYBERNESIS_DATA) + "/architectures/standard.arch"));
  {
    // The architecture's kinds, under another name, and their inclusion into the schema's Kinds and Examples.
    std::string text = block(script, "Examples");
    std::string arch = categorical_script(r.architecture);
    const auto start = arch.find("category Kinds {");
    std::string kinds = arch.substr(start, arch.find("\n}", start) + 2 - start) + "\n";
    kinds.replace(0, std::string("category Kinds").size(), "category ArchitectureKinds");
    text += kinds;
    text += "functor Into : ArchitectureKinds -> Examples {\n";
    for (const auto& k : r.architecture.kinds) text += "  " + k.name + " -> " + k.name + "\n";
    text += "}\ncheck Into is full\ncheck Into is faithful\n";
    Session k;
    const Value answers = k.run(text);
    std::size_t held = 0;
    std::string why;
    for (const Value& o : answers.items()) {
      held += field(o, "status") == "holds";
      if (field(o, "status") != "holds" && why.empty()) why = field(o, "claim") + ": " + field(o, "reason");
    }
    s.require("the kinds the architecture's blocks yield are the schema's, ordered alike: the inclusion is a functor, full and faithful", answers.size() == 5 && held == 5, why);
  }
  return s.done();
}
