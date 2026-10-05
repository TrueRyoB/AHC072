# Implementation plan ver 0.2

## Objective and non-negotiable model

The absolute score is `T + 100000E`, so clearing every slime dominates saving a few actions. The solver must first produce a legal replay that ends with `E = 0`; it then minimizes its emitted action count `T`.

- A move removes the top suffix of a stack, reverses that suffix, and appends it to the landing stack.
- Its range is determined by the **retained** source height: `1 <= l <= k + 1`. It is not determined by the height of the travelling block.
- Every crossed cell is floor. The landing height is checked before automatic homecoming and must be at most eight.
- After each move, homecoming is repeated at both changed cells while the top slime matches that cell's nest.
- Stack order therefore matters for mixed colors. A same-color travelling block is reversal-safe.

The former claim that the one-cell baseline always fits the action limit is deliberately removed. The solver relies on an exact replay and the 100000-action cap rather than an unproved distance estimate.

## Complex ideas reduced to implementable subproblems

| Strategy concern | Known subproblem | First implementation |
| --- | --- | --- |
| Legal move and backflip handling | Exact state transition system | A board simulator owns every stack, applies reversal and both endpoint cleanups, and rejects an illegal operation before it is emitted. |
| Routes around walls | Shortest paths on an unweighted graph | Breadth-first search returns a floor path whenever a slime or block needs a destination. |
| Other piles as jump stages | Greedy bounded-range path traversal | While a top block follows a straight run, keep every slime below it. A retained base of height `k` permits a jump of up to `k + 1`, so untouched piles become safe launch stages. |
| Different colors sharing a route resource | Persistent mixed-color station | Choose an occupied floor cell and add three nearby slimes, preferring distinct colors. The four-slime stack remains in place while other colors visit it, use its retained height for departure, and leave. |
| Choosing stations and their users | Demand-weighted station selection | Evaluate source cells as candidate stations. A user is selected when the first straight departure from the station saves more jumps than the detour to reach it. Stations must serve at least two colors. |
| Scheduling interactions | Build, use, release | First build the mixed base. Route every assigned user through it. Release the base to its nests only after the final assigned user. The direct solver is only a completion fallback when no station candidate replays legally. |

## Implementation sequence

1. Parse the board into terrain, nest locations, and bottom-to-top stacks.
2. Build a direct per-slime plan. This is the completion fallback.
3. Select mixed-color stations and a set of users from at least two colors for each candidate.
4. Build a four-slime base at the selected station, preserving it while every assigned user arrives, jumps away, and returns home.
5. Release the resting base only after its assigned demand is exhausted, then clear remaining slimes directly.
6. On every route, choose the longest straight legal jump permitted by the actual retained base and landing capacity.
7. Replay every candidate from the original state. Accept it only if every move is legal, all stacks are empty, and it uses at most 100000 operations.

## Deliberate boundary of this version

This version uses a mixed-color stack as a persistent base, while each travelling user is a one-slime top block. Moving a mixed travelling block and later splitting it at divergent nests remains a separate search layer because its reversal order must be planned explicitly.
