/* LeetCode 94: Binary Tree Inorder Traversal — recursive. */
#include <stdlib.h>

struct TreeNode {
    int val;
    struct TreeNode *left;
    struct TreeNode *right;
};

static void helper(int *returnSize, struct TreeNode *root, int *arr) {
    if (root == NULL) return;
    helper(returnSize, root->left, arr);
    arr[(*returnSize)++] = root->val;
    helper(returnSize, root->right, arr);
}

int *inorderTraversal(struct TreeNode *root, int *returnSize) {
    int *res = (int *)malloc(100 * sizeof(int));
    *returnSize = 0;
    helper(returnSize, root, res);
    return res;
}
