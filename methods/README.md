# methods: Predefined Methods for the Blocks

*Status: draft.*

## 1. Concepts

Adapters of methods that already exist in the sibling libraries, written for no block of kybernesis. Each states its block's ports once, by taking them from the architecture, and reads ports by name, so it fills any block with the names and types it reads. Admission decides whether it may (`blocks/admission.hpp`).

| Block | Declared | Formal | Statistical |
|---|---|---|---|
| Interpretation | `declared_readings` (a table set down) | `construal_readings` (tekmerion's construal: the readings that stand, by their sources) | `precedent_readings` (tekmerion's precedents under a Dirichlet prior) |
| Estimation | `face_value` (a counterexample: refused as unsound) | `possibility_filter` | `bayes_filter` (tyche's Bayes's rule on a POMDP) |
| Evaluation | | | `exceedance` (the probability a term holds under each admissible reading, judged by a standard: standing over the readings, an interval of probabilities) |
| Policy | `threshold_policy` | | `hsvi_policy` (tyche's HSVI) |
| Any | `constant`, `identity`, `table`, `mechanism` | | |
| Referred blocks | `transition`, `identity` (of the world) | | |

`example.hpp` is an invented example, said to be invented: a district by a river, with its instance, its POMDP, its world as relations, and a method for every block of the standard architecture.

## 2. Checks

`check_kybernesis_methods`:

- every block of the standard architecture filled and admitted;
- the alternatives admitted, and the face-value estimator refused;
- the methods' answers held to cases worked by hand: construal, precedents, exceedance under both readings, Bayes's rule, a contradiction, HSVI's choices.

Run with `--report`, it prints the report as JSON.
