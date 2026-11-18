# blocks: Methods and Their Admission

*Status: draft.*

## 1. Concepts

A block is a contract: typed input and output ports, and laws. A **method** fills a block when it meets the contract and the laws on a finite instance of the architecture's types. That is decided, never assumed. Any algorithm can be a method, through an adapter that reads the block's inputs into the algorithm's terms and writes its answer back as values.

- **Values.** A finite value is a label, possibly drawn from a law. A belief is a law over its base type. Every value carries a standing and a measure.
- **Shadows and representatives.** The shadow of a belief is its support. Laws are decided on representatives of every shadow: the uniform law on a support and a skewed one.
- **Laws by kategoria.** The method's tabulation is a relation, and the laws are equations of string diagrams decided in a model in boolean matrices (kategoria's `holds_in`):
  - total: f ; d = d;
  - deterministic: f ; c = c ; (f ⊗ f);
  - sound: m ; step = m ; step ; m', in a world given as relations;
  - referred: the world's methods fill the world's blocks.
- **Carry.** After a revision of the architecture, the methods are re-admitted, and those whose block was removed or changed are refused.

## 2. Content

| Header | Holds |
|---|---|
| `values.hpp` | `TypeSpec`, `Instance`, `Value`, `shadows_of`, `members`, `representatives` |
| `method.hpp` | `Method`, `for_block`, `port` |
| `admission.hpp` | `World`, `Admission`, `admit`, `soundness`, `carry`, `json_entry` |

## 3. Checks

`check_kybernesis_blocks` admits or refuses methods written as plain functions:

- the exact possibilistic filter is admitted as sound;
- the face-value estimator is refused as unsound;
- a deterministic evaluation is admitted, and a noisy one refused;
- methods with other ports, methods that throw, and methods of the record put into the world's block are refused;
- methods are carried over two revisions.
