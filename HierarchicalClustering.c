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

/// Fonctionns statiques d'aide pour hclustClustersDist ///

static void collectLeavesRec(BTree *tree, BTNode *n, List *out) // fct recursive pour faire une liste des feuilles d'un sous-arbre/cluster(methode plus facile qu'avec mapleaves)
{
    if (n == NULL)
        return;

    if (btIsExternal(tree, n)) // si on est sur une feuille
    {
        llInsertLast(out, btGetData(tree, n)); // on ajoute la data de la feuille à la liste
        return;
    }

    collectLeavesRec(tree, btLeft(tree, n), n);  // on descend dans le sous-arbre gauche
    collectLeavesRec(tree, btRight(tree, n), n); // on descend dans le sous-arbre droit
}

static void clustersDistRec(BTree *tree, BTNode *n, BTNode *parent, double T, List *clusters) // T pour threshold(plus court)
{
    if (n == NULL)
        return;

    if (btIsInternal(tree, n)) // verifie que c'est un noeud interne
    {
        double dn = *(double *)btGetData(tree, n); // distance du noeud courant(on cast le retour de GetData en double)

        int parentAbove = 1; // boolean pour savoir si le parent est au dessus du seuil T(1 = true, 0 = false)
        if (parent != NULL)  // sinon tout l'arbre est en dessous du seuil
        {
            double dp = *(double *)btGetData(tree, parent); // distance du parent
            parentAbove = (dp > T);                         // on verifie si le parent est au dessus du seuil
        }

        // creation d'un cluster si conditions remplies///

        if (dn <= T && parentAbove) // verifie que distance avec noeud en dessous < T < distance avec parent au dessus(conditions pour former un cluster)
        {
            List *new = llCreateEmpty();    // on cree une "new"list pour stocker les feuilles du cluster
            collectLeavesRec(tree, n, new); // on collecte les feuilles du cluster
            llInsertLast(clusters, new);    // on ajoute le "new"cluster a la liste des clusters
            return;
        }
    }

    // sinon on cherche plus bas

    clustersDistRec(tree, btLeft(tree, n), n, T, clusters);
    clustersDistRec(tree, btRight(tree, n), n, T, clusters);
}

List *hclustGetClustersDist(Hclust *hc, double distanceThreshold)
{
    List *clusters = llCreateEmpty();   // liste de clusters a retourner
    if (hc == NULL || hc->tree == NULL) // si jamais struct hc ou arbre dedans NULL
        return clusters;

    BTNode *root = btRoot(hc->tree);                                    // on récupère la racine
    clustersDistRec(hc->tree, root, NULL, distanceThreshold, clusters); // on appelle la fct recursive avec la racine
    return clusters;
}