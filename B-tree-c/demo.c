#include "_BTree.h"

_CMP cmp(const void* a, const void* b) {
  if (*(int*)a < *(int*)b) {
    return SMALLER;
  }
  else if (*(int*)a > * (int*)b) {
    return BIGGER;
  }
  else {
    return EQUAL;
  }
}
void inorder(TreeNode* h, int level) {
  if (h == NULL) {
    return;
  }
  KeyNode* k = h->key;
  TreeNode* child = h->child;
  printf("level:%d ", level);
  while (k != NULL) {
    printf("%d ", *(int*)k->value);
    k = k->next;
  }
  printf("\n");
  while (child != NULL) {
    inorder(child, level + 1);
    child = child->next;
  }
}
int main() {
  int map[] = {
    5,6,1,4,5,14,52,17,84,100,26,58,95,47,
    1,4,5,3,0,48,78,95,65,14,789,145,14,
    17,87,25,26,111,201,102,458,5555,65,14,
    230,-1,549,777,1024,256,2048,10001,10002,10003,
    10004,10005,10006,10007
  };
  int len = sizeof(map) / sizeof(int);
  BTree tree;
  initBTree(&tree, 5, &cmp);
  for (int i = 0; i < len; i++) {
    put(&tree,&map[i]);
  }
  int a = 6;
  inorder(tree.head, 0);
  printf("======================\n");
  remove_element(&tree,&a);
  inorder(tree.head, 0);
  printf("======================\n");
  a = 14;
  remove_element(&tree, &a);
  inorder(tree.head, 0);
  printf("======================\n");
  a = 17;
  remove_element(&tree, &a);
  inorder(tree.head, 0);
  printf("======================\n");
  a = 789;
  remove_element(&tree, &a);
  inorder(tree.head, 0);
  return 0;
}