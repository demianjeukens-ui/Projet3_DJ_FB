#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h> //Pour log = ln
#include "HierarchicalClustering.h"
#include "Phylogenetic.h"
#include "Dict.h"
#include "LinkedList.h"

typedef struct
{
    Dict *dna_sequences; // Dictionnaire à qui on donne le nom d'une espece et nous renvoie sa séquence d'ADN
} PhyloDistParams;

// Fonction d'aide pour la distance ADN
static int isPurine(char base)
{
    return (base == 'A' || base == 'G');
}

static int isPyrimidine(char base)
{
    return (base == 'C' || base == 'T');
}

double phyloDNADist(char *dna1, char *dna2)
{
    size_t len1 = strlen(dna1);
    size_t len2 = strlen(dna2);
    size_t n = (len1 < len2) ? len1 : len2;

    if (n == 0)
    {
        return 0.0;
    }

    int count_Q = 0; // Transition (Q)
    int count_P = 0; // Transversion (P)

    for (size_t i = 0; i < n; i++)
    {
        char base1 = dna1[i];
        char base2 = dna2[i];

        if (base1 == base2)
        {
            continue;
        }

        if ((isPurine(base1) && isPurine(base2)) || (isPyrimidine(base1) && isPyrimidine(base2)))
        {
            count_Q++;
        }

        else
        {
            count_P++;
        }
    }

    double Q = (double)count_Q / n;
    double P = (double)count_P / n;

    double arg1 = 1.0 - 2.0 * P - Q;
    double arg2 = 1.0 - 2.0 * Q;

    if (arg1 <= 0.0 || arg2 <= 0.0)
    {
        return 1000.0; // Valeur arbitraire pour dire que la distance est trop grande
    }

    double dist = -0.5 * log(arg1) - 0.25 * log(arg2); // Formule de l'énoncé

    return dist;
}
