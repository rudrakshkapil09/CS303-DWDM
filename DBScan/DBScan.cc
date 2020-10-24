//DBSCAN
#include <fstream>
#include <set>
#include <queue>
#include <string>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <iostream>
using namespace std;

//––––––––––––––global–consts–––––––––––––//
const int NUMBER_OF_POINTS = 100;
const int X_MAX = 40;
const int Y_MAX = 40;
const string inputFile = "input.txt";
const string outputFile = "clusters.txt";
const int E = 10;
const int MIN_PTS = 10;

//–––––––––––––––structs––––––––––––––––––––//
struct Point {
    double id, x, y;
    
    //constructors
    Point() {
        id = x = y = -1;
    }
    
    Point(double id, double x, double y)
    {
        this->id = id;
        this->x = x;
        this->y = y;
    }
};

enum State {UNVISITED = 0, VISITED, NOISY};
enum ClusterState {NOT_CLUSTERED = 0, CLUSTERED = 1};

//––––––––––––––––global variables–––––––––––––––//
vector<Point> allPoints(NUMBER_OF_POINTS);
vector<Point> unvisited;

vector<State> state(NUMBER_OF_POINTS);
vector<ClusterState> clusterState(NUMBER_OF_POINTS);

ifstream fin;
ofstream fout;

int currentClusterIndex = 1;

//–––––––––––––––utility–functions–––––––––––––––//

//swap 2 points
void swap(Point &p1, Point &p2)
{
    swap(p1.id, p2.id);
    swap(p1.x, p2.x);
    swap(p1.y, p2.y);
}

//generate n 2d points and store in file
void generatePoints()
{
    fout.open(inputFile);
    for (int i = 0; i < NUMBER_OF_POINTS; i++) {
        int x = rand() % X_MAX;
        int y = rand() % Y_MAX;
        fout << x << " " << y << " -1\n";
    }
    fout.close();
}
                    
//get initial points and store them in allPoints
void getInitialPoints()
{
    fin.open(inputFile);
    int curr, index = 0, x = -1, y = -1;
    bool xVal = true;
    
    while (fin >> curr) {
        //end of curent point
        if (curr == -1) {
            allPoints[index] = Point(index, x, y);
            index++;
            continue;
        }
        
        //set and toggle
        (xVal) ? x = curr : y = curr;
        xVal = !xVal;
    }
    
    fin.close();
}

//returns euclidian distance between two points
double getDistance(Point p1, Point p2)
{
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

bool checkUnvisited()
{
    for (int i = 0; i < state.size(); i++)
        if (state[i] == UNVISITED)
            return true;
    return false;
}

int findPoint(Point p, vector<Point> vec)
{
    for (int i = 0; i < vec.size(); i++)
        if (p.id == vec[i].id)
            return i;
    return -1;
}


//outputs a cluster to file
void printPoint(Point p1, bool idFlag)
{
    if (idFlag)
        fout << "[" << p1.id+1 << "]\t";
    fout << "{" << p1.x << ", " << p1.y << "}\n";
}

void outputC(vector<Point> C)
{
    fout << "Cluster " << currentClusterIndex++ << ":\n";
    for (int i = 0; i < C.size(); i++)
        printPoint(C[i], true);
    fout << "\n––––––––––––––––––––––––––\n\n";
}

//DBSCAN
void DBSCAN() {
    //mark all points as unvisited
    for (int i = 0; i < NUMBER_OF_POINTS; i++) {
        state[i] = UNVISITED;
        clusterState[i] = NOT_CLUSTERED;
    }
    
    //loop till none are unvisited
    do {
        //randomly pick an unvisited
        int index;
        do {
            index = rand() % NUMBER_OF_POINTS;
        } while (state[index] != UNVISITED);

        //mark it as visited
        state[index] = VISITED;
        Point p = allPoints[index];
        
        //get its e-neighbourhood - add to it if distance is <= e
        vector<Point> neighbourhood;
        for (int i = 0; i < NUMBER_OF_POINTS; i++) {
            if (i == index) continue;   //skip the point
            if (getDistance(allPoints[i], p) <= E)
                neighbourhood.push_back(allPoints[i]);
        }
        
        //if it has at least min points
        if (neighbourhood.size() >= MIN_PTS) {
            //create a new cluster C and p to C
            vector<Point> C;
            C.push_back(p);
            clusterState[index] = CLUSTERED;
            
            //for each unvisited e-neighbour
            for (int i = 0; i < neighbourhood.size(); i++) {
                //gets actual global index of the e-nighbour, pdash
                Point pDash = neighbourhood[i];
                int pointIndex = findPoint(pDash, allPoints);
                
                //if pdash is unvisited
                if (state[pointIndex] == UNVISITED) {
                    state[pointIndex] = VISITED;
                    
                    //get e-neighbourhood of pdash
                    vector<Point> newNeighbourhood;
                    for (int j = 0; j < NUMBER_OF_POINTS; j++) {
                        if (j == pointIndex) continue;   //skip the point itself
                        if (getDistance(allPoints[j], pDash) <= E)
                            newNeighbourhood.push_back(allPoints[j]);
                    }
                    
                    //if the size of pdash's e-n is >= MINPTS
                    if (newNeighbourhood.size() >= MIN_PTS) {
                        //add those points to N
                        for (int i = 0; i < newNeighbourhood.size(); i++)
                            neighbourhood.push_back(newNeighbourhood[i]);
                    }
                }
                
                //if the point is not part of a cluster, add it to C
                if (clusterState[pointIndex] == NOT_CLUSTERED) {
                    clusterState[pointIndex] = CLUSTERED;
                    C.push_back(pDash);
                }
            }
            
            outputC(C);
        }
        else state[index] = NOISY;
        
    } while (checkUnvisited());
}

//––––––––––––––––––––MAIN–––––––––––––––––––––––//
int main()
{
    generatePoints();
    
    fout.open(outputFile);
    
    srand((unsigned)time(NULL));
    getInitialPoints();
    DBSCAN();

    fout.close();
    return 0;
}

