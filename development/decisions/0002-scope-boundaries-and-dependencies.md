# 0002. Scope, Boundaries and Dependencies: the Architecture of Steering as Data, Admission by Verification, Dual Verdicts

- **Status:** Accepted
- **Date:** 2026-11-18
- **Decided by:** the user ("I will found kybernesis"; "ensure it is easily to revise and extend … if formal verifications in category and other constraints aspect pass, we should allow revise our framework architecture"; "if a method satisfy the constraints of a block and the relations with other blocks, we can put into it"; "we should like law's one combine formal reasoning + statistical methods"), with Claude; the infrastructure's ADR 0130

## 1. Context

The infrastructure's idea of 2026-11-18 (*From evidence to action*) recorded a framework for steering a system known only through evidence. Its parts are evidence and its collection, interpretation and estimation, evaluation by operators in parallel, policy making, policy and decision. Over them sit three nested loops of knowledge production, transition and governance. Four rounds of discussion revised it. The user asked for it to be a foundation that defines interfaces and their relations, verified categorically; that is easy to revise and extend, blocks included; that offers predefined methods for its blocks, adapted from algorithms not written for it; and that combines formal reasoning with statistics, as the law domain does.

## 2. Decision

1. **Scope.** kybernesis holds:
   - the architecture of the loop, as data;
   - its verification;
   - the blocks and the admission of methods to them;
   - adapters of existing methods;
   - the dual verdict;
   - the loop run.

   Later it will hold policy making as evolution, transfer between relational systems, and the constitution. A method in general (an estimator, a solver, an interpreter) stays its sibling's; kybernesis holds its adapter.
2. **The architecture is data.** It is declared in a script (registers, stages, kinds, types, blocks, boundaries, wires) read by a grammar in BNF on morphe and printed back. The code that verifies, admits and runs knows no block by name. A revision is a new script, admitted exactly when its checks pass:
   - constraints on the declaration;
   - categorical claims answered by kategoria. The kinds form a thin category with a terminal kind, each kind lies in its register functorially, and each block's stage is a functor from the step category to the stages.
3. **Admission is decided.** A method fills a block when, on a finite instance of the architecture's types, its ports are the block's and the block's laws hold. The laws are decided by kategoria in a model in relations of the method's tabulation over representatives of its inputs' shadows: total; deterministic; sound for estimators in a world given as relations; referred for the world's own blocks. An undecided law is reported as undecided. When an architecture is revised, its methods are re-admitted (carry).
4. **Dual verdicts.** Every conclusion carries a standing (formal) and a measure (statistical), kept apart and combined by a declared standard. A proposal is undecided by every standard.
5. **Dependencies.** kategoria, tekmerion, tyche, apodeixis and morphe directly; peira, nomos, heuresis, kinesis, physis and calx through them. kybernesis is the highest of the foundations, and never depends on a domain.
6. **Pure core.** Language models and files belong to the infrastructure's edges. A model's answer enters as a proposal.

## 3. Alternatives Considered

- **A domain** (beside the policy domain). Rejected, because the framework names no object in nature; the policy domain will be one of its users.
- **The architecture in code**, as classes for each block. Rejected, because the user asked for blocks to be added, removed and changed, which code makes a restructuring and data makes a revision.
- **Admission by declaration**, where a method states which laws it obeys. Rejected: a law claimed is not a law shown, and the face-value estimator, lawful in form and unsound, is the counterexample kept in the checks.
- **Laws decided directly in C++ without kategoria.** Kept only as a fallback for instances too large for a model. The user asked for categorical verification, and kategoria's models make the laws equations of string diagrams, the same as in the schema.

## 4. Consequences

- A new block is a few lines of script; whether the loop still holds together is answered at once.
- A method written for another purpose needs an adapter, and admission tells whether the adapter is faithful to the block.
- Checks are made on finite instances. A method's behaviour beyond them is the business of its sibling's own checks.
