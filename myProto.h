#pragma once

#include "cFunctions.c"

#define FILE_INPUT "input.txt"
#define FILE_OUTPUT "output.txt"

int*** readPicturesFromFile(FILE* fp, double* matchingValue, int* numOfPics, int** picIds, int** picsN);
int*** readObjectsFromFile(FILE* fp, int* numOfObjs, int** objIds, int** objsN);
double calculateMatchingValue(int** picture, int** object, int oSize, int i, int j, double matchingValue);
double calculateAbs(double num);