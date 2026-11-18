# kernel: Dual Verdicts, Scales and Doctrines

*Status: draft.*

## 1. Concepts

- **Dual verdicts.** Every conclusion of kybernesis carries two answers, kept apart:
  - a *standing*, the formal answer: clear, arguable, untenable or undecided;
  - a *measure*, the statistical answer: a probability, an interval of probabilities, or an estimate with its error.

  A *standard* says how they combine: a least standing, a probability above a threshold, or both, as the law domain's standards of proof do. A method of the family *proposal* (a model's word) is undecided by every standard: it is never the record's.
- **Scales.** A value space's scale is a group of admissible transformations (nominal, ordinal, interval, ratio, absolute). A statement is meaningful when its truth is invariant under the group; `meaningful` searches a family of admissible transformations for one that breaks it.
- **Doctrines.** The kinds of uncertainty on finite spaces: a point, a set of possible points, a law. The Dirac law and the support compare them, and the support is natural: supp ∘ D f = P f ∘ supp.

## 2. Content

| Header | Holds |
|---|---|
| `verdicts.hpp` | `Family`, `Standing`, `Measure`, `Provenance`, `Finding<T>`, `Standard`, `judge`, `Check`, `all_of` |
| `scales.hpp` | `Scale`, `admissible`, `transformations`, `meaningful` |
| `doctrines.hpp` | `Distribution`, `Possible`, `dirac`, `support`, `push`, `probability`, `natural` |

## 3. Checks

`check_kybernesis_kernel` holds:

- standards on standings and measures, and the rule that a proposal never holds;
- meaningfulness on each scale, against comparisons worked by hand;
- the naturality of the support, on 200 random laws and maps.
