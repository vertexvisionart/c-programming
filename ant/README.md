# ant — multi-strategy traveling salesman variant

A constrained TSP-like problem: an "ant" starts at the origin, visits a
chosen subset of cities to collect prizes, and pays a cargo-weighted
travel cost. The decay factor `D` makes later sales less valuable, and
the cargo factor `C` makes longer tours linearly more expensive.

Three independent solvers, plus two test generators.

## Files

- `strategy_sector_chunk.c` — partitions cities by polar angle, then
  greedily extracts contiguous profitable sub-trips per sector.
- `strategy_greedy.c` — sorts cities by profit-per-distance, grows tours
  by picking the best next move.
- `strategy_dp_window.c` — sliding-window DP over angle-sorted and
  distance-sorted orderings; keeps whichever wins.
- `gen_ring.c` — cities on a noisy ring around the origin.
- `gen_stress.c` — four distribution types (clustered, grid, uniform,
  spiral) for stress testing.

## Build

```sh
cc -O2 -Wall -o sector strategy_sector_chunk.c -lm
cc -O2 -Wall -o greedy strategy_greedy.c -lm
cc -O2 -Wall -o dp     strategy_dp_window.c -lm
cc -O2 -Wall -o gen_ring gen_ring.c -lm
cc -O2 -Wall -o gen_stress gen_stress.c -lm
```

## Run

```sh
./gen_ring 5000 > input.txt
./sector  < input.txt > out.txt
```

Input: `N C D` then `N` lines of `x y p`. Output: tours separated by
`0 0`, each starting with `x y k` (k cities in this tour).
