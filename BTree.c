#include <stdio.h>
#include <stdlib.h>
#include "BTree.h"

struct BTNode_t
{
  BTNode *parent;
  BTNode *left;
  BTNode *right;
  void *data;
};

struct BTree_t
{
  BTNode *root;
  int size;
};

static void terminate(char *m);
static void FreeNodesRec(BTNode *n);
static BTNode *createNode(void *data);

static void terminate(char *m)
{
  printf("%s\n", m);
  exit(EXIT_FAILURE);
}

BTree *btCreate(void)
{
  BTree *tree = malloc(sizeof(BTree));
  if (!tree)
    terminate("btCreate: tree can not be created");
  tree->root = NULL;
  tree->size = 0;
  return tree;
}

static void FreeNodesRec(BTNode *n)
{
  if (!n)
    return;
  FreeNodesRec(n->left);
  FreeNodesRec(n->right);
  free(n);
}

void btFree(BTree *tree)
{
  FreeNodesRec(tree->root);
  free(tree);
}

static BTNode *createNode(void *data)
{
  BTNode *n = malloc(sizeof(BTNode));
  if (!n)
    terminate("createNode: Node can not be created");
  n->data = data;
  n->left = NULL;
  n->right = NULL;
  n->parent = NULL;
  return n;
}

BTNode *btCreateRoot(BTree *tree, void *data)
{
  BTNode *root = createNode(data);
  tree->root = root;
  tree->size = 1;
  return root;
}

BTNode *btInsertLeft(BTree *tree, BTNode *n, void *data)
{
  BTNode *nleft = createNode(data);
  n->left = nleft;
  nleft->parent = n;
  tree->size++;
  return nleft;
}

BTNode *btInsertRight(BTree *tree, BTNode *n, void *data)
{
  BTNode *nright = createNode(data);
  n->right = nright;
  nright->parent = n;
  tree->size++;
  return nright;
}

BTNode *btRoot(BTree *tree)
{
  return tree->root;
}

BTNode *btLeft(BTree *tree, BTNode *n)
{
  (void)(tree);
  return n->left;
}

BTNode *btRight(BTree *tree, BTNode *n)
{
  (void)(tree);
  return n->right;
}

BTNode *btParent(BTree *tree, BTNode *n)
{
  (void)(tree);
  return n->parent;
}

void *btGetData(BTree *tree, BTNode *n)
{
  (void)(tree);
  return n->data;
}

int btSize(BTree *tree)
{
  return tree->size;
}

int btIsRoot(BTree *tree, BTNode *n)
{
  (void)(tree);
  return (n->parent == NULL);
}

int btIsInternal(BTree *tree, BTNode *n)
{
  (void)(tree);
  return (n->left != NULL || n->right != NULL);
}

int btIsExternal(BTree *tree, BTNode *n)
{
  (void)(tree);
  return (n->left == NULL && n->right == NULL);
}

int btHasLeft(BTree *tree, BTNode *n)
{
  (void)(tree);
  return (n->left != NULL);
}

int btHasRight(BTree *tree, BTNode *n)
{
  (void)(tree);
  return (n->right != NULL);
}

// Les fonctions ci-dessous sont a completer

void btMapLeaves(BTree *tree, BTNode *n, void (*f)(void *data, void *fparams), void *fparams)
{
  if (!n) // au cas ou on appelle mapleaves la ou il n'y a pas de noeud
    return;

  if (btIsExternal(tree, n)) // si on est sur une feuille
  {
    void *data = btGetData(tree, n);
    f(data, fparams); // on applique la fonction f sur la feuille(ses data)
  }

  else // on appelle recursivement sur les enfants
  {
    if (btHasLeft(tree, n))
      btMapLeaves(tree, btLeft(tree, n), f, fparams);

    if (btHasRight(tree, n))
      btMapLeaves(tree, btRight(tree, n), f, fparams);
  }
}

void btMergeTrees(BTree *lefttree, BTree *righttree, void *data)
{
}
