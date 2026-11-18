# CLAUDE.md

This file guides Claude Code in the kybernesis repository.

kybernesis is one of the foundation libraries of [Noetarium, the Science and Humanity Infrastructure](https://github.com/huangwanhong-maker/Noetarium-The-Science-and-Humanity-Infrastructure), which includes it as the git submodule `foundations/kybernesis` once its repository exists. It is a project in its own right and must make sense without the infrastructure.

## 1. Purpose and Scope

kybernesis (κυβέρνησις, steering, government: the root of "govern" and of "cybernetics") is the steering of a system known only through evidence, and the evolution of how it is steered. It holds:

- **the architecture of the loop as data:** registers, stages, kinds of fact, types, blocks with ports and laws, boundaries and wires, read from a script and printed back, revised by anyone, and admitted only when its checks pass. The checks are constraints on the declaration and categorical claims answered by kategoria;
- **blocks and the admission of methods:** a method fills a block when it meets the block's contract and laws, decided on a finite instance, the laws by kategoria in a model in relations of the method's tabulation (total, deterministic, sound);
- **predefined methods,** formal and statistical, adapted from the sibling libraries (tekmerion's construal and precedents, tyche's Bayes's rule and HSVI), with declared and world methods;
- **dual verdicts:** every conclusion with its standing (formal) and its measure (statistical) kept apart, combined by a declared standard; a proposal never holds;
- **the loop run** on an instance, for any admitted architecture;
- later: policy making as evolution with an honest protocol (on heuresis), transfer between relational systems as a lens, and the constitution (versions of the schema, rules of change, migration of records).

It builds on these sibling libraries:

- **kategoria**, for every categorical claim (its script session and its monoidal models);
- **tekmerion**, for construal and interpreters;
- **tyche**, for decisions under partial observation;
- **apodeixis**, for verdicts;
- **morphe**, for the grammar of architectures (its BNF reader);
- peira, nomos, heuresis, kinesis, physis and calx, as the siblings' order requires.

Out of scope:

| Topic | Where it belongs |
|---|---|
| An estimator, a solver, an interpreter in general | tyche, tekmerion, apodeixis, kinesis (kybernesis adapts them) |
| Categories, functors and their verification in general | kategoria |
| Search and evolution in general | heuresis |
| A particular city, court, clinic or policy | the infrastructure's domains |
| Calling a language model, reading files, any work that touches the world | the infrastructure's API and works |

The boundary is [decision 0002](development/decisions/0002-scope-boundaries-and-dependencies.md): what is about the architecture of steering, the admission of methods to it, and the loop in general is kybernesis's; a method itself is its sibling's, and kybernesis holds only its adapter.

## 2. Invariants

1. **Dependencies.** kybernesis depends only on kategoria, tekmerion, tyche, apodeixis, morphe, peira, nomos, heuresis, kinesis, physis and calx among project libraries, and never on the infrastructure's domains, edges or works.
2. **The architecture is data.** No block is known by name to the code that verifies, admits or runs; an architecture is declared and may be revised, and a revision is admitted only when its checks pass.
3. **Admission is decided, never assumed.** A method fills a block only when its contract and the block's laws hold on an instance; an undecided law is said to be undecided.
4. **A proposal is never the record's.** A method of the family proposal (a model's word, grade C) is undecided by every standard.
5. **Pure.** No I/O beyond files whose paths are passed in, no global state, no network.
6. **No particular objects.** Its examples are invented and say so.
7. **Reuse.** What a sibling holds is used through its public headers, never written again here; a general method kybernesis needs goes first to the sibling where it belongs.
8. **Cited sources.** Every method cites its source; equation or section numbers are given only where certain.

## 3. Design Philosophy

kybernesis shares its governing principle with its sibling repositories: **modular, extensible, revisable**.

- **Status vocabulary:**
  - `planned`: named, no content yet;
  - `seed`: an idea, possibly wrong;
  - `draft`: being worked out;
  - `stable`: tested and relied upon;
  - `deprecated`: superseded, with a pointer to its replacement.
- **Revisability is checked.** A revision of the architecture, a new block, a new law or a new method is welcome when it passes; what it changes is reported by name.
- **Formal and statistical together.** Methods of both families answer in the same dual verdict, so that they are compared where they meet.

## 4. Repository Layout

```
kybernesis/
├── CMakeLists.txt   the project: one header-only target kybernesis::<module> per module; the siblings added once
├── kernel/ · architecture/ · blocks/ · methods/ · loop/
├── architectures/   architecture scripts: the standard architecture, and revisions
├── schema/          the schema stated as categories (framework.cat), checked by kategoria
├── docs/            the theory note and figures
└── development/     decisions · ideas · release notes
```

Each module directory holds `README.md` (concepts first, then the content, then the checks), `include/kybernesis/<module>/` and `tests/check_*.cpp`, check programs on calx's `verification`, registered with CTest by `kybernesis_module`, which also tells them where the architectures and the schema are kept (`KYBERNESIS_DATA`). The modules and their order are in [decision 0003](development/decisions/0003-the-first-modules.md).

## 5. Working Procedures

### 5.1 Decisions and Ideas

- Decisions are numbered records in `development/decisions/`. Records are never rewritten; a later record supersedes an earlier one.
- Ideas are files named `development/ideas/YYYY-MM-DD-slug.md`, with status `seed`, `promoted` or `abandoned`.
- **Release notes.** At each commit, add to `development/release-notes/YYYY-MM-DD.md` (one file per day, one short section per commit) a concise summary in plain prose of what is new, what one can now do and what works better. It is not a list of changes (the git log keeps those), and it is not written between commits.

### 5.2 Change Discipline

- Before restructuring or changing an invariant (§2), propose the change and wait for the user's agreement. A revision of an architecture script is not a restructuring: it is the library's ordinary use.
- Keep each change to one concept.

### 5.3 Shared Conventions

The infrastructure and every one of its foundation libraries keep these conventions identical: the writing style; the status vocabulary; the decision, idea and release-note conventions. Change them in every repository together.

### 5.4 Working Inside the Infrastructure

When you edit kybernesis inside the infrastructure's `foundations/kybernesis`: commit in kybernesis first, then the updated pointer in the infrastructure. When a change needs a sibling to change as well, commit them in dependency order: calx and morphe, physis, tyche, peira, nomos, heuresis, kategoria, kinesis, apodeixis, archeion, tekmerion, then kybernesis.

## 6. Conventions

- **Registers** are ordered by aboutness, lowest first (Manifestational < Relational < Ontological in the standard architecture); **stages** by the order of a turn, earliest first.
- **Tuples** of values are numbered with the first port most significant, as kategoria's Kronecker product numbers pairs (i, j) as i·n + j.
- **A belief's shadow** is its support, numbered by bits: label i is bit 2^i.
- **Labels** in architectures and figures are formal nouns and notation; never spoken or interrogative phrasing. Nothing presupposes a physical space, a time or a discipline: loops are counted in turns; transition is between versions and between relational systems.
- **Writing style.**
  - Headings are declarative, numbered and academic, never questions.
  - Concepts come before structure.
  - Label asides with *Rationale.*, *Remark.* or *Caveat.*
  - British spelling; English throughout.

## 7. Technology

- **Language: C++.** This follows ADR 0003 of the infrastructure.
- **Provisional:** C++20, header-only targets `kybernesis::<module>`; CMake 3.20 or later. Checks are programs on calx's `verification`, registered with CTest. On the development machine the compiler is GCC in WSL.
- **Dependency resolution.** Inside the infrastructure, kybernesis uses the siblings under `foundations/`, adding each that no larger project has added. Never nest submodules.
- **Licences.** kybernesis is MIT-licensed. Check every dependency before adopting it.

## 8. Current State

As of 2026-11-18, the first five modules are written and checked ([decision 0003](development/decisions/0003-the-first-modules.md); 86 checks): `kernel`, `architecture`, `blocks`, `methods` and `loop`, with the standard architecture (`architectures/standard.arch`) and the schema (`schema/framework.cat`, 42 statements). The infrastructure's visualization layer draws architectures as computational graphs (`visualization/js/steering/`). The repository is to be published at `https://github.com/huangwanhong-maker/kybernesis`; the user creates it.
