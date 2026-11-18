# record: The Trajectory of a Steered System

*Status: draft.*

## 1. Concepts

The record keeps every value of every block at every turn, with how it came to be. Five acts make an entry:

- **method:** an admitted method applied to the block's inputs;
- **observe:** values taken in from outside the loop, at a boundary or at a block the world feeds. The world (a referred block) never has an entry: it is observed where it feeds the loop, never computed;
- **fill:** entered by a person where no method runs;
- **amend:** a method's value changed by a person, the computed value kept beside it;
- **review:** a value accepted or refused before it passes on. A refusal holds the block, and a proposal passes only through a review.

What a person enters is held to what can be held:

- **its type,** by the check of the type;
- **the block's bound,** where one is given: an estimate that excludes every state the evidence allows is refused, and one that leaves some out is entered with the remark;
- **its authority:** an entry counts as the record's only where the rules empower the person's role for that act at that block. Otherwise it is kept as a proposal.

Files are named by the SHA-256 of their bytes. A branch is an overlay on its parent as the parent stood when it was taken, so it re-reads a turn without touching the main line. Every act is sealed in archeion's hash-chained journal.

A turn run again recomputes what methods made from replaced inputs. A person's entry is never replaced: it is reported stale. The record can then be:

- verified (every digest and the chain);
- replayed (every computed value recomputed);
- compared between turns or branches;
- searched for what rests on an entry, a file or an agent;
- given as W3C PROV.

## 2. Content

| Header | Holds |
|---|---|
| `record.hpp` | `Act`, `Entry`, `Rules`, `Branch`, `Trajectory`; `open`, `keep`, `initial`, `attach`; `current`, `inputs_for`, `stale`; `apply`, `observe`, `fill`, `amend`, `review`, `branch`, `revise`; `run_turn`, `propagate`; `verify`, `replay`, `value`, `diff`, `resting_on`, `lineage` |
| `finite.hpp` | an instance's values as canonical text (`write`, `parse`), the checks of its types, a block's method seen by the record, `support_bound` |

## 3. Checks

`check_kybernesis_record` (33 checks), on the invented riverside example and the standard architecture:

- the world observed, not computed;
- a proposal and its review;
- an amendment that keeps the computed value and makes what follows stale;
- an estimate refused, and another entered with its remark;
- a held block;
- two branches: one re-reads a turn of persons' entries, the other recomputes from the first turn;
- replay;
- an altered entry, value and journal, and a missing entry, each found;
- what rests on an amendment, a file and an agent;
- the record as PROV without fault;
- a revision sealed;
- an act dated before the last, refused.
