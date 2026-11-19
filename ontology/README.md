# ontology: The Ontology of Facts as Data

*Status: draft.*

## 1. Concepts

A fixed taxonomy of facts serves one domain until the next needs another kind, another axis or another relation. Here the ontology is declared as data, in a script. It holds:

- **registers,** the order of aboutness;
- **a forest of kinds,** each in a register, never more about what obtains than its parent;
- **axes,** nominal or ordered, applying to kinds and everything beneath them, and required for some of them;
- **relations,** typed by the kinds of their ends.

An ontology is admitted when two things hold:

- **its constraints:** names distinct, kinds a forest, registers in order, axes and relations on declared kinds;
- **its categorical claims, answered by kategoria:** the registers and the kinds are orders, the kinds have one root, and a kind's register is a functor of the two orders.

Records of facts are checked against an ontology, and every problem is named.

A revision is a second ontology with a mapping. It is admitted when:

- every old kind, value and relation has an image;
- each kind keeps its register;
- an ordered axis keeps its order;
- each relation's image contains the images of its ends;
- kategoria decides that the map of kinds is a functor of their orders.

Records are carried along an admitted revision by relabelling. What the revision does not keep is said, and what it now requires and the record lacks is named.

## 2. Content

| Header | Holds |
|---|---|
| `ontology.hpp` | `Ontology`, `Axis`, `Relation`; `read`, `print`; `constraints`, `categorical_script`, `verify`; `Fact`, `Link`, `Record`, `problems`; `Mapping`, `read_mapping`, `admit`, `migrate` |

The ontology of the standard architecture's facts is `ontologies/facts.ont`. Its invented revision is `ontologies/facts-v2.ont`, with the mapping `ontologies/facts-1-to-2.rev`.

## 3. Checks

`check_kybernesis_ontology` (20 checks):

- both versions read, print back and are admitted;
- faulty declarations are refused by the constraint naming the fault: a syntax error with its line, a kind above its parent's register, a kind of an undeclared kind, an axis required where it does not apply;
- two roots are refused by kategoria;
- a record's problems are each named;
- the revision is admitted and its changes are said;
- three revisions are refused: one moving a kind's register, one that is no functor (refused by kategoria), and one reversing an ordered axis;
- a record carried along the revision, with what is lost and what is now lacking named.
