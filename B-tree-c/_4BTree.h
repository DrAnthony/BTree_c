#ifndef _BTREE_H
#define _BTREE_H
#include <stdlib.h>
#include <stdio.h>

enum _BOOL_VALUE { FALSE, TRUE };
enum _CMP { BIGGER, EQUAL, SMALLER };
typedef int BOOL;

template <typename ElementType>
struct KeyNode;

template <typename ElementType>
struct TreeNode {
  int size = 0;
  KeyNode<ElementType>* key = NULL;
  TreeNode<ElementType>* next = NULL;
  TreeNode<ElementType>* child = NULL;
  TreeNode<ElementType>* parent = NULL;
};

template <typename ElementType>
struct KeyNode {
  ElementType value;
  TreeNode<ElementType>* p = NULL;
  KeyNode<ElementType>* next = NULL;
};

template <typename ElementType>
class BTree {
private:
  int m_m;
  _CMP(*m_cmp)(const ElementType&, const ElementType&);//比较函数
  TreeNode<ElementType>* m_head;
  int m_size;
  int m_min;
  void freeNode(TreeNode<ElementType>*);
  BOOL insert(KeyNode<ElementType>*);
  TreeNode<ElementType>* split(TreeNode<ElementType>*);
  void getBros(TreeNode<ElementType>* self, TreeNode<ElementType>* left, TreeNode<ElementType>* right);
  KeyNode<ElementType>* getPKeyNode(TreeNode<ElementType>* l);
  void borrowFromRight(TreeNode<ElementType>* self, TreeNode<ElementType>* right);
  void borrowFromLeft(TreeNode<ElementType>* self, TreeNode<ElementType>* right);
  TreeNode<ElementType>* combine(TreeNode<ElementType>* self, TreeNode<ElementType>* another);
public:
  BTree() :m_size(0), m_head(new TreeNode<ElementType>) {};
  ~BTree();
  void init(int, _CMP(*_cmp)(const ElementType& e1, const ElementType& e2));
  TreeNode<ElementType>* getHead();
  KeyNode<ElementType>* get(const ElementType&);
  KeyNode<ElementType>* put(ElementType);
  void remove(const ElementType);
  int length();
  void destory();
};

