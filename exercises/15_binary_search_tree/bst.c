#include <stdio.h>
#include <stdlib.h>

/* ================================================================
   BINARY SEARCH TREE (BST)

   Property: for every node N,
     - all values in N's left subtree  are < N->value
     - all values in N's right subtree are > N->value

   This ordering makes search O(log n) on a balanced tree.
   ================================================================ */

typedef struct Node {
    int          value;
    struct Node *left;
    struct Node *right;
} Node;

/* --- Construction / destruction --- */
Node *node_create(int value);
Node *bst_insert(Node *root, int value);
void  bst_free(Node *root);

/* --- Search --- */
Node *bst_search(Node *root, int value);
int   bst_contains(Node *root, int value);
Node *bst_min(Node *root);
Node *bst_max(Node *root);

/* --- Deletion --- */
Node *bst_delete(Node *root, int value);

/* --- Traversal (all O(n)) --- */
void bst_inorder(Node *root);     /* left → node → right  → sorted ascending  */
void bst_preorder(Node *root);    /* node → left → right  → useful for copying */
void bst_postorder(Node *root);   /* left → right → node  → useful for freeing */

/* --- Metrics --- */
int  bst_height(Node *root);
int  bst_count(Node *root);

/* --- Visualisation --- */
void bst_print(Node *root, int depth);

/* ------------------------------------------------------------------ */

int main(void) {

    /* --- Build a tree --- */
    printf("=== Insert ===\n");
    Node *root = NULL;
    int values[] = {50, 30, 70, 20, 40, 60, 80, 10, 35, 45};
    int n = sizeof(values) / sizeof(values[0]);

    for (int i = 0; i < n; i++) {
        root = bst_insert(root, values[i]);
        printf("inserted %d\n", values[i]);
    }

    /* --- Visualise the tree --- */
    printf("\n=== Tree structure ===\n");
    bst_print(root, 0);

    /* --- Traversals --- */
    printf("\n=== Traversals ===\n");
    printf("inorder   (sorted): "); bst_inorder(root);   printf("\n");
    printf("preorder  (root-first): "); bst_preorder(root);  printf("\n");
    printf("postorder (root-last):  "); bst_postorder(root); printf("\n");

    /* --- Metrics --- */
    printf("\n=== Metrics ===\n");
    printf("count  = %d\n", bst_count(root));
    printf("height = %d\n", bst_height(root));
    printf("min    = %d\n", bst_min(root)->value);
    printf("max    = %d\n", bst_max(root)->value);

    /* --- Search --- */
    printf("\n=== Search ===\n");
    int targets[] = {40, 55, 10, 99};
    for (int i = 0; i < 4; i++) {
        Node *found = bst_search(root, targets[i]);
        printf("search(%2d): %s\n", targets[i], found ? "found" : "not found");
    }

    /* --- Deletion: three cases --- */
    printf("\n=== Delete ===\n");

    /* Case 1: delete a leaf (10) */
    printf("delete 10 (leaf):\n");
    root = bst_delete(root, 10);
    printf("  inorder: "); bst_inorder(root); printf("\n");

    /* Case 2: delete a node with one child (20 — now has no left child) */
    printf("delete 20 (one child):\n");
    root = bst_delete(root, 20);
    printf("  inorder: "); bst_inorder(root); printf("\n");

    /* Case 3: delete a node with two children (30) —
       replaced by its in-order successor (the minimum of the right subtree) */
    printf("delete 30 (two children):\n");
    root = bst_delete(root, 30);
    printf("  inorder: "); bst_inorder(root); printf("\n");

    /* Delete the root itself */
    printf("delete 50 (root):\n");
    root = bst_delete(root, 50);
    printf("  inorder: "); bst_inorder(root); printf("\n");

    printf("\n=== Final tree ===\n");
    bst_print(root, 0);
    printf("count=%d  height=%d\n", bst_count(root), bst_height(root));

    bst_free(root);
    return 0;
}

/* ------------------------------------------------------------------ */

Node *node_create(int value) {
    Node *n = malloc(sizeof(Node));
    if (n == NULL) { fprintf(stderr, "malloc failed\n"); exit(1); }
    n->value = value;
    n->left  = NULL;
    n->right = NULL;
    return n;
}

/* Insert returns the (possibly new) root — callers must assign the result. */
Node *bst_insert(Node *root, int value) {
    if (root == NULL) return node_create(value);
    if (value < root->value)
        root->left  = bst_insert(root->left,  value);
    else if (value > root->value)
        root->right = bst_insert(root->right, value);
    /* duplicate values are ignored */
    return root;
}

/* Postorder traversal frees children before the parent. */
void bst_free(Node *root) {
    if (root == NULL) return;
    bst_free(root->left);
    bst_free(root->right);
    free(root);
}

/* ------------------------------------------------------------------ */

Node *bst_search(Node *root, int value) {
    if (root == NULL || root->value == value) return root;
    if (value < root->value) return bst_search(root->left,  value);
    else                     return bst_search(root->right, value);
}

int bst_contains(Node *root, int value) {
    return bst_search(root, value) != NULL;
}

/* Leftmost node holds the minimum value. */
Node *bst_min(Node *root) {
    if (root == NULL || root->left == NULL) return root;
    return bst_min(root->left);
}

Node *bst_max(Node *root) {
    if (root == NULL || root->right == NULL) return root;
    return bst_max(root->right);
}

/* ------------------------------------------------------------------ */

/* Delete has three cases:
   1. Node is a leaf         — just free it.
   2. Node has one child     — replace node with that child.
   3. Node has two children  — replace value with in-order successor
                               (min of right subtree), then delete
                               the successor from the right subtree. */
Node *bst_delete(Node *root, int value) {
    if (root == NULL) return NULL;

    if (value < root->value) {
        root->left  = bst_delete(root->left,  value);
    } else if (value > root->value) {
        root->right = bst_delete(root->right, value);
    } else {
        /* Found the node to delete */
        if (root->left == NULL) {
            Node *tmp = root->right;
            free(root);
            return tmp;                     /* cases 1 and 2 (no left child) */
        }
        if (root->right == NULL) {
            Node *tmp = root->left;
            free(root);
            return tmp;                     /* case 2 (no right child) */
        }
        /* Case 3: two children — find in-order successor */
        Node *successor  = bst_min(root->right);
        root->value      = successor->value;
        root->right      = bst_delete(root->right, successor->value);
    }
    return root;
}

/* ------------------------------------------------------------------ */

void bst_inorder(Node *root) {
    if (root == NULL) return;
    bst_inorder(root->left);
    printf("%d ", root->value);
    bst_inorder(root->right);
}

void bst_preorder(Node *root) {
    if (root == NULL) return;
    printf("%d ", root->value);
    bst_preorder(root->left);
    bst_preorder(root->right);
}

void bst_postorder(Node *root) {
    if (root == NULL) return;
    bst_postorder(root->left);
    bst_postorder(root->right);
    printf("%d ", root->value);
}

int bst_height(Node *root) {
    if (root == NULL) return 0;
    int lh = bst_height(root->left);
    int rh = bst_height(root->right);
    return 1 + (lh > rh ? lh : rh);
}

int bst_count(Node *root) {
    if (root == NULL) return 0;
    return 1 + bst_count(root->left) + bst_count(root->right);
}

/* Print the tree rotated 90° — right subtree on top, root at left edge. */
void bst_print(Node *root, int depth) {
    if (root == NULL) return;
    bst_print(root->right, depth + 1);
    for (int i = 0; i < depth; i++) printf("    ");
    printf("%d\n", root->value);
    bst_print(root->left, depth + 1);
}
