# transfer: Practices Carried between Relational Systems

*Status: draft.*

## 1. Concepts

A practice is a policy over the states of the system it came from. Carrying it to another system needs a lens:

- **f:** the target read in the source's terms;
- **g:** the source's actions realised in the target.

It is warranted where the mechanism commutes, f ∘ τ_X ∘ (1 ⊗ g) ≈ τ_J ∘ (f ⊗ 1). Two answers are kept apart:

- **the square in relations,** decided by kategoria, which says where the supports differ and which side reaches what the other does not;
- **the square in laws,** the total variation at each target state and source action, judged within a declared tolerance.

Support factors are conditions on the target's state. A factor is:

- **sufficient** when the square commutes wherever it holds;
- **necessary** when the square commutes nowhere it fails.

"It will work here" is a dual verdict over the readings of the target's terms:

- **clear** when, under every reading, the target is believed to be only where the square commutes;
- **untenable** when, under every reading, it is believed to be only where the square does not commute;
- **arguable** otherwise, with what it turns on.

Its measure is the interval of the probability that the square commutes where the target is.

*Caveat.* Commuting is the agreement of mechanisms, not the success of the practice.

## 2. Content

| Header | Holds |
|---|---|
| `transfer.hpp` | `System`, `Lens`, `Reading`, `Factor`, `Square`, `Assessment`; `well_formed`, `push`, `carry`, `square`, `assess` |

## 3. Checks

`check_kybernesis_transfer` (18 checks), on an invented example of leaving a hazard's reach:

- refuges within reach is the support factor, sufficient and necessary within a tolerance of 0.15;
- the square in relations fails, and holds where it should;
- "it will work here" is arguable over two readings of "within reach", with probability in [0.5, 1];
- a stricter tolerance changes the factor;
- clear and untenable cases;
- the carried policy.
