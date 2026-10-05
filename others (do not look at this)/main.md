# `main.cpp` map

## Solver contract

The solver outputs only legal moves, replays each selected candidate from the original state, and accepts a candidate only if every slime has returned home within 100000 operations.

The planning invariant is:

- A travelling subgroup is a contiguous top run of one color.
- Landing on the same color merges that run before its next departure.
- Landing on another color is only allowed while the currently executing route continues immediately. It is a transient launch base, not a new scheduled subgroup.
- A persistent mixed-color stack is allowed only at a selected station. A user lands there, departs immediately, and leaves the mixed base below it unchanged.

## Constants and data records

| Name | Purpose |
| --- | --- |
| `MAX_ACTIONS`, `MAX_HEIGHT` | Contest limits. |
| `DR`, `DC`, `DIR` | Direction vectors and output letters. |
| `Point` | A grid coordinate; its operators compare coordinates. |
| `Move` | One output operation: source, retained count, direction, and range. |
| `SlimeRef` | An initial slime's position and color. It is used only while selecting and building a station. |
| `StationSpec` | A station cell, four base slimes, assigned users, and its static ranking score. |

## `Board`

`Board` is the exact mutable simulator. `stack_at[r][c]` stores every tower bottom-to-top.

| Method | Purpose |
| --- | --- |
| `inside` | Checks board bounds. |
| `floor` | Checks that a cell is inside and is not a wall. |
| `height` | Returns current stack height. |
| `top_color` | Returns the top color, or `-1` for an empty stack. |
| `top_run` | Counts the contiguous top slimes of one color. This defines a mergeable subgroup. |
| `homecoming` | Removes matching top slimes repeatedly at one nest. |
| `apply` | Validates one move, removes and reverses its top suffix, checks pre-homecoming landing height, and performs homecoming at both changed endpoints. |
| `empty` | Checks that all slimes have left the board. |

## Route helpers

| Function | Purpose |
| --- | --- |
| `block_distances` | BFS distances for a block of a specified size. A landing cell is usable only when it can hold that block. |
| `block_path` | Reconstructs one such BFS path. It may cross occupied cells because a jump can pass over them. |
| `first_straight_run` | Measures the first straight segment from a station to a nest; this estimates the station's first-jump benefit. |

## `Planner`

`Planner` owns a `Board` copy and the candidate move list.

| Method | Purpose |
| --- | --- |
| `emit` | Applies and records a legal move, enforcing the action cap. |
| `move_group` | Moves a top monochromatic block toward a goal. When `merge_same` is true, it adds every same-color top run found at a landing cell to the travelling count before planning the next jump. When false, it moves exactly `fixed_count` top slimes and preserves the stack below. Station construction uses this controlled mode. |
| `finish` | Clears the remaining board. It first chooses a same-color target that is closer than the current group's nest, merges there, and otherwise sends the longest reachable top run to its nest. If a full run cannot fit a route, it moves its top slime to make progress and allow a later merge. |

## Station selection and use

| Function | Purpose |
| --- | --- |
| `station_specs` | Tests each initial slime cell as a station. It selects three nearby donors, preferring distinct colors, to create a height-four mixed lower base. It then keeps users whose first station jump repays their station detour. Users must span at least two colors and cannot have the station base's protected top color. |
| `station_solution` | Builds the base without merging it, routes one user at a time onto and immediately away from the station, then calls `Planner::finish` to release the base and clear all remaining groups with same-color merges enabled. |
| `replay_is_complete` | Replays an entire candidate from the original board as the final legality and completion check. |

## `main`

1. Parse terrain, nests, and initial slimes.
2. Generate station specifications and replay the best eight.
3. Keep the shortest complete station candidate.
4. If no station candidate completes, run `Planner::finish` from the initial state.
5. Print the retained move list.

## Current missing points

- The station score is still a static estimate. It does not price later changes in stack height or route occupancy.
- `finish` chooses merges from shortest-path distances, rather than replaying a wider search over merge order.
- Mixed-color travelling blocks are deliberately excluded. Supporting them requires tracking their full order and planning how nests split that order.
- Only one persistent station is used in a candidate. Multiple stations and handoff routes are not yet searched.
