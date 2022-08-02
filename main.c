#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include "myProto.h"

int* findMatchingValue(int** picture, int** object,int pSize, int oSize, double matchingValue);

int main(int argc, char* argv[]) {
    double matchingValue, start_time;
    int *picIds, *picsN, numOfPics, ***pictures; // pictures details
    int *objIds , *objsN, numOfObjs, ***objects; // objects details
    int* result; // variable to store result of matching value search
    int size, rank;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    start_time = MPI_Wtime();
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (size != 2) {
       printf("Run with two processes only\n");
       MPI_Abort(MPI_COMM_WORLD, __LINE__);
    }
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // process 0 read the file details and store it in variables
    if(rank == 0) {
        // open file for reading
        FILE* fp = fopen(FILE_INPUT, "r");
        if(fp == NULL) {
            printf("Cannot open file %s\n", FILE_INPUT);
            exit(0);
        }
         // initiliazation
        pictures = readPicturesFromFile(fp,&matchingValue,&numOfPics,&picIds,&picsN);
        objects = readObjectsFromFile(fp,&numOfObjs,&objIds,&objsN);
        // close reading
        fclose(fp);
    }
    // bcast matching value
    MPI_Bcast(&matchingValue, 1, MPI_DOUBLE, 0 , MPI_COMM_WORLD);
    // bcast number of pictures
    MPI_Bcast(&numOfPics, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // bcast number of objects
    MPI_Bcast(&numOfObjs, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(rank == 0) {
        // send pictures size array
        MPI_Send(picsN, numOfPics, MPI_INT, 1, 0, MPI_COMM_WORLD);
        // send objects size array
        MPI_Send(objsN, numOfObjs, MPI_INT, 1, 0, MPI_COMM_WORLD);
        // send objects id array
        MPI_Send(objIds, numOfObjs, MPI_INT, 1, 0, MPI_COMM_WORLD);
    } else {
        // allocate memory
        picsN = (int*) malloc(numOfPics * sizeof(int));
        objsN = (int*) malloc(numOfObjs * sizeof(int));
        objIds = (int*) malloc(numOfObjs * sizeof(int));
        // receive pictures size array
        MPI_Recv(picsN, numOfPics, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        // receive objects size array
        MPI_Recv(objsN, numOfObjs, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        // receive objects id array
        MPI_Recv(objIds, numOfObjs, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }
    // divide pictures size by 2
    int half = numOfPics/2; 
    if(rank == 0) {
        // send half of the pictures to the second process
        for(int i = half; i < numOfPics; i++)
            for(int j = 0; j < picsN[i]; j++)
                MPI_Send(&pictures[i][j][0],picsN[i],MPI_INT,1,0,MPI_COMM_WORLD);
        // send all of the objects to the second process
        for(int i = 0; i < numOfObjs; i++)
            for(int j = 0; j < objsN[i]; j++)
                MPI_Send(&objects[i][j][0],objsN[i],MPI_INT,1,0,MPI_COMM_WORLD);
    } else {
        // allocate memory for half of the pictures
        pictures = (int***) malloc(half * sizeof(int**));
        for(int i = half; i < numOfPics; i++) {
            pictures[i] = (int**) malloc(picsN[i] * sizeof(int*));
            for(int j = 0; j < picsN[i]; j++)
                pictures[i][j] = (int*) malloc(picsN[i] * sizeof(int));
        }
        // allocate memory for objects
        objects = (int***) malloc(numOfObjs * sizeof(int**));
        for(int i = 0; i < numOfObjs; i++) {
            objects[i] = (int**) malloc(objsN[i] * sizeof(int*));
            for(int j = 0; j < objsN[i]; j++)
                objects[i][j] = (int*) malloc(objsN[i] * sizeof(int));
        }
        // receive half of the pictures from the main process
        for(int i = half; i < numOfPics; i++)
            for(int j = 0; j < picsN[i]; j++)
                MPI_Recv(&pictures[i][j][0], picsN[i], MPI_INT, 0,0,MPI_COMM_WORLD, &status);
        // receive all of the objects from the main process
        for(int i = 0; i < numOfObjs; i++) 
            for(int j = 0; j < objsN[i]; j++) 
                MPI_Recv(&objects[i][j][0], objsN[i], MPI_INT, 0,0,MPI_COMM_WORLD, &status);
    }

    if(rank == 0) {
        // open file for writing
        FILE* fp = fopen(FILE_OUTPUT,"w");
        if(fp == NULL) {
            printf("Failed opening the file.\n");
            exit(0);
        }

        // for each picture find a matching value from the objects and stop when found
        for(int i = 0; i < half; i++) {
            int j;
            for(j = 0; j < numOfObjs; j++) {
                result = findMatchingValue(pictures[i], objects[j],picsN[i],objsN[j],matchingValue);
                if(result[0] != -1 && result[1] != -1)
                    break;
            }
            if(result[0] != -1 && result[1] != -1)
                fprintf(fp,"Picture %d found Object %d in Position (%d,%d)\n",i,j,result[0],result[1]);
            else 
                fprintf(fp,"Picture %d No Objects were found\n", i);
        }

        // receive half of the results from the second process
        for(int i = half; i < numOfPics; i++) {
            // receive object index
            int j;
            MPI_Recv(&j, 1, MPI_INT, 1, 0 ,MPI_COMM_WORLD, &status);
            // receive the result and write to file
            MPI_Recv(result, 2, MPI_INT, 1, 0 ,MPI_COMM_WORLD, &status);

            if(result[0] != -1 && result[1] != -1)
                fprintf(fp,"Picture %d found Object %d in Position (%d,%d)\n",i,j,result[0],result[1]);
            else
                fprintf(fp,"Picture %d No Objects were found\n", i);
        }
        // close writing file
        fclose(fp);
    } else {
        // for each picture find a matching value from the objects and stop when found
        for(int i = half; i < numOfPics; i++) {
            int j;
            for(j = 0; j < numOfObjs; j++) {
                result = findMatchingValue(pictures[i], objects[j],picsN[i],objsN[j],matchingValue);
                if(result[0] != -1 && result[1] != -1)
                    break;
            }
            // send object index for the picture
            MPI_Send(&j, 1 ,MPI_INT, 0 ,0 ,MPI_COMM_WORLD);
            // send result for the picture
            MPI_Send(result, 2 ,MPI_INT, 0 ,0 ,MPI_COMM_WORLD);
        }
    }
    if(rank == 0)
        printf("Total time: %lf.\n",MPI_Wtime() - start_time);
    MPI_Finalize();
   

    return 1;
}

int* findMatchingValue(int** picture, int** object,int pSize, int oSize, double matchingValue) {
    int* result = (int*)malloc(2 * sizeof(int));
    result[0] = -1;
    result[1] = -1;

    if(oSize > pSize) // if object size is bigger than picture size then no matching value can be found
        return 0;

    int i, j, found = 0;
    double sum;
#pragma omp parallel for num_threads(4) collapse(2)
    for(i = 0; i < pSize; i++) {
        for(j = 0 ; j < pSize; j++) {
            if(i + oSize <= pSize && j + oSize <= pSize && found == 0) {
                double sum = calculateMatchingValue(picture,object,oSize,i,j,matchingValue);
                if(sum < matchingValue && found == 0) {
                    result[0] = i;
                    result[1] = j;
                    found = 1;
                }
            }
        }
    }
    return result;
}
