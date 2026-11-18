# architecture: The Architecture as Data, Read, Verified and Revised

*Status: draft.*

## 1. Concepts

The architecture of the loop from evidence to action is a declaration, not code. It has:

- registers, ordered by aboutness;
- the stages of a turn;
- the kinds of fact, examples among them;
- types;
- blocks with typed ports and laws, and boundaries;
- wires, some carried to the next turn.

It is read from a script and printed back, so that it can be revised in a file or in code. A revision is admitted exactly when its checks pass. The checks are of two kinds:

- *constraints* on the declaration;
- *categorical claims* that kategoria answers on a script generated from it.

Some properties are categorical and some are not:

- that no wire runs back against the order of a turn is a functor's existence;
- that a wire does not skip a stage across registers, so that the record sees the system only through what is observed, is a property of paths, and is a constraint ([theory](../docs/theory.md), §2).

## 2. Content

| Header | Holds |
|---|---|
| `model.hpp` | `Stage`, `Kind`, `Type`, `Port`, `Block`, `Wire`, `Architecture` |
| `language.hpp` | the grammar in BNF (read by morphe), `statements`, `read`, `print` |
| `checks.hpp` | `known_laws`, `constraints`, `categorical_script`, `categorical`, `verify` |
| `revision.hpp` | `difference`, `revise` |
| `report.hpp` | `report_json`, the report the visualization layer's computational graph reads |

## 3. Checks

`check_kybernesis_architecture`:

- the standard architecture read with its counts, printed back to itself, and admitted, with kategoria's ten claims;
- a syntax fault reported with its line;
- each kind of fault refused by its own rule:
  - a wire backwards, refused by kategoria alone;
  - a wire over a stage across registers, refused by its constraint while the functor still holds;
  - an example yielded by a block;
  - an input fed by nothing;
  - an unknown law;
  - a cycle of kinds;
  - a kind under an example;
  - a block in the wrong register;
- revisions admitted and refused.

`check_kybernesis_schema`:

- the schema's 42 statements answered by kategoria as expected;
- the architecture's kinds included into the schema's, fully and faithfully.
