#include "_BTree.h"

void initBTree(BTree* _btree, int _m, _CMP(*_cmp)(const void* e1, const void* e2)) {
  if (_m < 3) {
    printf("The order of BTree should be more than 2\n");
    exit(EXIT_FAILURE);
  }
  _btree->m = _m;
  _btree->cmp = _cmp;
  _btree->size = 0;
  _btree->min = (_m % 2 == 0) ? _m / 2 - 1 : _m / 2;
  _btree->head = malloc(sizeof(TreeNode));
  initTreeNode(_btree->head);
}
void initTreeNode(TreeNode* treeNode) {
  if (treeNode == NULL) {
    printf("The treeNode is NULL\n");
    return;
  }
  treeNode->size = 0;
  treeNode->child = NULL;
  treeNode->key = NULL;
  treeNode->next = NULL;
  treeNode->parent = NULL;
}
void initKeyNode(KeyNode* keyNode) {
  if (keyNode == NULL) {
    printf("The keyNode is NULL\n");
    return;
  }
  keyNode->next = NULL;
  keyNode->p = NULL;
  keyNode->value = NULL;
}

static void freeNode(TreeNode* node) {
  if (node == NULL) {
    return;
  }
  KeyNode* key = node->key;
  KeyNode* kn;
  TreeNode* child = node->child;
  TreeNode* next = node->next;
  free(node);
  node = NULL;
  while (key != NULL) {
    kn = key->next;
    free(key);
    key = NULL;
    key = kn;
  }
  freeNode(next);
  freeNode(child);
}

KeyNode* get(BTree* btree, const void* element) {
  TreeNode* m_head = btree->head;
  TreeNode* h = m_head->child;
  KeyNode* k = m_head->key;

  while (1) {
    if (btree->cmp(element, k->value) == EQUAL) {
      return k;
    }
    else if (btree->cmp(element, k->value) == SMALLER) {
      if (h == NULL) {
        return NULL;
      }
      k = h->key;
      h = h->child;
    }
    else {
      if (k->next == NULL) {
        if (h == NULL) {
          return NULL;
        }
        k = h->next->key;
        h = h->next->child;
      }
      else {
        k = k->next;
        h = h == NULL ? NULL : h->next;
      }
    }
  }
}

static TreeNode* split(BTree* btree, TreeNode* treeNode) {
  TreeNode* m_head = btree->head;
  KeyNode* kn;//分裂出准备插入到父结点的KeyNode
  KeyNode* ktemp;//分裂出准备插入到父结点的KeyNode的前继结点，也做辅助key结点
  TreeNode* newNode = malloc(sizeof(TreeNode));//分裂出的另一TreeNode
  initTreeNode(newNode);
  TreeNode* ttemp;//辅助TreeNode,标记老节点子链断开和新节点子链的开始
  BOOL isLeaf = treeNode->child == NULL;
  ktemp = treeNode->key;
  ttemp = treeNode->child;
  //找出分裂出的KeyNode和待分裂结点的分裂处
  for (int i = 0; i < btree->min - 1; i++) {
    ktemp = ktemp->next;
    if (!isLeaf) {
      ttemp = ttemp->next;
    }
  }
  kn = ktemp->next;
  //子链比keyNode链长1,故需要后移一位
  if (!isLeaf) {
    ttemp = ttemp->next;
  }

  //处理待分裂及分裂后结点的Key链及子链及size
  ktemp->next = NULL;
  ktemp = kn->next;
  kn->next = NULL;
  newNode->key = ktemp;
  while (ktemp != NULL) {
    ktemp->p = newNode;
    ktemp = ktemp->next;
  }
  if (!isLeaf) {
    newNode->child = ttemp->next;
    ttemp->next = NULL;
    ttemp = newNode->child;
    while (ttemp != NULL) {
      ttemp->parent = newNode;
      ttemp = ttemp->next;
    }
  }
  newNode->next = treeNode->next;
  newNode->parent = treeNode->parent;
  treeNode->next = newNode;
  treeNode->size = btree->min;
  newNode->size = btree->m - 1 - btree->min;

  //将分裂出的key插入父结点
  TreeNode* par = treeNode->parent;//父结点
  //1.当前结点为根节点，则需要新建根节点
  if (par == NULL) {
    par = malloc(sizeof(TreeNode));
    initTreeNode(par);
    par->size++;
    par->child = treeNode;
    treeNode->parent = par;
    newNode->parent = par;
    par->key = kn;
    kn->p = par;
    btree->head = par;
    return m_head;
  }

  //2.当前结点不为根节点
  kn->p = par;
  par->size++;
  ktemp = par->key;
  //2.1若当前结点为父结点的头节点，将key插入到头部
  if (treeNode == par->child) {
    kn->next = ktemp;
    par->key = kn;
    return par;
  }
  //2.2当前结点不为头结点
  while (ktemp->next != NULL && btree->cmp(ktemp->next->value, kn->value) == SMALLER) {
    ktemp = ktemp->next;
  }
  kn->next = ktemp->next;
  ktemp->next = kn;
  return par;
}

