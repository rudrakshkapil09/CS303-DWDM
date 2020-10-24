//sna
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
const int NUMBER_OF_POINTS = 20;
const int X_MAX = 40;
const int Y_MAX = 40;
const string inputFile = "input.txt";
const string outputFile = "clusters.txt";

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

typedef
struct TreeNode {
    int dummy = 1;  //for printing only
    pair<int, int> mean;
    vector<Point *> points;
    struct TreeNode *lc;
    struct TreeNode *rc;
} *TPTR;

//––––––––––––––––global variables–––––––––––––––//
vector<Point *> allPoints(NUMBER_OF_POINTS);
ifstream fin;
ofstream fout;
set <TPTR> :: iterator sit, sit2;

//–––––––––––––––utility–functions–––––––––––––––//

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
            allPoints[index] = new Point(index, x, y);
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
double getDistance(pair<int, int> p1, pair<int, int> p2)
{
    return sqrt(pow(p1.first - p2.first, 2) + pow(p1.second - p2.second, 2));
}

//returns a vector containing union of both points - i/p vectors must be and are disjoint
vector<Point *> unionPoints(vector<Point *> v1, vector<Point *> v2)
{
    vector<Point *> res = v1;
    for (int i = 0; i < v2.size(); i++)
        res.push_back(v2[i]);
    return res;
}

//gets mean of vector of points as a pair (xmean, ymean)
pair<int, int> getMean(vector<Point *> points)
{
    double xSum = 0, ySum = 0, num = points.size();
    for (int i = 0; i < points.size(); i++) {
        xSum += points[i]->x;
        ySum += points[i]->y;
    }
    
    return make_pair(xSum/num, ySum/num);
}

//prints a point
void printPoint(Point *p1, bool idFlag)
{
    fout << "{";
    if (idFlag)
        fout << p1->id+1 << ",";
    fout << p1->x << "," << p1->y << "}";
}

//prints a vector of points, i.e. the contents of a node
void printVector(vector<Point * > p)
{
    fout << "[";
    for (int i = 0; i < p.size(); i++) {
        printPoint(p[i], true);
        if (i != p.size() -1)
            fout << ", ";
    }
    fout << "]  ";
}

//prints level order
void levelOrder(queue<TPTR> Q)
{
    if (Q.size() > 0) {
        TPTR T = Q.front();
        Q.pop();
        if (T->dummy == 1 && Q.size() > 0) {
            fout << "\n\n";
            Q.push(T);
        }
        else {
            //print
            if (T->dummy == 0)
                printVector(T->points);
            if (T->lc) Q.push(T->lc);
            if (T->rc) Q.push(T->rc);
        }
        levelOrder(Q);
    }
}


//create a dendrogram:
void createAndPrintDendrogram()
{
    //use a set to store all current clusters
    set <TPTR> clusters;
    
    //create initial clusters and store
    for (int i = 0; i < NUMBER_OF_POINTS; i++) {
        TPTR temp = new TreeNode;
        temp->mean = make_pair(allPoints[i]->x, allPoints[i]->y);
        temp->points.push_back(allPoints[i]);
        temp->lc = temp->rc = NULL;
        temp->dummy = 0;
        clusters.insert(temp);
    }
    
    //keep combining until we have just 1 left
    while (clusters.size() > 1) {
        pair <TPTR, TPTR> minSoFar;
        double minMeanDiff = INT_MAX;
        //find pair of clusters having least difference in mean
        for (sit = clusters.begin(); sit != clusters.end(); sit++) {
            for (sit2 = next(sit); sit2 != clusters.end(); sit2++) {
                //get difference in means and update minimum if required
                double meanDiff = getDistance((*sit)->mean, (*sit2)->mean);
                if (meanDiff < minMeanDiff) {
                    minMeanDiff = meanDiff;
                    minSoFar = make_pair(*sit, *sit2);
                }
            }
        }
        
        //create a new node as parent of both nodes in minSoFar
        TPTR parent = new TreeNode;
        parent->lc = minSoFar.first;
        parent->dummy = 0;
        parent->rc = minSoFar.second;
        parent->points = unionPoints(parent->lc->points, parent->rc->points);
        parent->mean = getMean(parent->points);
        
        //remove the original nodes from the set, and add the parent
        clusters.erase(minSoFar.first);
        clusters.erase(minSoFar.second);
        clusters.insert(parent);
    }
    
    //print the root in level order
    TPTR dummy = new TreeNode;
    dummy->dummy = 1;
    dummy->lc = dummy->rc = NULL;
    queue<TPTR> Q;
    Q.push(*clusters.begin());  //only one item, so can use clusters.begin();
    Q.push(dummy);
    levelOrder(Q);
}

//––––––––––––––––––––MAIN–––––––––––––––––––––––//
int main()
{
    generatePoints();
    
    fout.open(outputFile);
    
    srand((unsigned)time(NULL));
    getInitialPoints();
    createAndPrintDendrogram();

    fout.close();
    return 0;
}

