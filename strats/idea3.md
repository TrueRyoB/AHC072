# core ideas

beam search - make a move that optimizes the board state

evaluation factor

- E(k, d): a k-1 height tower from a k distance away from the core joint to the path to the nest; for each contributing slimes
- D(): difference in the cumulative distance of slimes to their nests



---

We need to build a good evaluation factor so that we can take advantage of both scaffolding and merging. 
To build E(k, d) efficiently, we need to keep track of the topological order of slimes and make sure to save them at the corner or something to perform a so-called "reserve" action for other slimes to use itself. It is also very important to use the persistent data structure or something to reduce the runtime complexity to judge every stage. 
For a node at distance k-1, a height of exactly k is preferred more than k+n (n>0).


it is important to take advantage of the information distribution to decide the best improment possible.




## potential improvements

- recurrent subordinization as an algorithm to maximize carriage potential (some cases include 24 slimes of the same color like wow)
- isolated slimes should cooperate no matter what
- higher tower should not be built - they should remain low for other slimes on path to use
- categorization should consider the height limit of 8 and the slime density as a useful means
- distributed tower building to support the most prevalent travel distance need of 4
- do not jump forward too much when there are other slimes in need of using you as a scaffold

(origin of being reserve?!)


---

(note from chatgpt about my plan)


I think your intuition is reasonable: **the strategy you have reached is not an obvious baseline**, but I would be careful about interpreting that as "most participants won't find it." In AHC, strong contestants often arrive at surprisingly similar ideas through completely different abstractions.

The interesting part is the sequence of insights you've reached:

1. **dMST** as a way of organizing desired slime relationships.
2. Realizing that dMSTs need not intersect to interact.
3. Recognizing **scaffolding** as potentially much more important than direct movement.
4. Realizing that the useful property of a state is not necessarily its distance to completion, but **what cheap actions the state makes possible**.
5. Using dMST mainly for **candidate reduction**, rather than treating it as the solution structure.
6. Moving toward **beam search over physical states**, with a heuristic measuring future jumpability/scaffolding.

That is a substantially more sophisticated trajectory than "greedy nearest slime to nearest nest."

### Where I think people can get trapped

There are several very plausible local optima.

**Pitfall 1: shortest-path thinking**

A natural first model is:

```text
slime → its nest
```

and then optimize movement distance.

But the operation cost isn't proportional to Euclidean/Manhattan distance in the obvious way. A tower can make one operation move a whole group, so spatial distance is not the fundamental resource.

---

**Pitfall 2: treating each color independently**

Because every slime eventually belongs to one nest, it is extremely natural to construct:

```text
color 0 solution
color 1 solution
...
```

But cross-color towers make this decomposition fundamentally wrong.

---

**Pitfall 3: MST as the final structure**

This is the more sophisticated trap.

Someone may discover that a tree-like structure is useful and conclude:

> "The optimal solution is approximately the MST."

But the actual operations are **state transformations**, and temporary structures may have enormous value despite not belonging to any final MST.

You have already identified this distinction.

---

**Pitfall 4: optimizing immediate operation reduction**

A greedy heuristic might evaluate:

$$
\Delta T
$$

for the next action.

But a scaffolding action can have:

$$
\Delta T_{\text{immediate}}\approx 0
$$

while dramatically increasing the number of cheap future actions.

That's precisely where beam search can outperform greedy search.

---

### The part I think is genuinely non-obvious

This:

> **A good board state may be one that has not made much visible progress toward the goal, but has created a large number of cheap future transitions.**

That's a fairly deep AHC observation.

It changes the evaluation function from something like

$$
V(S)=-\text{remaining distance}
$$

to

$$
V(S)
\approx
\text{completed work}
+
\text{future action potential}.
$$

And your specific observation about a tower of height \(h\) being valuable when positioned around \(h-1\) cells from a useful destination/junction is exactly the sort of **domain-specific potential function** that can make beam search work.

---

### But there is a danger in the opposite direction

I would not assume:

> "Nobody else has thought of this."

Top AHC contestants are very good at discovering structural properties quickly, and several people may independently arrive at:

* beam search,
* tower potential,
* future-action evaluation,
* cross-color scaffolding,
* local simulation,
* randomized construction.

The differentiation may instead be in **how accurately they evaluate a state and how efficiently they search it**.

In other words, the strategic idea might be shared by the top 5–20%, while the implementation details determine whether it becomes top 5% or top 30%.

---

### Your current top-30% result actually fits this

If your current solution is around top 30% **before implementing the beam/scaffolding-state evaluation**, I wouldn't interpret that as evidence that the conceptual approach is failing.

I'd interpret it as:

```text
good structural understanding
        ↓
reasonable first solution
        ↓
not yet enough state-space optimization
```

The next question is therefore empirical:

> **Does your scaffolding potential predict which intermediate states are actually good?**

If yes, you may have found the missing search heuristic.

If no, then the interesting realization was still useful, because it tells you exactly what hypothesis failed.

That's the AHC mindset I'd use here: **don't try to determine whether your idea is clever enough; measure whether it gives a useful ordering over states.**