template <typename ElementType>
void BTree<ElementType>::init(int _m, _CMP(*_cmp)(const ElementType& e1, const ElementType& e2)) {
  if (_m <= 2) {
    std::cout << "参数_m不能小于2" << std::endl;
    destory();
    exit(EXIT_FAILURE);
  }
  m_cmp = _cmp;
  m_m = _m;
  m_min = m_m % 2 == 0 ? m_m / 2 - 1 : m_m / 2;
}
template <typename ElementType>
BTree<ElementType>::~BTree() {
  destory();
}
template <typename ElementType>
void BTree<ElementType>::freeNode(TreeNode<ElementType>* node) {
  if (node == NULL) {
    return;
  }
  KeyNode<ElementType>* key = node->key;
  KeyNode<ElementType>* kn;
  TreeNode<ElementType>* child = node->child;
  TreeNode<ElementType>* next = node->next;
  delete node;
  node = NULL;
  while (key != NULL) {
    kn = key->next;
    delete key;
    key = NULL;
    key = kn;
  }
  freeNode(next);
  freeNode(child);
}
template <typename ElementType>
int BTree<ElementType>::length() {
  return m_size;
}
template <typename ElementType>
void BTree<ElementType>::destory() {
  freeNode(m_head);
}
template <typename ElementType>
TreeNode<ElementType>* BTree<ElementType>::getHead() {
  return m_head;

}
template <typename ElementType>
KeyNode<ElementType>* BTree<ElementType>::get(const ElementType& element) {
  TreeNode<ElementType>* h = m_head->child;
  KeyNode<ElementType>* k = m_head->key;

  while (1) {
    if (m_cmp(element, k->value) == EQUAL) {
      return k;
    }
    else if (m_cmp(element, k->value) == SMALLER) {
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

//将新节点插入到叶子节点，若成功插入返回TRUE，node不变，若已存在该节点，返回FALSE，
//node更新为树中已存在的结点地址
template <typename ElementType>
BOOL BTree<ElementType>::insert(KeyNode<ElementType>* node) {
  TreeNode<ElementType>* h = m_head->child;
  KeyNode<ElementType>* k = m_head->key;
  KeyNode<ElementType>* kpre = NULL;
  ElementType element = node->value;
  BOOL isLeaf;
  node->p = m_head;
  while (1) {
    isLeaf = (h == NULL) ? TRUE : FALSE;
    if (isLeaf && k == NULL) {
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
      isLeaf = (h == NULL) ? TRUE : FALSE;
    }
    if (m_cmp(element, k->value) == EQUAL) {
      k->value = node->value;
      delete node;
      node = NULL;
      m_size--;
      node = k;
      return FALSE;
    }
    else if (m_cmp(element, k->value) == SMALLER) {
      if (isLeaf) {
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
      if (!isLeaf) {
        h = h->next;
      }
      kpre = k;
      k = k->next;
    }
  }
  node->p->size++;
  return TRUE;
}

//分裂操作，将size>m-1结点分裂，并插入到父结点，返回父结点
template <typename ElementType>
TreeNode<ElementType>* BTree<ElementType>::split(TreeNode<ElementType>* treeNode) {
  KeyNode<ElementType>* kn;//分裂出准备插入到父结点的KeyNode
  KeyNode<ElementType>* ktemp;//分裂出准备插入到父结点的KeyNode的前继结点，也做辅助key结点
  TreeNode<ElementType>* newNode = new TreeNode<ElementType>;//分裂出的另一TreeNode
  TreeNode<ElementType>* ttemp;//辅助TreeNode,标记老节点子链断开和新节点子链的开始
  BOOL isLeaf = treeNode->child == NULL;
  ktemp = treeNode->key;
  ttemp = treeNode->child;
  //找出分裂出的KeyNode和待分裂结点的分裂处
  for (int i = 0; i < m_min - 1; i++) {
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
  treeNode->size = m_min;
  newNode->size = m_m - 1 - m_min;

  //将分裂出的key插入父结点
  TreeNode<ElementType>* par = treeNode->parent;//父结点
  //1.当前结点为根节点，则需要新建根节点
  if (par == NULL) {
    par = new TreeNode<ElementType>;
    par->size++;
    par->child = treeNode;
    treeNode->parent = par;
    newNode->parent = par;
    par->key = kn;
    kn->p = par;
    m_head = par;
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
  while (ktemp->next != NULL && m_cmp(ktemp->next->value, kn->value) == SMALLER) {
    ktemp = ktemp->next;
  }
  kn->next = ktemp->next;
  ktemp->next = kn;
  return par;
}

template <typename ElementType>
KeyNode<ElementType>* BTree<ElementType>::put(ElementType element) {
  m_size++;
  KeyNode<ElementType>* node = new KeyNode<ElementType>;
  node->value = element;
  if (!insert(node)) {
    m_size--;
    return node;
  }
  TreeNode<ElementType>* s = node->p;//当前结点

  while (1) {
    if (s->size < m_m) {
      break;
    }
    s = split(s);
  }
  return node;
}

template <typename ElementType>
void BTree<ElementType>::remove(const ElementType element) {
  m_size--;
  //查找删除结点的位置
  KeyNode<ElementType>* kn = get(element);
  if (kn == NULL) {
    m_size++;
    return;
  }
  //找到替代节点，直接后继
  TreeNode<ElementType>* rt = kn->p;;
  KeyNode<ElementType>* ktemp = rt->key;
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
  delete kn;
  kn = NULL;

  /*
  lbro:左兄弟 rbro：右兄弟 rtp:父结点
  */
  TreeNode<ElementType>* lbro, * rbro, * rtp;
  TreeNode<ElementType>* lchild, * rchild;
  KeyNode<ElementType>* kpre = NULL;
  TreeNode<ElementType>* tt;
  while (1) {
    //当前结点长度符合要求
    if (rt->size >= m_min) {
      return;
    }

    //当前结点为头节点
    if (rt == m_head) {
      //头节点空，替换头节点
      if (rt->size == 0) {
        m_head = rt->child;
        delete rt;
        rt = NULL;
      }
      return;
    }
    rtp = rt->parent;
    tt = rtp->child;
    rbro = rt->next;
    lbro = rtp->child;
    //找到兄弟结点
    if (lbro == rt) {
      lbro = NULL;
    }
    else {
      while (lbro->next != rt) {
        lbro = lbro->next;
      }
    }
    kpre = NULL;
    tt = rtp->child;
    //右兄弟结点不为空且可以借
    if (rbro != NULL && rbro->size > m_min) {
      lchild = rt->child;
      rchild = rbro->child;
      if (tt == rt) {
        ktemp = rtp->key;
        rtp->key = rbro->key;
        rtp->key->p = rtp;
        rbro->key = rtp->key->next;
        rtp->key->next = ktemp->next;
        kpre = rt->key;
        if (kpre == NULL) {
          rt->key = ktemp;
        }
        else {
          while (kpre->next != NULL) {
            kpre = kpre->next;
          }
          kpre->next = ktemp;
        }
        ktemp->p = rt;
      }
      else {
        kpre = rtp->key;
        while (tt->next != rt) {
          tt = tt->next;
          kpre = kpre->next;
        }
        ktemp = kpre->next;
        rbro->key->p = kpre->p;
        kpre->next = rbro->key;
        rbro->key = rbro->key->next;
        kpre->next->next = ktemp->next;
        kpre = rt->key;
        if (kpre == NULL) {
          rt->key = ktemp;
        }
        else {
          while (kpre->next != NULL) {
            kpre = kpre->next;
          }
          kpre->next = ktemp;
        }
        ktemp->p = rt;
        ktemp->next = NULL;
      }
      if (lchild != NULL) {
        while (lchild->next != NULL)
        {
          lchild = lchild->next;
        }
        lchild->next = rchild;
        rbro->child = rchild->next;
        rchild->next = NULL;
        rchild->parent = rt;
      }
      rt->size++;
      rbro->size--;
      return;
    }
    //左兄弟结点不为空且可以借
    else if (lbro != NULL && lbro->size > m_min) {
      lchild = lbro->child;
      rchild = rt->child;
      if (tt == lbro) {
        ktemp = lbro->key;
        kpre = NULL;
        while (ktemp->next != NULL) {
          kpre = ktemp;
          ktemp = ktemp->next;
        }
        kpre->next = NULL;
        kpre = rtp->key;
        rtp->key = ktemp;
        ktemp->p = rtp;
        ktemp->next = kpre->next;
        kpre->p = rt;
        kpre->next = rt->key;
        rt->key = kpre;
      }
      else {
        kpre = rtp->key;
        while (tt->next != lbro) {
          tt = tt->next;
          kpre = kpre->next;
        }
        ktemp = kpre->next;
        kpre->next = ktemp->next;
        ktemp->p = rt;
        ktemp->next = rt->key;
        rt->key = ktemp;
        ktemp = lbro->key;
        KeyNode<ElementType>* kkt = NULL;
        while (ktemp->next != NULL) {
          kkt = ktemp;
          ktemp = ktemp->next;
        }
        kkt->next = NULL;
        ktemp->p = rtp;
        ktemp->next = kpre->next;
        kpre->next = ktemp;
      }
      if (lchild != NULL) {
        while (lchild->next->next != NULL)
        {
          lchild = lchild->next;
        }
        lchild->next->next = rchild;
        rt->child = lchild->next;
        lchild->next->parent = rt;
        lchild->next = NULL;
      }
      rt->size++;
      lbro->size--;
      return;
    }
    //不可借，两结点合并，对应的key值加入合并后的结点，父结点删除该key，将父结点作为当前结点继续向上调节
    else {
      TreeNode<ElementType>* l, * r;
      if (rbro != NULL) {
        l = rt; r = rbro;
      }
      else {
        l = lbro; r = rt;
      }
      lchild = l->child;
      rchild = r->child;
      if (tt == l) {
        ktemp = rtp->key;
        rtp->key = ktemp->next;
        l->size = l->size + 1 + r->size;
        l->next = r->next;
        ktemp->p = l;
        kpre = l->key;
        if (kpre != NULL) {
          while (kpre->next != NULL) {
            kpre = kpre->next;
          }
          kpre->next = ktemp;
          ktemp->next = r->key;
          ktemp = ktemp->next;
        }
        else {
          l->key = ktemp;
          ktemp->next = r->key;
          ktemp = ktemp->next;
        }
        while (ktemp != NULL) {
          ktemp->p = l;
          ktemp = ktemp->next;
        }
        if (lchild != NULL) {
          while (lchild->next != NULL)
          {
            lchild = lchild->next;
          }
          lchild->next = rchild;
          lchild = lchild->next;
          while (lchild != NULL)
          {
            lchild->parent = l;
            lchild = lchild->next;
          }
        }
      }
      else {
        kpre = rtp->key;
        while (tt->next != l) {
          tt = tt->next;
          kpre = kpre->next;
        }
        ktemp = kpre->next;
        kpre->next = ktemp->next;
        kpre = l->key;
        l->size = l->size + 1 + r->size;
        l->next = r->next;
        ktemp->p = l;
        if (kpre == NULL) {
          l->key = r->key;
          ktemp = l->key;
        }
        else {
          while (kpre->next != NULL) {
            kpre = kpre->next;
          }
          kpre->next = ktemp;
          ktemp->next = r->key;
          ktemp = ktemp->next;
        }
        while (ktemp != NULL) {
          ktemp->p = l;
          ktemp = ktemp->next;
        }
        if (lchild != NULL) {
          while (lchild->next != NULL)
          {
            lchild = lchild->next;
          }
          lchild->next = rchild;
          lchild = lchild->next;
          while (lchild != NULL)
          {
            lchild->parent = l;
            lchild = lchild->next;
          }
        }
      }
      rtp->size--;
      delete r;
      r = NULL;
      rt = rtp;
    }
  }
}
#endif //_BTREE_H
