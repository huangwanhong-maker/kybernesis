# 0005. Methods Chosen per Block, Boundaries Carried, and the Ontology of Facts as Data

- **Status:** Accepted, provisionally
- **Date:** 2026-11-19
- **Decided by:** Claude, at the user's word:
  - on the choice of a block's method: "4 can proceed", the algorithms themselves being "already very well-formed";
  - on the fact categories: "like what we do for the policy making foundation we just made, make the ontological fact category can be programmed, revisable can be better, as long as they satisfied constraints"

## 1. Context

Three things were found wanting once the record (decision 0004) was used.

- **The method of a block was fixed.** It was whichever one the work registered. Reading old evidence again by another method (pooling instead of intersecting intervals, say) needed a new work, not an act.
- **Boundaries had to be entered again at every turn.** The constitution, the proposals and the practices each needed fresh entries each turn, although they usually stand.
- **The taxonomy of facts was fixed.** The architecture declares kinds of fact as data. The richer taxonomy the law domain uses (five axes) is fixed in that domain's C++, held to a schema by kategoria. A domain that needs another kind, axis or relation has to change code.

## 2. Decision

**`record` gains choices and carrying.**

- **A choice is an act of its own,** sealed in the journal. From a turn on, on a branch, a block is filled by the method of a name among its alternatives (a `Registry`, the first the default). It is made only by a role the rules empower to `choose` for that block, and only among the alternatives given.
- **Branches read choices as they read entries.** Their own choices take precedence, and they inherit their parent's as the parent stood when they were taken.
- **A turn run again** recomputes what another method made from that turn on. Persons' entries are, as ever, reported stale and never replaced.
- **Replay** finds each entry's own method among the alternatives by its name.
- **`carry`** enters a boundary's last values again at the next turn. It is an act by whoever asks, in their role, and the reason says where the values came from. Authority applies as to any entry.

**A module `ontology`: the ontology of facts as data.**

- **The script declares:**
  - registers;
  - a forest of kinds, each in a register;
  - axes, nominal or ordered, with the kinds they apply to and those for which they are required;
  - relations typed by the kinds of their ends.

  It is read by morphe's BNF and printed back.
- **Admission.** An ontology is admitted when its constraints hold (names distinct, kinds a forest, a kind never more about what obtains than its parent, axes on declared kinds, required only where they apply, relations on declared kinds) and kategoria answers its claims: the registers and the kinds are thin; the kinds have one root; the register of a kind is a functor of their orders.
- **Records.** Records of facts are checked against it, every problem named.
- **Revision.** A revision is a second ontology with a mapping (kinds, values, relations renamed or dropped). It is admitted when every old kind, value and relation has an image, and each kind keeps its register. An ordered axis must keep its order. Each relation's image must contain the images of its ends. Last, kategoria decides that the map of kinds is a functor of their orders.
- **Carrying records.** Records are carried along an admitted revision by relabelling. What the revision does not keep is said by name, and what it now requires and the record lacks is named.

## 3. Alternatives Considered

- **A choice of method as an entry.** Rejected: an entry holds a value of a block at a turn; a choice holds for every later turn until another is made. It is sealed beside the entries, not among them.
- **Carrying boundaries silently, by reading the last turn's value.** Rejected: a standing constitution is still a declaration made at each turn, by someone. Carrying is an act, so that who let it stand is recorded.
- **The ontology inside the architecture language.** Rejected: an architecture says how a loop computes; an ontology says what facts there are and how they are described. A work may revise one without the other.
- **Moving the law domain's taxonomy onto the ontology now.** Deferred: it restructures that domain and is proposed to the user. The law domain's taxonomy can then be declared as an ontology script, with its C++ kept as a check of agreement during the move.

## 4. Consequences

- The checks grow to 162 in nine programs (record 38, ontology 20).
- The infrastructure's API offers alternative methods per block (estimation by what the evidence allows, pooled by fixed or random effects on peira's synthesis, or the latest item; interpretation by declared readings or the constitution's alone). The Urban Studio's plug-in lets a person choose among them in the opened block.
- **Not built:**
  - the rules of authority as part of the record (a change of rules sealed and judged by the rules in force), suspended by the user;
  - records migrated backwards along a revision (the pullback);
  - ontologies composed from parts (a work's own kinds added to a shared core).
