// The language of architectures: a script read into an Architecture, and an Architecture printed back (decision 0002).
//
// A script is a sequence of statements, one to a line; '#' begins a comment to the end of the line. A block or a boundary
// has a body in braces, its ports one to a line, closed by a line holding only "}". The grammar of one statement is BNF,
// read by morphe (morphe::formal::compile_bnf) and parsed by Earley's parser, so that a statement that is wrong is answered
// with how far it is right and what could have come next:
//
//   architecture <Name> version <Integer>
//   registers <R> < <R> < ...                       the order of aboutness, lowest first
//   stages <S> < <S> < ...                          the stages of a turn, earliest first
//   stage <S> in <Register>
//   kind <Name> [under <Parent>] [in <Register>]
//   example <Name> under <Parent>
//   type <Name> in <Register>
//   block|boundary <Name> yields <Kind> at <Stage> [laws <law>, ...] {
//     in|out <port> : <Type>
//   }
//   wire <Block>.<port> -> <Block>.<port> [next]
//
// The reader checks syntax only. Whether names are declared, types agree and the whole is well formed is the business of
// checks.hpp, so that a script with several faults is answered with all of them.
//
// print is the reader's inverse up to comments, blank lines and the order of a block's ports between inputs and outputs:
// read(print(a)) = a for every architecture a that read produced (checked), so that a revision can be made in code and
// written back as a script.
#pragma once

#include <kybernesis/architecture/model.hpp>

#include <morphe/formal/bnf.hpp>

#include <cctype>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace kybernesis::architecture {

inline std::string_view grammar_text() {
  return R"bnf(
statement ::= header | registers | stages | stage | kind | example | type | block | wire ;
header    ::= "architecture" _sp aname _sp "version" _sp aversion ;
aname     ::= _name ;
aversion  ::= [0-9]+ ;
registers ::= "registers" _sp rname ( _s "<" _s rname )* ;
rname     ::= _name ;
stages    ::= "stages" _sp sname ( _s "<" _s sname )* ;
sname     ::= _name ;
stage     ::= "stage" _sp sname _sp "in" _sp regname ;
regname   ::= _name ;
kind      ::= "kind" _sp kname ( _sp "under" _sp kparent )? ( _sp "in" _sp regname )? ;
example   ::= "example" _sp kname _sp "under" _sp kparent ;
kname     ::= _name ;
kparent   ::= _name ;
type      ::= "type" _sp tname _sp "in" _sp regname ;
tname     ::= _name ;
block     ::= bkind _sp bname _sp "yields" _sp byields _sp "at" _sp bstage ( _sp "laws" _sp law ( _s "," _s law )* )? _s "{" _s ( port ( _s ";" _s port )* )? _s "}" ;
bkind     ::= "block" | "boundary" ;
bname     ::= _name ;
byields   ::= _name ;
bstage    ::= _name ;
law       ::= _name ;
port      ::= direction _sp pname _s ":" _s ptype ;
direction ::= "in" | "out" ;
pname     ::= _name ;
ptype     ::= _name ;
wire      ::= "wire" _sp wfrom "." wout _s "->" _s wto "." win ( _sp wnext )? ;
wfrom     ::= _name ;
wout      ::= _name ;
wto       ::= _name ;
win       ::= _name ;
wnext     ::= "next" ;
_name     ::= [A-Za-z_] [A-Za-z0-9_]* ;
_sp       ::= [ \t]+ ;
_s        ::= [ \t]* ;
)bnf";
}

inline const morphe::formal::Bnf& grammar() {
  static const morphe::formal::Bnf bnf = morphe::formal::compile_bnf(grammar_text());
  return bnf;
}

struct Fault {
  std::size_t line = 0;
  std::string message;
};

struct Statement {
  std::string text;  // normalised: a block's body joined by "; " inside its braces
  std::size_t line = 1;
};

namespace detail {

inline std::string trim(std::string_view s) {
  std::size_t a = 0, b = s.size();
  while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) a++;
  while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) b--;
  return std::string(s.substr(a, b - a));
}

inline bool starts_with(const std::string& s, std::string_view p) { return s.compare(0, p.size(), p) == 0; }

}  // namespace detail

// The statements of a script, comments removed, a block's body gathered into its statement.
inline std::vector<Statement> statements(std::string_view script, std::vector<Fault>& faults) {
  std::vector<std::string> lines;
  {
    std::string cur;
    for (char c : script) {
      if (c == '\r') continue;
      if (c == '\n') lines.push_back(cur), cur.clear();
      else cur += c;
    }
    lines.push_back(cur);
  }
  std::vector<Statement> out;
  for (std::size_t i = 0; i < lines.size(); i++) {
    std::string l = lines[i];
    if (const auto h = l.find('#'); h != std::string::npos) l.erase(h);
    l = detail::trim(l);
    if (l.empty()) continue;
    const bool opens = (detail::starts_with(l, "block ") || detail::starts_with(l, "boundary ")) && !l.empty() && l.back() == '{';
    if (!opens) {
      out.push_back({l, i + 1});
      continue;
    }
    std::string head = detail::trim(std::string_view(l).substr(0, l.size() - 1));
    std::vector<std::string> ports;
    std::size_t j = i + 1;
    bool closed = false;
    for (; j < lines.size(); j++) {
      std::string p = lines[j];
      if (const auto h = p.find('#'); h != std::string::npos) p.erase(h);
      p = detail::trim(p);
      if (p.empty()) continue;
      if (p == "}") {
        closed = true;
        break;
      }
      ports.push_back(p);
    }
    if (!closed) {
      faults.push_back({i + 1, "the body of this block is not closed by a line holding only \"}\""});
      break;
    }
    std::string text = head + " {";
    for (std::size_t k = 0; k < ports.size(); k++) text += (k ? "; " : " ") + ports[k];
    text += " }";
    out.push_back({text, i + 1});
    i = j;
  }
  return out;
}

