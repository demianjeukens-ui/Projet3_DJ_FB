#include <stdlib.h>
#include <string.h> // Pour strdup
#include <stdio.h>
#include "HierarchicalClustering.h"
#include "BTree.h"
#include "LinkedList.h"
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

/// hclustBuildTree///

static int compare_pairs(const void *a, const void *b) // Compare les distances des pairs A et B
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

static void update_dict(void *data, void *fparams) // Fonction appelée par btMapLeaves pour mettre à jour le dictionnaire
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

/// hclustFree ///

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

    if (hc->finaltree != NULL)
    {
        BTNode *root = btRoot(hc->finaltree); // on récupère la racine pour donner a freeNodeDataRec
        freeNodeDataRec(hc->finaltree, root);
        btFree(hc->finaltree); // on libère l'arbre apres toute les datas pour éviter les fuites mémoires sur data
    }

    free(hc);
}

/// hclustDepth ///

static int depthRec(BTree *tree, BTNode *n) // fct recursive pour calculer la profondeur
{
    if (n == NULL) // si pas de noeud
        return 0;

    if (btIsExternal(tree, n)) // si on est sur une feuille
        return 0;

    int leftDepth = depthRec(tree, btLeft(tree, n));   // profondeur du sous-arbre gauche
    int rightDepth = depthRec(tree, btRight(tree, n)); // profondeur du sous-arbre droit

    return 1 + (leftDepth > rightDepth ? leftDepth : rightDepth); // on retourne la profondeur max + 1(pour le noeud courant)
}

int hclustDepth(Hclust *hc)
{
    if (hc == NULL || hc->finaltree == NULL) // si aucun hc ou hc vide
        return 0;

    BTNode *root = btRoot(hc->finaltree); // on récupère la racine
    return depthRec(hc->finaltree, root); // on appelle la fct recursive avec la racine
}

/// hclustNBLeaves ///

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
    if (hc == NULL || hc->finaltree == NULL) // si jamais aucun hc ou arbre
        return 0;

    BTNode *root = btRoot(hc->finaltree);    // on récupère la racine
    return nbLeavesRec(hc->finaltree, root); // on appelle la fct recursive avec la racine
}

/// hclustPrintTree ///

static void printRec(FILE *out, BTree *tree, BTNode *n, int indent)
{
    if (n == NULL)
        return;

    for (int i = 0; i < indent; i++) // fais des indentations pour la lisibilité
        fprintf(out, "  ");

    if (btIsExternal(tree, n)) // si on est sur une feuille
    {
        fprintf(out, "%s\n", (char *)btGetData(tree, n)); // on imprime le nom de l'objet
    }
    else // noeud interne
    {
        double dist = *(double *)btGetData(tree, n); // on récupère la distance
        fprintf(out, "Distance: %.4f\n", dist);      // on imprime la distance

        printRec(out, tree, btLeft(tree, n), indent + 1);  // on descend dans le sous-arbre gauche
        printRec(out, tree, btRight(tree, n), indent + 1); // on descend dans le sous-arbre droit
    }
}

void hclustPrintTree(FILE *fp, Hclust *hc)
{
    if (hc == NULL || hc->finaltree == NULL) // si jamais aucun hc ou hc vide
        return;

    BTNode *root = btRoot(hc->finaltree); // on récupère la racine
    printRec(fp, hc->finaltree, root, 0); // on appelle la fct recursive avec la racine
}

/// hclustClustersDist ///

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
    List *clusters = llCreateEmpty();        // liste de clusters a retourner
    if (hc == NULL || hc->finaltree == NULL) // si jamais struct hc ou arbre dedans NULL
        return clusters;

    BTNode *root = btRoot(hc->finaltree);                                    // on récupère la racine
    clustersDistRec(hc->finaltree, root, NULL, distanceThreshold, clusters); // on appelle la fct recursive avec la racine
    return clusters;
}

/// hclustGetClustersK ///

static BTNode *maxDistanceNode(BTree *tree, List *nodes) // retourne le noeud interne avec la distance la plus grande pour pouvoir le couper
{
    double maxDist = -1.0; // on initialise a -1 pour etre sur que toute distance positive soit plus grande
    BTNode *best = NULL;   // noeud avec la distance max

    for (Node *p = llHead(nodes); p != NULL; p = llNext(p))
    {
        BTNode *n = (BTNode *)llData(p); // on cast le data retourné par llData en BTNode*
        if (!btIsInternal(tree, n))      // on ne considère que les noeuds internes
            continue;

        double dist = *(double *)btGetData(tree, n); // on récupère la distance du noeud courant
        if (best == NULL || dist > maxDist)          // premier noeud (-> initialise) ou distance plus grande que la max actuelle
        {
            maxDist = dist;
            best = n;
        }
    }
    return best;
}

List *hclustGetClustersK(Hclust *hc, int K)
{
    List *clusters = llCreateEmpty();        // liste de clusters a retourner
    if (hc == NULL || hc->finaltree == NULL) // si jamais struct hc ou arbre dedans NULL
        return clusters;

    BTree *tree = hc->finaltree;
    BTNode *root = btRoot(tree); // on récupère la racine
    if (root == NULL)
        return clusters;

    List *candidates = llCreateEmpty(); // liste des noeuds candidats a être coupés
    llInsertLast(candidates, root);     // on commence avec la racine comme

    int nb = 1; // on a déjà un cluster (la racine)

    while (nb < K)
    {
        BTNode *best = maxDistanceNode(tree, candidates); // on cherche le noeud avec la distance max
        if (best == NULL)                                 // plus de noeud interne a couper
            break;

        List *newCandidates = llCreateEmpty(); // nouvelle liste de candidats après la coupe

        while (llLength(candidates) > 0) // pour chaque noeud dans candidates
        {
            BTNode *cur = (BTNode *)llPopFirst(candidates); // on récupère le premier noeud

            if (cur == best) // si c'est le meilleur noeud a couper
            {
                BTNode *l = btLeft(tree, cur);  // on récupère le fils gauche
                BTNode *r = btRight(tree, cur); // on récupère le fils droit

                if (l != NULL)
                {
                    llInsertLast(newCandidates, l); // on ajoute le fils gauche aux nouveaux candidats
                    nb++;                           // on a un cluster de plus
                }
                if (r != NULL)
                {
                    llInsertLast(newCandidates, r); // on ajoute le fils droit aux nouveaux candidats
                    nb++;                           // on a un cluster de plus
                }
                else // sinon on garde le noeud tel quel
                {
                    llInsertLast(newCandidates, cur);
                }
            }
            llFree(candidates);         // on libère l'ancienne liste de candidats
            candidates = newCandidates; // on remplace par la nouvelle liste
            nb++;                       // on a un cluster de plus
        }

        for (Node *p = llHead(candidates); p != NULL; p = llNext(p))
        {
            BTNode *n = (BTNode *)llData(p); // on cast le data retourné par llData en BTNode*
            List *new = llCreateEmpty();     // on crée une "new"list pour stocker les feuilles du cluster
            collectLeavesRec(tree, n, new);  // on collecte les feuilles du cluster
            llInsertLast(clusters, new);     // on ajoute le "new"cluster a la liste des clusters
        }

        llFree(candidates); // on libère la liste des candidats
        return clusters;
    }
}

/// hclustGettree ///

BTree *hclustGettree(Hclust *hc)
{
    if (hc == NULL)
        return NULL;

    return hc->finaltree;
}