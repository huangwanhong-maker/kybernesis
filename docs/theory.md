# The Theory of kybernesis: An Architecture of Steering, Declared, Verified and Filled

*Status: draft (2026-11-18). The formal account of what kybernesis computes and checks. The framework it implements, and the discussion that shaped it, are recorded in the infrastructure's idea of 2026-11-18 (From evidence to action) and its ADR 0130; this note states what the library holds.*

## 1. An Architecture

**Definition 1.1.** An *architecture* $\mathcal A$ consists of:

1. a finite chain of *registers* $R_1 < \dots < R_m$, the order of aboutness (what is manifested is relational, what is relational concerns what obtains);
2. a finite chain of *stages* $S_1 < \dots < S_n$, the moments of one turn, each in a register $\rho(S_i)$;
3. a forest of *kinds* $K$, each kind under its parent, each in a register $\rho(k)$, its own or its parent's; some kinds are *examples*, leaves declared with the operators that yield them;
4. *types* $T$, each in a register;
5. *blocks* $B$, each yielding a kind $\kappa(b)$ at a stage $\sigma(b)$, with typed input and output ports and a set of laws; a *boundary* is a block with outputs only;
6. *wires* $W$, each from an output port to an input port, some marked *next* (a delay to the next turn).

An architecture is read from a script (the grammar is in `architecture/language.hpp`) and printed back; $\mathrm{read}(\mathrm{print}(\mathcal A)) = \mathcal A$.

## 2. Admission of an Architecture

**Definition 2.1 (constraints).** $\mathcal A$ is *well formed* when:

1. names are distinct and every name used is declared;
2. kinds form a forest with registers, no kind lies under an example, and no block yields an example;
3. $\rho(\kappa(b)) = \rho(\sigma(b))$ for every block, and every output's type lies in its block's register;
4. a boundary has outputs and no inputs, and every other block has both;
5. every wire joins ports of one type, every input is fed by exactly one wire, and every output feeds at least one;
6. the wires not marked next form an acyclic graph;
7. a wire not marked next from stage $S_i$ to stage $S_j$ with $j > i + 1$ joins stages of one register;
8. every law is known, and a referred block lies in the highest register.

**Definition 2.2 (categorical claims).** Let $\mathbf{Reg}$, $\mathbf{Kind}$ and $\mathbf{Stage}$ be the thin categories of the three orders. Let $\mathbf{Step}$ be the free category on the graph whose vertices are the blocks and whose edges are the wires not marked next. $\mathcal A$ satisfies its categorical claims when:

- $\mathbf{Reg}$, $\mathbf{Kind}$ and $\mathbf{Stage}$ are thin;
- $\mathbf{Kind}$ has a terminal object;
- $k \mapsto \rho(k)$ is a functor $\mathbf{Kind} \to \mathbf{Reg}$;
- $b \mapsto \sigma(b)$ is a functor $\mathbf{Step} \to \mathbf{Stage}$.

kategoria decides each claim on a script generated from $\mathcal A$ (`categorical_script`).

**Proposition 2.3.** The assignment $b \mapsto \sigma(b)$ extends to a functor $\mathbf{Step} \to \mathbf{Stage}$ exactly when every wire not marked next goes from a stage to the same or a later one.

