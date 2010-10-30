/*** INCLUDES ***/
#include "mpi.h"
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <iostream>
#include <fstream>
#include <ctime>

using namespace std;

/*** BASIC CONSTANTS ***/
#define     STACKSTEP           100
#define     CHECK_MIN_INT       50
#define     CHECK_MAX_INT       300
#define     CHECK_STEP          50

#define     MSG_TOKEN_WHITE     2001
#define     MSG_TOKEN_BLACK     2002
#define     MSG_WORK_REQUEST    1000
#define     MSG_WORK_SENT       1001
#define     MSG_WORK_NOWORK     1002
#define     MSG_WORK_ASKLATER   1003
#define     MSG_NEW_MINIMAL     1004
#define     MSG_MINIMAL_DATA    1005

#define     MSG_FINISH          2000
#define     MINIMAL_DEPTH       4

#define     STATUS_WORKING         1
#define     STATUS_NODATA          2
#define     STATUS_ANSWERPENDING   3

/*** GLOBAL VARIABLES ***/
int * nodes;
double * distances;
int * localStack;
int nodesCount;
int stackPointer;
int stackBottom;
int stackSize;
int stackItemSize;
int status;

/******************************
 * Stack push
 ******************************/
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

/******************************
 * Stack pop
 ******************************/
int pop(int * data) {
    stackPointer -= stackItemSize;
    for (int x = 0; x < stackItemSize; x++) {
        data[x] = localStack[stackPointer++];
    }
    stackPointer -= stackItemSize;
}

/******************************
 * Stack pop from bottom
 ******************************/
int popBottom(int * data) {
    for (int x = 0; x < stackItemSize; x++) {
        data[x] = localStack[stackBottom++];
    }
}

/******************************
 * Stack top
 ******************************/
int top(int * data) {
    stackPointer -= stackItemSize;
    for (int x = 0; x < stackItemSize; x++) {
        data[x] = localStack[stackPointer++];
    }
}

/******************************
 * Compute distance
 ******************************/
double getDistance(int nodeA, int nodeB) {
    double distance = 0;
    distance += pow((nodes[(nodeA * 2)] - nodes[(nodeB * 2)]), 2);
    distance += pow((nodes[(nodeA * 2) + 1] - nodes[(nodeB * 2) + 1]), 2);
    distance = sqrt(distance);
    return distance;
}

/******************************
 * Pregenerates all distances
 ******************************/
bool generateDistances() {
    int distancesCount = pow(nodesCount, 2);
    distances = new double[distancesCount];
    for (int x = 0; x < nodesCount; x++) {
        for (int y = 0; y < nodesCount; y++) {
            distances[x + y * nodesCount] = getDistance(x, y);
        }
    }
}

/******************************
 * Get distance of whole row
 ******************************/
double itemDistance(int * data) {
    double distance = 0;
    int x = 1, y;
    while (x < (stackItemSize-1)) {
        while (data[x] == 0) {
            x++;
        }
        y = x+1;
        while (data[y] == 0) {
            y++;
        }
        if (y < stackItemSize) distance += distances[(data[x] - 1) + ((data[y] - 1) * nodesCount)];
        x++;
    }
    return distance;
}

/******************************
 * Get last node on the left side od graph
 ******************************/
int getMostLeftNode() {
    int tempX = nodes[0];
    int node = 0;
    for (int x = 1; x < nodesCount; x++) {
        if (tempX > nodes[x * 2]) {
            tempX = nodes[x * 2];
            node = x;
        }
    }
    return node;
}

/******************************
 * Shortest path finder
 ******************************/
