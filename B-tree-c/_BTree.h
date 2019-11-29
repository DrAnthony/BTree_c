#ifndef _BTREE_H
#define _BTREE_H
#include <stdlib.h>
#include <stdio.h>

enum E_BOOL_VALUE { FALSE, TRUE };
enum E_CMP { BIGGER, EQUAL, SMALLER };

typedef int BOOL;
typedef enum E_CMP _CMP;
typedef struct _KeyNode KeyNode;
typedef struct _TreeNode TreeNode;
typedef struct _BTree BTree;

struct _TreeNode {
  int size;
  KeyNode* key;
  TreeNode* next;
  TreeNode* child;
  TreeNode* parent;
};
struct _KeyNode {
  void* value;
  TreeNode* p;
  KeyNode* next;
};

struct _BTree {
  int m;
  int size;
  int min;
  TreeNode* head;
  _CMP(*cmp)(const void*, const void*);
};
void initBTree(BTree* _btree, int _m, _CMP(*_cmp)(const void* e1, const void* e2));
static void initTreeNode(TreeNode* treeNode);
static void initKeyNode(KeyNode* keyNode);
static void freeNode(TreeNode*);

KeyNode* get(BTree*,const void*);

static TreeNode* split(BTree* btree,TreeNode*);
static BOOL insert(BTree* btree, KeyNode*);
KeyNode* put(BTree* btree, void*);

static void getBros(BTree* btree, const TreeNode* self, TreeNode** left, TreeNode** right);
static KeyNode* getParKeyNode(BTree* btree, TreeNode* l);
static void borrowFromRight(BTree* btree, TreeNode* self, TreeNode* right);
static void borrowFromLeft(BTree* btree, TreeNode* self, TreeNode* right);
static TreeNode* combine(BTree* btree, TreeNode* self, TreeNode* another);
void remove_element(BTree* btree, const void* element);
void destory(BTree* btree);
#endif //_BTREE_H
