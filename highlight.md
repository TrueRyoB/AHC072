# AHC solver handoff

## Current solver

`main.cpp` is an exact state simulator with a receding-horizon beam search.
It stores every tower bottom-to-top, applies reversals and automatic homing at
both endpoints, and emits only the operations simulated internally.

Search parameters:

- beam depth: 8
- beam width: 10
- candidates per node: 16
- beam budget: 0.90 seconds
- greedy group-move budget: 1.18 seconds
- hard solve cutoff: 1.45 seconds, leaving output time

After the search budget, the solver uses group-progress moves, then monotone
single-slime shortest-path moves. A full landing cell is opened by propagating
an empty capacity slot backward along a legal adjacent path.

## Combined technical flow forest

For each color, the solver builds a canonical shortest-path arborescence from
all floor cells to that color's nest. In every beam state it inserts every
live slime into its color tree and accumulates its demand toward the root.

The union of all colors is the **combined flow forest**:

- `flow[color][cell]` is the number of live slimes whose technical shortest
  route passes through that cell.
- A joint is a cell carrying at least two units of flow, with either multiple
  colors or at least two upstream units.
- Isolated occupied cells can attach to a nearby joint in an 8-neighbor
  diagonal planning graph when the estimated detour is at most two steps.
- Diagonal links only shape evaluation; emitted moves remain legal orthogonal
  jumps.

This is a dynamic shortest-path forest, not an exact directed Steiner-tree or
MST solver. It is cheap enough to rebuild for every beam state.

## Reserve value and release debt

Tower height has no intrinsic reward. A tower at cell `v` with height `h`
receives value only from external unresolved flow:

```
sum over colors c:
    external_flow(c, v) * supported_jump_benefit(c, v, h)
```

`external_flow` subtracts slimes already in the tower, then adds eligible
isolated branches attached to the joint. Height 8 earns no reserve value,
because another slime cannot land on it.

Moving a tower can lower the supported jump benefit for flow through its cell.
`releaseDebt` applies a large candidate penalty, including at least 15000 per
affected external flow unit, so the search retains a base until its dependent
branches have caught up.

## Same-color coalescence

For a contiguous top run of `m` slimes of color `c`, the evaluation includes
an estimate of the cumulative operations saved by moving them together:

```
6 * (m - 1) * routeCost(distance_to_nest(c))
```

Candidate ranking adds the corresponding merge bonus only when a monochrome
moved segment lands on a matching monochrome top run. This makes long-route
same-color merges compete directly with scaffolding moves.

## Cycle prevention

The beam forbids an immediate reverse edge, deduplicates equal states at each
depth, and heavily penalizes a state equal to the beam root or to one of the
last 32 executed states. With one slime left, it bypasses beam search and
uses only a shortest-path step.

## Verification status

The solution program has not been executed in this workspace. The configured
UCRT64 compiler path is absent here, so the latest `main.cpp` has not been
compiled locally.