struct Reading {
  Architecture architecture;
  std::vector<Fault> faults;  // syntax only; empty when every statement was read
  bool ok() const { return faults.empty(); }
};

// Reads a script. Every statement is parsed; a statement that is wrong is reported with its line and what could have come
// next, and the others are still read.
inline Reading read(std::string_view script) {
  Reading r;
  Architecture& a = r.architecture;
  const auto stmts = statements(script, r.faults);
  bool header = false;
  for (const auto& st : stmts) {
    const auto parsed = grammar().parse(st.text);
    if (!parsed.accepted) {
      r.faults.push_back({st.line, parsed.error});
      continue;
    }
    const morphe::formal::ParseNode& top = parsed.tree;
    const morphe::formal::ParseNode* n = top.children.empty() ? &top : &top.children.front();
    const std::string& what = n->name;
    auto text = [&](const char* child) -> std::string {
      const auto* c = n->child(child);
      return c ? c->text : std::string();
    };
    if (what == "header") {
      if (header) r.faults.push_back({st.line, "a second architecture statement"});
      header = true;
      a.name = text("aname");
      a.version = std::stol(text("aversion"));
    } else if (what == "registers") {
      if (!a.registers.empty()) r.faults.push_back({st.line, "a second registers statement"});
      for (const auto* x : n->all("rname")) a.registers.push_back(x->text);
    } else if (what == "stages") {
      if (!a.stage_order.empty()) r.faults.push_back({st.line, "a second stages statement"});
      for (const auto* x : n->all("sname")) a.stage_order.push_back(x->text);
    } else if (what == "stage") {
      a.stages.push_back({text("sname"), text("regname")});
    } else if (what == "kind") {
      a.kinds.push_back({text("kname"), text("kparent"), text("regname"), false});
    } else if (what == "example") {
      a.kinds.push_back({text("kname"), text("kparent"), "", true});
    } else if (what == "type") {
      a.types.push_back({text("tname"), text("regname")});
    } else if (what == "block") {
      Block b;
      b.boundary = text("bkind") == "boundary";
      b.name = text("bname"), b.yields = text("byields"), b.stage = text("bstage");
      for (const auto* x : n->all("law")) b.laws.push_back(x->text);
      for (const auto* p : n->all("port")) {
        const auto* d = p->child("direction");
        Port port{p->child("pname")->text, p->child("ptype")->text};
        (d && d->text == "in" ? b.inputs : b.outputs).push_back(port);
      }
      a.blocks.push_back(std::move(b));
    } else if (what == "wire") {
      a.wires.push_back({text("wfrom"), text("wout"), text("wto"), text("win"), n->child("wnext") != nullptr});
    } else {
      r.faults.push_back({st.line, "a statement of no known form: " + what});
    }
  }
  if (!header && r.faults.empty()) r.faults.push_back({1, "no architecture statement (architecture <Name> version <Integer>)"});
  return r;
}

// Prints an architecture as a script that reads back to it.
inline std::string print(const Architecture& a) {
  std::ostringstream o;
  o << "architecture " << a.name << " version " << a.version << "\n";
  if (!a.registers.empty()) {
    o << "registers";
    for (std::size_t i = 0; i < a.registers.size(); i++) o << (i ? " < " : " ") << a.registers[i];
    o << "\n";
  }
  if (!a.stage_order.empty()) {
    o << "stages";
    for (std::size_t i = 0; i < a.stage_order.size(); i++) o << (i ? " < " : " ") << a.stage_order[i];
    o << "\n";
  }
  for (const auto& s : a.stages) o << "stage " << s.name << " in " << s.register_ << "\n";
  for (const auto& k : a.kinds) {
    if (k.example) {
      o << "example " << k.name << " under " << k.parent << "\n";
      continue;
    }
    o << "kind " << k.name;
    if (!k.parent.empty()) o << " under " << k.parent;
    if (!k.register_.empty()) o << " in " << k.register_;
    o << "\n";
  }
  for (const auto& t : a.types) o << "type " << t.name << " in " << t.register_ << "\n";
  for (const auto& b : a.blocks) {
    o << (b.boundary ? "boundary " : "block ") << b.name << " yields " << b.yields << " at " << b.stage;
    for (std::size_t i = 0; i < b.laws.size(); i++) o << (i ? ", " : " laws ") << b.laws[i];
    o << " {\n";
    for (const auto& p : b.inputs) o << "  in " << p.name << " : " << p.type << "\n";
    for (const auto& p : b.outputs) o << "  out " << p.name << " : " << p.type << "\n";
    o << "}\n";
  }
  for (const auto& w : a.wires) o << "wire " << w.words() << "\n";
  return o.str();
}

}  // namespace kybernesis::architecture
