#include <stdio.h>
#include <stdlib.h>
#include "myProto.h"

double calculateAbs(double num) {
    if(num < 0) {
        return num * -1;
    }
    return num;
}

int*** readPicturesFromFile(FILE* fp, double* matchingValue, int* numOfPics, int** picIds, int** picsN) {
    int n;
    int*** pictures;

    // read matching value
    fscanf(fp, "%lf\n", matchingValue);

     // read number of pictures
    fscanf(fp, "%d\n", numOfPics);

    // memory allocation
    *picsN = malloc(*numOfPics * sizeof(int));
    *picIds = malloc(*numOfPics * sizeof(int));

    pictures = (int***)malloc(*numOfPics * sizeof(int**));

    for(int i = 0; i < *numOfPics; i++) {
        // read pic id
        fscanf(fp, "%d\n", &(*picIds)[i]);
        // read pic n
        fscanf(fp, "%d\n", &(*picsN)[i]);
        // memory allocation
        pictures[i] = (int**) malloc((*picsN)[i] * sizeof(int*));
        for(int j = 0; j < (*picsN)[i]; j++) {
            // memory allocation
            pictures[i][j] = (int*) malloc((*picsN)[i] * sizeof(int));
            for(int k = 0; k < (*picsN)[i]; k++) {
                // read picture value
                fscanf(fp, "%d [^\n]", &pictures[i][j][k]);
            }
        }
    }

    return pictures;
}

int*** readObjectsFromFile(FILE* fp, int* numOfObjs, int** objIds, int** objsN) {
    int n;
    int*** objects;

     // read number of objects
    fscanf(fp, "%d\n", numOfObjs);

    // memory allocation
    *objsN = malloc(*numOfObjs * sizeof(int));
    *objIds = malloc(*numOfObjs * sizeof(int));

    objects = (int***)malloc(*numOfObjs * sizeof(int**));

    for(int i = 0; i < *numOfObjs; i++) {
        // read obj id
        fscanf(fp, "%d\n", &(*objIds)[i]);
        // read obj n
        fscanf(fp, "%d\n", &(*objsN)[i]);
        // memory allocation
        objects[i] = (int**) malloc((*objsN)[i] * sizeof(int*));
        for(int j = 0; j < (*objsN)[i]; j++) {
            // memory allocation
            objects[i][j] = (int*) malloc((*objsN)[i] * sizeof(int));
            for(int k = 0; k < (*objsN)[i]; k++) {
                // read object value
                fscanf(fp, "%d [^\n]", &objects[i][j][k]);
            }
        }
    }
    return objects;
}

double calculateMatchingValue(int** picture, int** object, int oSize, int i, int j, double matchingValue) {
    double sum = 0;
    for(int oRow = 0 ; oRow < oSize; oRow++) {
        for(int oCol = 0; oCol < oSize; oCol++) {
            double p = (double)(picture[i + oRow][j + oCol]);
            double o = (double)(object[oRow][oCol]);
            sum = (double)calculateAbs((p-o)/p); 
            if(sum >= matchingValue)
                break;
        }
    }
    return sum;
}
