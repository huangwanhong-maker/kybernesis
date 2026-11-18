# 0004. The Record and Transfer: Every Value with How It Came to Be, and Practices Carried by a Lens

- **Status:** Accepted, provisionally (built overnight at the user's word, to be confirmed by them; it changes invariant 1)
- **Date:** 2026-11-19
- **Decided by:** Claude, within the design of the user's engagement with a city (the infrastructure's `development/ideas/2026-11-19-an-engagement-with-a-city.md`), the user having asked that every block admit a person's intervention and keep files, that the trajectory be archived every time reusing the archive system, and that the work be extensible, flexible, revisable and of high quality

## 1. Context

The loop (decision 0003) fills every block with an admitted method. A real steering is not like that: most of what it knows is entered by people, what the world does is observed and never computed, and computed values are amended, accepted or refused by those with the authority to. The user asked for every block to be open to a person's intervention, with files, for the trajectory to be archived every time and revisited "to generate new insights", and for the archive system to be reused. The first case also needs a practice carried from one city to another, which the theory had stated as a lens (the infrastructure's idea of 2026-11-18, §3.7) and left planned.

## 2. Decision

Two modules, after `loop`:

| Module | Depends on | Content |
|---|---|---|
| `record` | blocks; archeion's `fixity` and `provenance` | the trajectory, its acts and their checks, files, branches, revisions, runs of a turn, verification, replay, difference, what rests on what, and lineage (below) |
| `transfer` | kernel; kategoria's `monoidal` | a lens (f, g) between declared systems; the square decided in relations by kategoria and measured in laws within a tolerance; support factors, sufficient and necessary; "it will work here" as a dual verdict over the readings of the target's terms; the carried policy |

The `record` module holds:

- **acts:** entries made by five acts (method, observe, fill, amend, review), each checked for its type, the block's bound and its authority, with a proposal kept apart;
- **files** named by their digests;
- **branches** as overlays on their parent as it stood when taken;
- **revisions** of the architecture;
- **runs of a turn** that recompute what methods made and report persons' entries stale;
- **verification, replay, difference** and what rests on what;
- **lineage** as W3C PROV;
- **finite values** written as canonical text (`finite.hpp`).

kybernesis now declares **archeion** among its dependencies, which changes invariant 1. The record's sealing and lineage are archeion's journal and provenance graph, used through their public headers and never written again here (invariant 7). archeion depends on calx, morphe and apodeixis, which kybernesis already declares, so the order stays acyclic.

Values in the record are text, each type's payload kept once under its digest. The record is agnostic about what blocks carry, and finite instances are one codec. The record keeps no clock: every act is given its instant, and instants never go back.

## 3. Alternatives Considered

- **Persons' entries as methods of the family declared.** Rejected. A method answers on every input and is admitted by its laws. A person answers once, and is held instead to the type, the block's bound where the record can check it, and authority.
- **Recomputing persons' entries when their inputs change.** Rejected. A person's judgement is never silently replaced; it is reported stale for its author to look at again.
- **Branches that copy the turns they share.** Rejected for overlays. A branch reads its parent as it stood when taken, so re-reading one block re-observes nothing, and later changes of the parent never leak into it.
- **A journal of kybernesis's own.** Rejected by invariant 7: archeion holds sealing and provenance.
- **Transfer only in laws.** Rejected. The square in relations, decided by kategoria, says exactly where the supports differ; the square in laws says by how much. They disagree usefully: a target may reach, within the tolerance, what the source never does.

## 4. Consequences

- The checks grow to 137 in eight programs (record 33, transfer 18).
- The infrastructure's API composes both (`api/cpp/science/steering.hpp`), and the Urban Studio's plug-in *From evidence to action* is their first work.
- Not built:
  - policy making as evolution;
  - the constitution's rules of change, beyond the record of revisions;
  - a transfer whose lens is learned rather than declared;
  - beliefs factored over components, so that a belief over more than sixteen states can be admitted.
