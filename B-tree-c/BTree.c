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
  KeyNode* kn;//���ѳ�׼�����뵽������KeyNode
  KeyNode* ktemp;//���ѳ�׼�����뵽������KeyNode��ǰ�̽�㣬Ҳ������key���
  TreeNode* newNode = malloc(sizeof(TreeNode));//���ѳ�����һTreeNode
  initTreeNode(newNode);
  TreeNode* ttemp;//����TreeNode,����Ͻڵ������Ͽ����½ڵ������Ŀ�ʼ
  BOOL isLeaf = treeNode->child == NULL;
  ktemp = treeNode->key;
  ttemp = treeNode->child;
  //�ҳ����ѳ���KeyNode�ʹ����ѽ��ķ��Ѵ�
  for (int i = 0; i < btree->min - 1; i++) {
    ktemp = ktemp->next;
    if (!isLeaf) {
      ttemp = ttemp->next;
    }
  }
  kn = ktemp->next;
  //������keyNode����1,����Ҫ����һλ
  if (!isLeaf) {
    ttemp = ttemp->next;
  }

  //��������Ѽ����Ѻ����Key����������size
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

  //�����ѳ���key���븸���
  TreeNode* par = treeNode->parent;//�����
  //1.��ǰ���Ϊ���ڵ㣬����Ҫ�½����ڵ�
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

  //2.��ǰ��㲻Ϊ���ڵ�
  kn->p = par;
  par->size++;
  ktemp = par->key;
  //2.1����ǰ���Ϊ������ͷ�ڵ㣬��key���뵽ͷ��
  if (treeNode == par->child) {
    kn->next = ktemp;
    par->key = kn;
    return par;
  }
  //2.2��ǰ��㲻Ϊͷ���
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
  TreeNode* s = node->p;//��ǰ���

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
  //���Ӹ����Ͽ���keyֵ
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
  //��������㼰�����keyֵ�б�
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
  //���������child
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
  //����ɾ������λ��
  KeyNode* kn = get(btree,element);
  if (kn == NULL) {
    btree->size++;
    return;
  }
  //�ҵ�����ڵ㣬ֱ�Ӻ��
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
  lbro:���ֵ� rbro�����ֵ�
  */
  TreeNode* lbro, * rbro;
  while (1) {
    if (rt->size >= btree->min) {
      break;
    }
    //��ǰ���Ϊͷ�ڵ�
    if (rt == m_head) {
      //ͷ�ڵ�գ��滻ͷ�ڵ�
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