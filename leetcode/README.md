# leetcode — short standalone solutions

Function-only solutions in the LeetCode submission style. Each file
exposes the entry point that the judge calls; there is no `main`. To
test locally, drop the function into a small driver.

| File | Problem |
|------|---------|
| `best_time_to_buy_and_sell_stock.c` | 121 — single-pass min/max |
| `binary_tree_inorder_traversal.c`   | 94 — recursive in-order walk |
| `climbing_stairs.c`                 | 70 — Fibonacci-style DP |
| `first_bad_version.c`               | 278 — binary search |
| `invert_binary_tree.c`              | 226 — recursive swap |
| `majority_element.c`                | 169 — sort and pick the middle |
| `maximum_depth_of_binary_tree.c`    | 104 — recursive depth |
| `maximum_subarray.c`                | 53 — Kadane's algorithm |
| `merge_sorted_array.c`              | 88 — in-place merge from the back |
| `merge_two_sorted_lists.c`          | 21 — sentinel-node merge |

## Syntax check

```sh
cc -c -O2 -Wall -o /dev/null <file>.c
```

Note: `first_bad_version.c` references the judge-provided `isBadVersion`
and won't link standalone.
