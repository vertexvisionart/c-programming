#include <stdlib.h>

struct TreeNode {
  int val;
  struct TreeNode *left;
  struct TreeNode *right;
};

int max(int a, int b) { return a > b ? a : b; }

int maxDepth(struct TreeNode *root) {

  if (root == NULL)
    return 0;

  int res = 0;

  res = max(maxDepth(root->left), maxDepth(root->right)) + 1;

  return res;
}