static BOOL insert(BTree* btree, KeyNode* node) {
  TreeNode* m_head = btree->head;
  TreeNode* h = m_head->child;
  KeyNode* k = m_head->key;
  KeyNode* kpre = NULL;
  void* element = node->value;
  BOOL isleaf;
  node->p = m_head;
  while (1) {
    isleaf = (h == NULL) ? TRUE : FALSE;
    if (isleaf && k == NULL) {
      if (kpre != NULL) {
        kpre->next = node;
      }
      else {
        node->p->key = node;
      }
      break;
    }
    if (k == NULL) {
      k = h->key;
      node->p = h;
      h = h->child;
      kpre = NULL;
      isleaf = (h == NULL) ? TRUE : FALSE;
    }
    if (btree->cmp(element, k->value) == EQUAL) {
      k->value = node->value;
      free(node);
      node = NULL;
      btree->size--;
      node = k;
      return FALSE;
    }
    else if (btree->cmp(element, k->value) == SMALLER) {
      if (isleaf) {
        node->next = k;
        if (kpre == NULL) {
          node->p->key = node;
        }
        else {
          kpre->next = node;
        }
        break;
      }
      else {
        k = h->key;
        kpre = NULL;
        node->p = h;
        h = h->child;
      }
    }
    else {
      if (!isleaf) {
        h = h->next;
      }
      kpre = k;
      k = k->next;
    }
  }
  node->p->size++;
  return TRUE;
}

KeyNode* put(BTree* btree, void* element) {
  btree->size++;
  KeyNode* node = malloc(sizeof(KeyNode));
  initKeyNode(node);
  node->value = element;
  if (!insert(btree,node)) {
    btree->size--;
    return node;
  }
  TreeNode* s = node->p;//当前结点

  while (1) {
    if (s->size < btree->m) {
      break;
    }
    s = split(btree,s);
  }
  return node;
}

static void getBros(BTree* btree,const TreeNode* self, TreeNode** left, TreeNode** right) {
  *left = NULL;
  *right = NULL;
  //the head node has no bro
  if (self == btree->head) {
    return;
  }
  *right = self->next;
  TreeNode* p = self->parent;
  //the self node is the first child of the p node,no left bro
  if (p->child == self) {
    return;
  }
  TreeNode* pre = p->child;
  while (pre->next != self) {
    pre = pre->next;
  }
  *left = pre;
}

static KeyNode* getParKeyNode(BTree* btree, TreeNode* left) {
  TreeNode* m_head = btree->head;
  if (left == m_head) {
    return NULL;
  }
  TreeNode* p = left->parent;
  TreeNode* temp = p->child;
  KeyNode* ktemp = p->key;
  while (temp != left) {
    temp = temp->next;
    ktemp = ktemp->next;
  }
  return ktemp;
}

static void borrowFromRight(BTree* btree, TreeNode* self, TreeNode* right) {
  TreeNode* m_head = btree->head;
  TreeNode* p = self->parent;
  KeyNode* pk;
  pk = getParKeyNode(btree,self);
  KeyNode* temp = malloc(sizeof(KeyNode));
  initKeyNode(temp);
  temp->value = pk->value;
  temp->p = self;
  pk->value = right->key->value;
  if (self->size == 0) {
    self->key = temp;
  }
  else {
    pk = self->key;
    while (pk->next != NULL) {
      pk = pk->next;
    }
    pk->next = temp;
  }
  temp = right->key;
  right->key = temp->next;
  free(temp); temp = NULL;
  if (self->child != NULL) {
    p = self->child;
    while (p->next != NULL) {
      p = p->next;
    }
    p->next = right->child;
    right->child = right->child->next;
    p->next->parent = self;
  }
  self->size++;
  right->size--;
}

