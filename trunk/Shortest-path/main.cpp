/* 
 * File:   main.cpp
 * Author: kafe
 *
 * Created on 26. září 2010, 23:31
 */

#include <cstdlib>
#include <iostream>
#include <math.h>
#include <iostream>
#include <fstream>
#include <ctime>

using namespace std;

//Basic constants
#define     STACKSTEP       20

//Basic variables
int * nodes;
double * distances;
int * localStack;
int nodesCount;
int stackPointer;
int stackSize;
int stackItemSize;

/*
 * Stack push
 */
int push(int * data) {
    for (int x = 0; x < stackItemSize; x++) {
        localStack[stackPointer++] = data[x];
    }
    if (stackPointer == stackSize) {
        int newStackSize = stackSize + (stackItemSize * STACKSTEP);
        int * newLocalStack = new int[newStackSize];
        for (int x = 0; x < stackSize; x++) {
            newLocalStack[x] = localStack[x];
        }
        stackSize = newStackSize;
        delete [] localStack;
        localStack = newLocalStack;
    }
}

/*
 * Stack pop
 */
int pop(int * data) {
    stackPointer -= stackItemSize;
    for (int x = 0; x < stackItemSize; x++) {
        data[x] = localStack[stackPointer++];
    }
    stackPointer -= stackItemSize;
}

/**
 * Get distance
 */
double getDistance(int nodeA, int nodeB) {
    double distance = 0;
    distance += pow((nodes[(nodeA * 2)] - nodes[(nodeB * 2)]), 2);
    distance += pow((nodes[(nodeA * 2) + 1] - nodes[(nodeB * 2) + 1]), 2);
    distance = sqrt(distance);
    return distance;
}

/**
 * Pregenerates all distances
 */
bool generateDistances() {
    int distancesCount = pow(nodesCount, 2);
    distances = new double[distancesCount];
    for (int x = 0; x < nodesCount; x++) {
        for (int y = 0; y < nodesCount; y++) {
            distances[x + y * nodesCount] = getDistance(x, y);
        }
    }
}

/**
 * Get distance of whole row
 */
double itemDistance(int * data) {
    double distance = 0;
    for (int x = 1; x < stackItemSize - 1; x++) {
        if (data[x] != 0 && data[x + 1] != 0) distance += distances[(data[x] - 1) + ((data[x + 1] - 1) * nodesCount)];
    }
    return distance;
}

/*
 * Shortest path finder
 */
int main(int argc, char** argv) {
    //Welcome text
    cout << "Shortest Hamilton's path. Waiting for input. :)" << endl;

    //Define basic variables
    char input[32];
    int numA, numB, statesTried = 0;
    time_t seconds;

    //Read count of nodes
    cin.getline(input, sizeof (input));
    nodesCount = atoi(input);
    //Check count
    if (nodesCount < 2) {
        cerr << "At least 2 nodes are required." << endl;
        return 1;
    }
    cout << nodesCount << " nodes will be loaded." << endl;

    //We know count, let's create an array with nodes
    nodes = new int[nodesCount * 2];
    for (int x = 0; x < nodesCount; x++) {
        cin.getline(input, sizeof (input), ' ');
        numA = atoi(input);
        cin.getline(input, sizeof (input), '\n');
        numB = atoi(input);

        nodes[(x * 2)] = numA;
        nodes[(x * 2) + 1] = numB;
        cout << "Node on " << numA << "x" << numB << " loaded." << endl;
    }
    cout << "Nodes loaded, searching shortest way." << endl;

    generateDistances();

    cout << "Distances pregenerated, running paralel algorithm." << endl;
    seconds = time(NULL);

    //Create basic stack
    stackItemSize = nodesCount + 1; //size of one stack item
    stackSize = stackItemSize * STACKSTEP;
    localStack = new int[stackSize]; //for first create only basic size of stack
    stackPointer = 0; //set pointer to first item
    //Insert first null elements
    int data[stackItemSize];
    int dataTemp[stackItemSize];
    for (int x = 0; x < stackItemSize; x++) {
        data[x] = 0;
    }
    push(data);

    //prepare variables for searching
    double minimalDistace = -1;
    int dataFinal[nodesCount];

    //Run while stack is not empty
    while (true) {
        //Get first item on stack
        pop(data);
        //Check if we have generated all nodes
        if (data[0] != nodesCount) {
            //We haven't, so generate more of them
            //First backup stack item, from which we're generating new data
            for (int x = 0; x < stackItemSize; x++) {
                dataTemp[x] = data[x];
            }
            //Now generate new series
            int y = 1;
            data[0] = dataTemp[0] + 1;
            for (int x = 0; x < (nodesCount - dataTemp[0]); x++) {
                //Search for empty space for new number
                while (data[y] != 0) {
                    y++;
                }
                data[y] = data[0];
                if (minimalDistace > itemDistance(data) || minimalDistace == -1 || stackPointer == 0) {
                    push(data); //Insert
                }
                //Reload old value
                data[y] = dataTemp[y++];
            }
        } else {
            //We have final part, now print distance
            double distance = itemDistance(data);
            statesTried++;
            if (minimalDistace > distance || minimalDistace == -1) {
                minimalDistace = distance;
                cout << "New minimal distance found: " << minimalDistace << endl;
                for (int x = 0; x < stackItemSize; x++) {
                    dataFinal[x] = data[x + 1] - 1;
                }
            }

            if (stackPointer == 0) {
                cout << "--------------------------------" << endl;
                cout << "Minimal way:" << endl;
                for (int x = 0; x < nodesCount; x++) {
                    cout << nodes[dataFinal[x]*2] << " " << nodes[(dataFinal[x]*2) + 1] << endl;
                }
                cout << "Distance: " << minimalDistace << endl;
                cout << "States searched: " << statesTried << endl;
                cout << "Seconds to search: " << time(NULL) - seconds << endl;
                cout << "--------------------------------" << endl;
                cout << "Exporting to file plot" << endl;

                ofstream exportFile;
                exportFile.open("plot");
                for (int x = 0; x < nodesCount; x++) {
                    exportFile << nodes[dataFinal[x]*2] << " " << nodes[(dataFinal[x]*2) + 1] << endl;
                }
                exportFile.close();
                cout << "--------------------------------" << endl;
                cout << "--------------DONE--------------" << endl;
                break;
            }


        }
    }


    return 0;
}
