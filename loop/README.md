# loop: The Loop Run

*Status: draft.*

## 1. Concepts

The executor turns any admitted architecture whose blocks are filled. It applies the blocks in a topological order of the wires within a turn. Wires marked next are read from the last turn, or from initial values on the first. Laws are drawn from a seeded stream, so that a run repeats. It knows no block by name: a revised architecture runs as soon as it is admitted.

## 2. Content

| Header | Holds |
|---|---|
| `run.hpp` | `Turn`, `Run`, `run` |

## 3. Checks

`check_kybernesis_loop`, on the invented example:

- 300 turns with the Bayes filter never lose the true state, and neither does the possibilistic filter; the face-value estimator loses it on many turns;
- evaluations come with every standing and with intervals;
- runs repeat;
- a revised architecture runs once its methods are carried over;
- an architecture not admitted, or a block without a method, is refused.