static void borrowFromLeft(BTree* btree, TreeNode* self, TreeNode* left) {
  TreeNode* m_head = btree->head;
  TreeNode* p = self->parent;
  KeyNode* pk;
  pk = getParKeyNode(btree,left);
  KeyNode* temp = malloc(sizeof(KeyNode));
  initKeyNode(temp);
  KeyNode* kpre = NULL;
  temp->value = pk->value;
  temp->p = self;
  temp->next = self->key;
  self->key = temp;
  temp = left->key;
  while (temp->next != NULL) {
    kpre = temp;
    temp = temp->next;
  }
  pk->value = temp->value;
  kpre->next = NULL;
  free(temp);
  temp = NULL;
  TreeNode* tpre = NULL;
  if (left->child != NULL) {
    p = left->child;
    while (p->next != NULL) {
      tpre = p;
      p = p->next;
    }
    tpre->next = NULL;
    p->next = self->child->next;
    self->child = p;
    p->parent = self;
  }
  self->size++;
  left->size--;
}

static TreeNode* combine(BTree* btree, TreeNode* left, TreeNode* right) {
  TreeNode* m_head = btree->head;
  KeyNode* pk = getParKeyNode(btree,left);
  KeyNode* ktemp = left->parent->key;
  //连接父结点断开的key值
  if (pk == ktemp) {
    left->parent->key = pk->next;
  }
  else {
    while (ktemp->next != pk) {
      ktemp = ktemp->next;
    }
    ktemp->next = pk->next;
  }
  pk->next = NULL;
  left->next = right->next;
  left->size = left->size + right->size + 1;
  ktemp = left->key;
  //连接两结点及父结点key值列表
  if (ktemp == NULL) {
    left->key = pk;
  }
  else {
    while (ktemp->next != NULL) {
      ktemp = ktemp->next;
    }
    ktemp->next = pk;
  }
  pk->next = right->key;
  while (pk != NULL) {
    pk->p = left;
    pk = pk->next;
  }
  //连接两结点child
  if (left->child != NULL) {
    TreeNode* ttemp = left->child;
    while (ttemp->next != NULL) {
      ttemp = ttemp->next;
    }
    ttemp->next = right->child;
    ttemp = ttemp->next;
    while (ttemp != NULL) {
      ttemp->parent = left;
      ttemp = ttemp->next;
    }
  }
  free(right);
  right = NULL;
  left->parent->size--;
  return left->parent;
}

void remove_element(BTree* btree, const void* element) {
  TreeNode* m_head = btree->head;
  btree->size--;
  //查找删除结点的位置
  KeyNode* kn = get(btree,element);
  if (kn == NULL) {
    btree->size++;
    return;
  }
  //找到替代节点，直接后继
  TreeNode* rt = kn->p;;
  KeyNode* ktemp = rt->key;
  if (kn->p->child != NULL) {
    rt = rt->child;
    while (ktemp != kn) {
      ktemp = ktemp->next;
      rt = rt->next;
    }
    rt = rt->next;
    while (rt->child != NULL) {
      rt = rt->child;
    }
    ktemp = rt->key;
    kn->value = ktemp->value;
    kn = ktemp;
  }

  if (kn == rt->key) {
    rt->key = kn->next;
  }
  else {
    ktemp = rt->key;
    while (ktemp->next != kn) {
      ktemp = ktemp->next;
    }
    ktemp->next = kn->next;
  }
  rt->size--;
  free(kn);
  kn = NULL;

  /*
  lbro:左兄弟 rbro：右兄弟
  */
  TreeNode* lbro, * rbro;
  while (1) {
    if (rt->size >= btree->min) {
      break;
    }
    //当前结点为头节点
    if (rt == m_head) {
      //头节点空，替换头节点
      if (rt->size == 0) {
        btree->head = rt->child;
        free(rt);
        rt = NULL;
      }
      break;
    }
    lbro = NULL; rbro = NULL;
    getBros(btree,rt, &lbro, &rbro);
    if (rbro != NULL && rbro->size > btree->min) {
      borrowFromRight(btree,rt, rbro);
      break;
    }
    else if (lbro != NULL && lbro->size > btree->min) {
      borrowFromLeft(btree,rt, lbro);
      break;
    }
    else if (rbro != NULL) {
      rt = combine(btree,rt, rbro);
    }
    else {
      rt = combine(btree,lbro, rt);
    }
  }
}
void destory(BTree* btree) {
  free(btree->head);
}