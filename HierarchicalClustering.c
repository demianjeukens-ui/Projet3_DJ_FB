#include <stdlib.h>
#include <string.h> // Pour strdup
#include <stdio.h>
#include "HierarchicalClustering.h"
#include "BTree.h"
#include "List.h"
#include "Dict.h"

struct Hclust_t
{
    BTree *finaltree;
};

// Strucutre servant au stockage
typedef struct
{
    const char *o1; // Nom objet 1
    const char *o2; // Nom objet 2
    double dist;    // Distance entre les 2 objets
} Pair_t;

// Structure permettant de passer les nouveaux paramètres(Dict et nouveau cluster) en arguments de btMapLeaves
typedef struct
{
    Dict *dict;
    BTree *new_cluster;
} New_params;

// Compare les distances des pairs A et B
static int compare_pairs(const void *a, const void *b)
{

    const Pair_t **Pair_A = (const Pair_t **)a;
    const Pair_t **Pair_B = (const Pair_t **)b;

    double dist_A = (*Pair_A)->dist;
    double dist_B = (*Pair_B)->dist;

    if (dist_A < dist_B)
        return -1;

    if (dist_A > dist_B)
        return 1;

    return 0;
}

// Fonction appelée par btMapLeaves pour mettre à jour le dictionnaire
static void update_dict(void *data, void *fparams)
{
    const char *object_name = (const char *)data;
    New_params *params = (New_params *)fparams;

    // Ecrase l'ancien nom de cluster par le nouveau
    dictInsert(params->dict, object_name, params->new_cluster);
}

Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams)
{
    int N = listSize(objects);
    if (N == 0)
        return NULL;

    Hclust *hc = malloc(sizeof(Hclust));
    if (!hc)
        exit(EXIT_FAILURE);
    hc->finaltree = NULL;
}

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