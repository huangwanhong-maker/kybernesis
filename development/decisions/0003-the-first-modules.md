# 0003. The First Modules: kernel, architecture, blocks, methods, loop

- **Status:** Accepted, provisionally
- **Date:** 2026-11-18
- **Decided by:** Claude, within decision 0002, the user's stages agreed in discussion

## 1. Context

Decision 0002 fixed the scope. The first stage was to be the core and a finite slice: the kernel, the architecture and its verification, blocks and admission, formal and statistical methods for interpretation, estimation, evaluation and policy, and the loop, with an example run end to end.

## 2. Decision

The modules, in their order, each depending only on those before it:

| Module | Depends on | Content |
|---|---|---|
| `kernel` | apodeixis's verdicts | families of methods, standing, measure, provenance, findings, standards and `judge`; checks; scales and meaningfulness; doctrines on finite spaces |
| `architecture` | kernel, morphe's BNF, kategoria's session | the model; the language and its printer; constraints and categorical claims; revisions; the report as JSON |
| `blocks` | architecture, kategoria's monoidal models | values and instances; methods; admission; carry |
| `methods` | blocks, tyche's decisions, tekmerion's construal and interpreters | adapters for every block of the standard architecture, alternatives of both families, an invented example |
| `loop` | methods, calx's random streams | the run of any admitted architecture |

The standard architecture is `architectures/standard.arch`; the schema `schema/framework.cat` is checked by the architecture module's `check_schema`, which also holds the architecture's kinds to the schema's.

## 3. Alternatives Considered

- **Evolution, transfer and the constitution now.** Deferred to the second stage, so that the first is small enough to check whole.
- **The example in the checks only.** Rejected: an invented example in `methods/example.hpp`, said to be invented, serves the checks of two modules and the documentation.

## 4. Consequences

- 86 checks in six programs at the founding.
- The infrastructure's visualization layer draws architectures and reports (`visualization/js/steering/`); the methods' check prints the report with `--report`.
