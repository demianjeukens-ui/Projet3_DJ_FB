#include <stdlib.h>
#include "HierarchicalClustering.h"

struct Hclust_t
{
    BTree *tree;
};

/// Partie ajoutée DEM ///

static void freeNodeDataRec(BTree *tree, BTNode *n) // fct recursive pour parcourir le clust et libérer la data de chaque noeud
{
    if (n == NULL)
        return;

    // on libere la data du noeud
    void *data = btGetData(tree, n);
    if (data != NULL)
        free(data);

    // on descend dans le hc récursivement
    if (btHasLeft(tree, n))
        freeNodeDataRec(tree, btLeft(tree, n));

    if (btHasRight(tree, n))
        freeNodeDataRec(tree, btRight(tree, n));
}

void hclustFree(Hclust *hc)
{
    if (hc == NULL)
        return;

    if (hc->tree != NULL)
    {
        BTNode *root = btRoot(hc->tree); // on récupère la racine pour donner a freeNodeDataRec
        freeNodeDataRec(hc->tree, root);
        btFree(hc->tree); // on libère l'arbre apres toute les datas pour éviter les fuites mémoires sur data
    }

    free(hc);
}

static int nbLeavesRec(BTree *tree, BTNode *n) // fct recursive pour compter le nb de feuilles
{
    if (n == NULL)
        return 0;

    if (btIsExternal(tree, n)) // si on est sur une feuille
        return 1;

    return nbLeavesRec(tree, btLeft(tree, n)) + nbLeavesRec(tree, btRight(tree, n)); // somme des feuilles des sous-arbres
}

int hclustNBLeaves(Hclust *hc)
{
    if (hc == NULL || hc->tree == NULL) // si jamais aucun hc ou arbre
        return 0;

    BTNode *root = btRoot(hc->tree);    // on récupère la racine
    return nbLeavesRec(hc->tree, root); // on appelle la fct recursive avec la racine
}