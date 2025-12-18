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

double phyloDNADistance(char *dna1, char *dna2)
{
    size_t len1 = strlen(dna1);
    size_t len2 = strlen(dna2);

    size_t n_min = (len1 < len2) ? len1 : len2;

    int Transitions = 0;   // Transitions
    int Transversions = 0; // Transversions
    int n = 0;

    for (size_t i = 0; i < n_min; i++)
    {
        char base1 = dna1[i];
        char base2 = dna2[i];

        if (base1 < 'A' || base2 < 'A')
        {
            continue;
        }

        n++;

        if (base1 == base2)
        {
            continue;
        }

        if ((isPurine(base1) && isPurine(base2)) || (isPyrimidine(base1) && isPyrimidine(base2)))
        {
            Transitions++;
        }

        else
        {
            Transversions++;
        }
    }

    if (n == 0)
    {
        return 0.0;
    }

    double P = (double)Transitions / n;
    double Q = (double)Transversions / n;

    double arg1 = 1.0 - 2.0 * P - Q;
    double arg2 = 1.0 - 2.0 * Q;

    const double EPSILON = 1e-12;
    if (arg1 <= EPSILON)
    {
        arg1 = EPSILON;
    }
    if (arg2 <= EPSILON)
    {
        arg2 = EPSILON;
    }

    double dist = -0.5 * log(arg1) - 0.25 * log(arg2);

    return dist;
}

/// phyloTreeCreate ///

#define MAXLINE_LENGTH 1024 // eviter les debordemnents de buffer avec fgets
// 1024 borne raisonnable et standard pour la plupart des cas
static double phyloDistFn(const char *obj1, const char *obj2, void *params)
{
    PhyloDistParams *p = (PhyloDistParams *)params;

    char *dna1 = (char *)dictSearch(p->dna_sequences, obj1);
    char *dna2 = (char *)dictSearch(p->dna_sequences, obj2);

    if (dna1 == NULL || dna2 == NULL) // si le fichier est mal formé
    {
        return 1e9; // Valeur arbitraire pour dire que la distance est trop grande
    }

    return phyloDNADistance(dna1, dna2);
}

Hclust *phyloTreeCreate(char *dna_sequences)
{
    char buffer[MAXLINE_LENGTH];
    FILE *file = fopen(dna_sequences, "r");

    if (file == NULL)
    {
        return NULL;
    }

    List *names = llCreateEmpty();
    Dict *DNA_dict = dictCreate(1000);

    while (fgets(buffer, MAXLINE_LENGTH, file))
    {
        buffer[strcspn(buffer, "\r\n")] = '\0';

        if (buffer[0] == '\0')
        {
            continue;
        }

        char *comma = strchr(buffer, ',');
        if (comma == NULL)
        {
            continue; // Ligne mal formée
        }

        *comma = '\0';
        char *name_in = buffer;
        char *dna_in = comma + 1;

        char *name = malloc(strlen(name_in) + 1);
        if (name == NULL)
        {
            fclose(file);
            llFree(names);
            dictFree(DNA_dict);
            return NULL;
        }
        strcpy(name, name_in);

        char *dna = malloc(strlen(dna_in) + 1);
        if (dna == NULL)
        {
            free(name);
            fclose(file);
            llFree(names);
            dictFree(DNA_dict);
            return NULL;
        }
        strcpy(dna, dna_in);

        llInsertLast(names, name);
        dictInsert(DNA_dict, name, dna);
    }

    fclose(file);

    PhyloDistParams params;
    params.dna_sequences = DNA_dict;

    Hclust *hc = hclustBuildTree(names, phyloDistFn, &params);

    dictFreeValues(DNA_dict, free); // Libération des séquences d'ADN
    llFreeData(names);              // Libération des noms

    return hc;
}
