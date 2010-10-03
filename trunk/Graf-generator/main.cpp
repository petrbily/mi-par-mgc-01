/* 
 * File:   main.cpp
 * Author: kafe
 *
 * Created on 23. září 2010, 11:21
 */

#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;

/*
 * Node struct
 */
typedef struct {
    int x;
    int y;
} node_t;

/*
 * Function checking, if node is not already inserted
 */
bool isInserted(int x, int y) {
    return false;
}

node_t generateNode(int maxX, int maxY) {
    //Generate numbers
    int x = rand() % maxX;
    int y = rand() % maxY;
    node_t node;
    node.x = x+1;
    node.y = y+1;
    return node;
}

/*
 * Main function
 */
int main(int argc, char** argv) {

    int maxX, maxY, nodes, round = 0;
    bool falseStart = false;

    //Test arguments
    if (argc == 7 || argc == 9) {
        for (int x = 1; x < (argc - 1); x = x + 2) {
            if (argv[x][0] != '-') {
                falseStart = true;
            }
            switch (argv[x][1]) {
                case 'n':
                    nodes = atoi(argv[x + 1]);
                    break;
                case 'x':
                    maxX = atoi(argv[x + 1]);
                    break;
                case 'y':
                    maxY = atoi(argv[x + 1]);
                    break;
                case 'r':
                    round = atoi(argv[x + 1]);
                    break;
                default:
                    falseStart = true;
                    break;
            }
        }
    } else {
        falseStart = true;
    }

    if (falseStart || nodes < 2 || maxX < 2 || maxY < 2) {
        cerr << "Usage: " << argv[0] << " -n [nodes] -x [maximal horizontal size] -y [maximal vertical size] -r [space around node]" << endl;
        return 1;
    }
    if (maxX * maxY < nodes) {
        cerr << "More nodes than field size." << endl;
        return 2;
    }

    //Init random seed
    srand(time(NULL));

    //We can continue and generate list
    vector<node_t> * nodesVector = new vector<node_t>;
    nodesVector->push_back(generateNode(maxX, maxY));

    for (int x = 1; x < nodes; x++) {
        while (true) {
            node_t newNode = generateNode(maxX, maxY);
            bool free = true;
            for (int y = 0; y < nodesVector->size(); y++) {
                if ((nodesVector->at(y).x <= newNode.x + round && nodesVector->at(y).x >= newNode.x - round) && (nodesVector->at(y).y <= newNode.y + round && nodesVector->at(y).y >= newNode.y - round)) {
                    free = false;
                }
            }
            if (free) {
                nodesVector->push_back(newNode);
                break;
            }
        }
    }

    cout << nodesVector->size() << endl;
    for (int x = 0; x < nodesVector->size(); x++) {
        cout << nodesVector->at(x).x << " " << nodesVector->at(x).y << endl;
    }

    return 0;
}

