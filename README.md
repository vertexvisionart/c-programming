# C Programming Exercises

Curated C exercises — algorithms, data structures, problem-solving.
Mostly C99.

## Layout

- [`ant/`](ant/) — multi-strategy solver for a constrained TSP-like
  problem (sector-chunking, greedy, sliding-window DP), plus stress
  generators.
- [`algorithms/`](algorithms/) — classic textbook examples: Euclidean
  GCD, balanced brackets, merge sort.
- [`leetcode/`](leetcode/) — short, function-only solutions in the
  LeetCode submission style.
- [`packs/`](packs/) — topical packs (numerical methods, graphs,
  parsers, data structures). ~140 exercises across `pack_15`–`pack_21`
  plus `22`–`25`, `123`, `test_scr`.

## Build

Per file, no project-wide build system:

```sh
cd <folder>
cc -O2 -Wall <file>.c -o <name>     # add -lm for ant/
```

## License

MIT — see [LICENSE](LICENSE).
