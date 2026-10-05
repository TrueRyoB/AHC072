# problem description in a nutshell

- multi-agent path finding
- eliminate all slimes by moving them to their destination
- you can move a set of slimes together
- you can use other slimes as a jump
- even the greedy naive implementation beats the max operations permited of 100000, so only focus on minimizing the number of operations


## core ideas

### path formation

- co-slime refers to a set of slimes with a shared destination
- the path they should take is formed using Minimum-Spanning Tree, where nodes are slimes, destinations, and cells that are guaranteed to be taken
- it is not a traditional MST, though.
1. it is directed from slimes to destination
2. it folds with another dMST - dMST alters its path a bit if needed

### how to categorize slimes into a set of co-slimes

- Check ```./sub/form-co-slimes.md``` 
- Runtime complexity: safe

### how to form the optimal dMST on a grid for each co-slime

- Check ```./sub/form-MST-on-grid.md``` 
- Runtime complexity: safe

#### constraints about folding dMST

There exist two interactions between different co-slimes for the smaller score.

1. scaffolding - serve itself as a jump for other co-slimes
2. join - be carried by other co-slimes

It is sometimes better to tolerace some deviance from joints local increase for deviations to achieve the global minimum.

#### what is deviation and how to realize it

- Check ```./sub/searching-for-deviations.md``` 

#### how to handle deviation to achieve dMST folding and eventually assign movement order

- Check ```./sub/folding-MST.md``` 