int main(int argc, char** argv) {
    //Define basic variables
    char input[32];
    int numA, numB;

    /*** MPI PART ***/
    MPI_Status msgStatus;
    int myProcessId, processCount;
    /* start up MPI */
    MPI_Init(&argc, &argv);

    /* find out process rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &myProcessId);

    /* find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &processCount);

    /*** END OF MPI PART ***/

    cout << myProcessId << ": Shortest Hamilton's path. Reading from file." << endl;

    //Open file
    ifstream importFile("nodes");
    if (!importFile.is_open()) {
        cerr << "Cannot open file." << endl;
        return 1;
    }
    //Read count of nodes
    importFile.getline(input, sizeof (input));
    nodesCount = atoi(input);
    //Check count
    if (nodesCount < 2) {
        cerr << "At least 2 nodes are required." << endl;
        return 1;
    }
    cout << myProcessId << ": " << nodesCount << " nodes will be loaded." << endl;

    //We know count, let's create an array with nodes
    nodes = new int[nodesCount * 2];
    for (int x = 0; x < nodesCount; x++) {
        importFile.getline(input, sizeof (input), ' ');
        numA = atoi(input);
        importFile.getline(input, sizeof (input), '\n');
        numB = atoi(input);

        nodes[(x * 2)] = numA;
        nodes[(x * 2) + 1] = numB;
        cout << myProcessId << ": Node on " << numA << "x" << numB << " loaded." << endl;
    }
    cout << myProcessId << ": Nodes loaded, searching shortest way." << endl;

    //Close file
    importFile.close();

    //Start timer
    time_t seconds = time(NULL);

    //Pregenerate distances
    generateDistances();

    cout << myProcessId << ": Distances pregenerated, searching firts minimal way." << endl;

    //TODO

    cout << myProcessId << ": Search completed, running paralel algorithm." << endl;

    //Create basic stack
    stackItemSize = nodesCount + 1; //size of one stack item
    stackSize = stackItemSize * STACKSTEP;
    localStack = new int[stackSize]; //for first create only basic size of stack
    stackPointer = 0; //set pointer to first item
    stackBottom = 0; //set bottom of stack to null
    int data[stackItemSize];
    int dataTemp[stackItemSize];
    int dataFinal[stackItemSize];
    int * dataPointer;

    //Insert first null elements, if this is root process
    if (myProcessId == 0) {
        for (int x = 0; x < stackItemSize; x++) {
            data[x] = 0;
            dataTemp[x] = 0;
            dataFinal[x] = 0;
        }
        push(data); //Push firts items to stack
    }

    //prepare variables for searching
    double minimalDistace = -1;
    double lastMinimalDistance = -1;
    int checkMessages = 0; //counter if is time to check arrived messages
    int sendMinimalMessages = 1; //counter if is time to send a minimal way update
    int checkMessagesEvery = CHECK_MAX_INT; //default interval to check messages and minimal way update

    //prepare variables for network communication
    int randomProcess;
    int minimalDistanceSource = -1;

    //Init random seed
    srand(time(NULL));

    //Run while stack is not empty
    status = STATUS_WORKING;
    bool firstRun = true;
    while (true) {

        if (processCount > 1) {
            //Check stack
            if (stackPointer == stackBottom && status != STATUS_ANSWERPENDING) {
                //Set status to nodata
                status = STATUS_NODATA;
                //Generate random node to ask for work
                randomProcess = rand() % processCount;
                //If this is first run, ask root for data first
                if (firstRun && myProcessId != 0) {
                    randomProcess = -1;
                    firstRun = false;
                }
            }



            /******************************
             * ACTIVE PART
             * Asking for data, sending minimal update...
             ******************************/
            //Check if data is needed
            if (status == STATUS_NODATA) {
                randomProcess = (++randomProcess % processCount); //add one to x and try next node
                if (randomProcess != myProcessId) {
                    cout << myProcessId << ": Sending request to process " << randomProcess << endl;
                    MPI_Send(&data, stackItemSize * sizeof (data), MPI_CHAR, randomProcess, MSG_WORK_REQUEST, MPI_COMM_WORLD);

                    //If this is root node, check if any node is in working state
                    if (myProcessId == 0) {
                        cout << myProcessId << ": No work - sending white token." << endl;
                        MPI_Send(&data, stackItemSize * sizeof (data), MPI_CHAR, ((myProcessId + 1) % processCount), MSG_TOKEN_WHITE, MPI_COMM_WORLD);
                    }

                    status = STATUS_ANSWERPENDING;
                }
            }

            //Check if is time to send update
            if ((sendMinimalMessages++ % (checkMessagesEvery * 5)) == 0 && (minimalDistace != lastMinimalDistance)) {
                cout << myProcessId << ": Sending minimal distance update: " << minimalDistace << endl;
                for (int x = 0; x < sizeof (minimalDistace); x++) {
                    data[x] = ((char*) &minimalDistace)[x];
                }
                for (int x = 0; x < processCount; x++) {
                    if (x != myProcessId) {
                        MPI_Send(data, stackItemSize * sizeof (data), MPI_CHAR, x, MSG_NEW_MINIMAL, MPI_COMM_WORLD);
                    }
                }
                lastMinimalDistance = minimalDistace;
                sendMinimalMessages = 1;
            }

            /******************************
             * PASSIVE PART
             * Receive answers, updates...
             ******************************/
            //Check if any message is waiting (if is time)
            if ((checkMessages++ % checkMessagesEvery) == 0) {
                int flag;
                MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &msgStatus);
                if (flag) {
                    //New message is here, process it
                    double tempMinimalDistace; //In case new minimal distance arrived
                    MPI_Recv(&data, stackItemSize * sizeof (data), MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &msgStatus);
                    switch (msgStatus.MPI_TAG) {

                            //Send some data to work on
                        case MSG_WORK_REQUEST:
                            if (stackPointer == stackBottom) {
                                cout << myProcessId << ": Data request from " << msgStatus.MPI_SOURCE << " received - but no data to give. Time: " << checkMessagesEvery << endl;
                                MPI_Send(data, stackItemSize * sizeof (data), MPI_CHAR, msgStatus.MPI_SOURCE, MSG_WORK_NOWORK, MPI_COMM_WORLD);
                            } else {
                                cout << myProcessId << ": Data request from " << msgStatus.MPI_SOURCE << " received. Time: " << checkMessagesEvery << endl;
                                popBottom(data);
                                MPI_Send(data, stackItemSize * sizeof (data), MPI_CHAR, msgStatus.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD);
                            }
                            break;

                            //Work is here
                        case MSG_WORK_SENT:
                            cout << myProcessId << ": Work from " << msgStatus.MPI_SOURCE << " accepted." << endl;
                            cout << myProcessId << ": Data ";
                            for (int y = 0; y < stackItemSize; y++) {
                                cout << data[y] << " ";
                            }
                            cout << endl;
                            push(data); //Add new item to stack
                            status = STATUS_WORKING;
                            break;

                            //Work is not here, search again
                        case MSG_WORK_NOWORK:
                            if (status = STATUS_ANSWERPENDING) {
                                status = STATUS_NODATA;
                            }
                            break;

                            //New minimal distance arrived
                        case MSG_NEW_MINIMAL:
                            for (int x = 0; x < sizeof (minimalDistace); x++) {
                                ((char*) &tempMinimalDistace)[x] = data[x];
                            }
                            if (minimalDistace > tempMinimalDistace || minimalDistace == -1) {
                                minimalDistace = tempMinimalDistace;
                                lastMinimalDistance = tempMinimalDistace;
                                minimalDistanceSource = msgStatus.MPI_SOURCE;
                                cout << myProcessId << ": New minimal distance received: " << minimalDistace << " from " << msgStatus.MPI_SOURCE << endl;
                            }
                            break;

                            //White token arrived, chceck if we have data or we are root node
                        case MSG_TOKEN_WHITE:
                            cout << myProcessId << ": White token from " << msgStatus.MPI_SOURCE << " received." << endl;
                            if (myProcessId == 0) { //This is the end my only friend
                                //Terminate other threads
                                for (int x = 0; x < processCount; x++) {
                                    if (x != myProcessId) {
                                        MPI_Send(data, stackItemSize * sizeof (data), MPI_CHAR, x, MSG_FINISH, MPI_COMM_WORLD);
                                    }
                                }
                                //Receive last data from node with comuted minimal way
                                MPI_Recv(&dataTemp, stackItemSize * sizeof (data), MPI_CHAR, minimalDistanceSource, MSG_MINIMAL_DATA, MPI_COMM_WORLD, &msgStatus);
                                cout << myProcessId << ": Last data from " << msgStatus.MPI_SOURCE << " received." << endl;
                                cout << myProcessId << ": Last data ";
                                for (int y = 0; y < stackItemSize; y++) {
                                    cout << dataTemp[y] << " ";
                                }
                                cout << endl;
                                //Check root minimal distance and received minimal distance
                                if (itemDistance(dataTemp) > itemDistance(dataFinal)) {
                                    minimalDistace = itemDistance(dataFinal);
                                    cout << myProcessId << ": Remote distance won: " << minimalDistace << endl;
                                    dataPointer = &dataFinal[0];
                                } else {
                                    minimalDistace = itemDistance(dataTemp);
                                    cout << myProcessId << ": Local distance won: " << minimalDistace << endl;
                                    dataPointer = &dataTemp[0];
                                }
                                cout << myProcessId << ": Computation data ";
                                for (int y = 0; y < stackItemSize; y++) {
                                    cout << *(dataPointer + y) << " ";
                                }
                                cout << endl;
                                cout << myProcessId << ": --------------------------------" << endl;
                                cout << myProcessId << ": Minimal way:" << endl;
                                for (int y = 1; y < stackItemSize; y++) {
                                    cout << nodes[((*(dataPointer + y)) - 1)*2] << " " << nodes[((*(dataPointer + y)) - 1)*2 + 1] << endl;
                                }
                                cout << myProcessId << ": Distance: " << minimalDistace << endl;
                                cout << myProcessId << ": Seconds to search: " << time(NULL) - seconds << endl;
                                cout << myProcessId << ": --------------------------------" << endl;
                                cout << myProcessId << ": Exporting to file plot" << endl;

                                ofstream exportFile;
                                exportFile.open("plot");
                                for (int y = 1; y < stackItemSize; y++) {
                                    exportFile << nodes[((*(dataPointer + y)) - 1)*2] << " " << nodes[((*(dataPointer + y)) - 1)*2 + 1] << endl;
                                }
                                exportFile.close();
                                cout << myProcessId << ": --------------------------------" << endl;
                                cout << myProcessId << ": --------------DONE--------------" << endl;
                                return 0;
                            } else {
                                if (status == STATUS_WORKING) {
                                    MPI_Send(data, stackItemSize * sizeof (data), MPI_CHAR, ((myProcessId + 1) % processCount), MSG_TOKEN_BLACK, MPI_COMM_WORLD);
                                } else {
                                    MPI_Send(data, stackItemSize * sizeof (data), MPI_CHAR, ((myProcessId + 1) % processCount), MSG_TOKEN_WHITE, MPI_COMM_WORLD);
                                }
                            }
                            break;

                            //Black token arrived, just send it around or drop it
                        case MSG_TOKEN_BLACK:
                            cout << myProcessId << ": Black token from " << msgStatus.MPI_SOURCE << " received." << endl;
                            if (myProcessId != 0) {
                                MPI_Send(data, stackItemSize * sizeof (data), MPI_CHAR, ((myProcessId + 1) % processCount), MSG_TOKEN_BLACK, MPI_COMM_WORLD);
                            }
                            break;

                            //Final message, we're done
                        case MSG_FINISH:
                            cout << myProcessId << ": Termination from " << msgStatus.MPI_SOURCE << " received - hurray!." << endl;
                            MPI_Send(dataFinal, stackItemSize * sizeof (data), MPI_CHAR, 0, MSG_MINIMAL_DATA, MPI_COMM_WORLD);
                            cout << myProcessId << ": Last message sent to root, good bye blue sky." << endl;
                            return 0;
                            break;
                        default:
                            cerr << "Communication error: " << msgStatus.MPI_TAG << endl;
                            return 2;
                            break;
                    }
                }
                checkMessages = 1;
            }

        } else {
            if (stackPointer == stackBottom) {
                minimalDistace = itemDistance(dataFinal);
                cout << myProcessId << ": Local distance won: " << minimalDistace << endl;
                dataPointer = &dataFinal[0];
                cout << myProcessId << ": --------------------------------" << endl;
                cout << myProcessId << ": Minimal way:" << endl;
                for (int y = 1; y < stackItemSize; y++) {
                    cout << nodes[((*(dataPointer + y)) - 1)*2] << " " << nodes[((*(dataPointer + y)) - 1)*2 + 1] << endl;
                }
                cout << myProcessId << ": Distance: " << minimalDistace << endl;
                cout << myProcessId << ": Seconds to search: " << time(NULL) - seconds << endl;
                cout << myProcessId << ": --------------------------------" << endl;
                cout << myProcessId << ": Exporting to file plot" << endl;

                ofstream exportFile;
                exportFile.open("plot");
                for (int y = 1; y < stackItemSize; y++) {
                    exportFile << nodes[((*(dataPointer + y)) - 1)*2] << " " << nodes[((*(dataPointer + y)) - 1)*2 + 1] << endl;
                }
                exportFile.close();
                cout << myProcessId << ": --------------------------------" << endl;
                cout << myProcessId << ": --------------DONE--------------" << endl;
                return 0;
            }
        }


        /******************************
         * ALGORITHM PART
         * Pop stack and compute...
         ******************************/
        if (status == STATUS_WORKING) {
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
                    if (minimalDistace > itemDistance(data) || minimalDistace == -1) {
                        push(data); //Insert
                    }
                    //Reload old value
                    data[y] = dataTemp[y++];
                }
            } else {
                //We have final part, now print distance
                double distance = itemDistance(data);
                if (minimalDistace > distance || minimalDistace == -1) {
                    minimalDistace = distance;
                    cout << myProcessId << ": New minimal distance found: " << minimalDistace << endl;
                    for (int x = 0; x < stackItemSize; x++) {
                        dataFinal[x] = data[x];
                    }
                }
            }
        }
    }

    return 0;
}