*Proof.* $\mathbf{Step}$ is free, so a functor is determined by an image for each generator. A wire from $b$ to $b'$ needs an arrow $\sigma(b) \to \sigma(b')$ in the thin chain, and such an arrow exists exactly when $\sigma(b) \le \sigma(b')$. ∎

*Remark (what no functor into the stages can say).* Clause 7 of Definition 2.1 is not a functorial property. In the standard architecture it is the statement that the record sees the system only through what is observed, and acts on it only through what is decided. If $\sigma(b) = S_1$ and $\sigma(b') = S_3$, the wire may be sent to the composite $S_1 \to S_2 \to S_3$, which exists in any category with the two arrows. Forbidding the wire is a condition on the graph of wires itself, a homomorphism into the reflexive graph of consecutive stages with jumps within a register, and kybernesis checks it as a constraint. The architecture check holds an example: a wire from the system to evaluation is refused by clause 7 while the functor of Proposition 2.3 still exists.

**Definition 2.4.** $\mathcal A$ is *admitted* when it is well formed and satisfies its categorical claims. A *revision* $(\mathcal A, \mathcal A')$ is admitted when $\mathcal A'$ is admitted and its version is greater; its difference (blocks, wires, kinds and types added, removed or changed) is reported.

## 3. Blocks, Methods and Their Admission

**Definition 3.1.** An *instance* gives each type either a finite set of labels or the beliefs over another type (laws over its labels). The *shadow* of a value is:

- its label, for a finite value;
- the support of its law, for a finite value drawn from a law;
- its support, for a belief, numbered by bits.

The *representatives* of a shadow are the label itself, or for a belief with support $U$ the uniform law on $U$ and, when $|U| \ge 2$, a skewed law on $U$.

**Definition 3.2.** A *method* for a block $b$ has $b$'s ports and maps input values to output values: a finite output may be a law still to be drawn, and a belief output is the law itself. Its *tabulation* on an instance is the relation
$$f \subseteq Y \times X,$$
where:

- $X$ is the product, over the input ports, of the representatives of their shadows;
- $Y$ is the product, over the output ports, of their shadows;
- $(y, x) \in f$ when $y$ is a possible output shadow on the representative input $x$.

**Definition 3.3 (laws of a block).** Read $f$ as a morphism of the Kleisli category of the non-empty powerset (total relations) when it is total, and in $\mathbf{Rel}$ in general; let $c$ and $d$ be copying and deleting.

- *total:* $f ; d_Y = d_X$;
- *deterministic:* $f ; c_Y = c_X ; (f \otimes f)$, Fritz's definition of a deterministic morphism of a Markov category (Fritz 2020, Definition 10.1);
- *sound*, for an estimator of a belief $B_s$ from a prior $B_x$, traces $O$, an action $A$ and other inputs $C$, in a world given as relations $\tau : S \otimes A \to S$ and $\omega : S \to O$:
  $$m ; \mathrm{step} = m ; \mathrm{step} ; m',$$
  where:
  - $\mathrm{step} : S \otimes B_x \otimes A \otimes C \to S \otimes B_s$ copies the action, moves the state by $\tau$, copies the state, observes it by $\omega$, and applies the method;
  - $m$ and $m'$ are the partial identities on the pairs whose state lies in the belief's support;
- *referred:* the method belongs to the world, which fills a referred block and no other.

kybernesis decides each law with kategoria's `holds_in` in a model in boolean matrices (`blocks/admission.hpp`).

**Proposition 3.4.** A method is sound in the sense of Definition 3.3 exactly when, for every representative prior $b_x$, every state $s$ in its support, every action $a$, every $s' \in \tau(s, a)$, every $o \in \omega(s')$ and every context $c$, every possible output belief contains $s'$ in its support.

*Proof.* Both sides are relations from $S \otimes B_x \otimes A \otimes C$ to $S \otimes B_s$. The left relates an input to the pairs (state reached, belief formed) of the step, when the input's state lies in its prior's support. The right keeps, of those pairs, the ones whose state lies in the formed belief's support. They are equal exactly when no pair is dropped. ∎

**Corollary 3.5.** Bayes's rule on a POMDP is sound in the world of the supports of its laws. When $s \in \operatorname{supp} b$, $T(s' \mid s, a) > 0$ and $Z(o \mid s') > 0$, the observation has positive probability under the belief, and the posterior gives $s'$ positive weight. The possibilistic filter is sound by construction. The estimator that believes the state a trace names is not sound once one state may produce a trace that names another.

*Remark (shadows).* The support is a monad morphism from finite distributions to the non-empty powerset, so supports of composites are composites of supports (`kernel/doctrines.hpp`, checked on random laws). This makes the possibilistic reading of soundness the right one. A method whose output depends on more than its inputs' supports, such as a threshold on a probability, is still checked faithfully, because the tabulation runs over representatives and not over shadows alone.

## 4. Dual Verdicts and Standards

A *finding* carries a value, a *standing* $\in \{\text{clear}, \text{arguable}, \text{untenable}, \text{undecided}\}$, a *measure* (a probability, an interval of probabilities, an estimate with its error, or none) and its provenance. A *standard* asks a least standing, a probability above a threshold, or both:

- an interval holds when its lower end exceeds the threshold, fails when its upper end does not, and is undecided otherwise;
- a proposal is undecided by every standard.

The evaluation of exceedance answers with the class of its finding over the admissible readings as standing and the interval of its probabilities over them as measure: the dual verdict along the loop (`methods/evaluation.hpp`).

## 5. Scales

A value space's *scale* is a group of admissible transformations (Stevens 1946; Krantz, Luce, Suppes and Tversky 1971), and a statement is *meaningful* when its truth is invariant under the group (Suppes and Zinnes 1963). `kernel/scales.hpp` searches a deterministic family of admissible transformations for one that changes a statement's truth. The checks hold that a comparison of means is refuted on an ordinal scale and a comparison of medians is not; that a ratio of values is refuted on an interval scale and a ratio of differences is not; and that order is refuted on a nominal scale.

## 6. The Loop

For an admitted architecture with a method in every block whose admission did not fail, one turn applies the blocks in a topological order of the wires within a turn. A block reads its inputs from this turn, or from the last turn for a wire marked next; a finite output that is a law is drawn from a seeded stream. The executor knows no block by name, so a revised architecture runs as soon as it is admitted and its methods carried over.

## 7. References

- Fritz, T. (2020). A synthetic approach to Markov kernels, conditional independence and theorems on sufficient statistics. *Advances in Mathematics* 370, 107239.
- Kaelbling, L. P., Littman, M. L. and Cassandra, A. R. (1998). Planning and acting in partially observable stochastic domains. *Artificial Intelligence* 101, 99–134.
- Krantz, D. H., Luce, R. D., Suppes, P. and Tversky, A. (1971). *Foundations of Measurement*, vol. I. Academic Press.
- Smith, T. and Simmons, R. (2004). Heuristic search value iteration for POMDPs. *UAI 2004*.
- Stevens, S. S. (1946). On the theory of scales of measurement. *Science* 103, 677–680.
- Suppes, P. and Zinnes, J. L. (1963). Basic measurement theory. In *Handbook of Mathematical Psychology*, vol. I, Wiley.
