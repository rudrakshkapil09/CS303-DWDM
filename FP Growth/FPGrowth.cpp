//sna
#include <fstream>
#include <set>
#include <queue>
#include <vector>
#include <iostream>
using namespace std;

//––––––––––––––––––––constants––––––––––––––––––––––//
const int minSup = 2;
const int itemsCount = 5;
const int transCount = 10;

//––––––––––––––––––––structures––––––––––––––––––//
typedef
struct TreeNode {
    int item;
    int support;
    TreeNode *next;
    TreeNode *child[itemsCount+1];    //because we're using 1 indexing
    TreeNode *parent;
    
    //constructors:
    TreeNode () {        //used only for root node
        item = 0;
        support = 0;
        next = NULL;
        parent = NULL;
        for (int i = 0; i <= itemsCount; i++)
            child[i] = NULL;
    }
    
    TreeNode (int x) {    //used for every other node
        item = x;
        support = 1;    //default you would make support 1
        next = NULL;
        parent = NULL;
        for (int i = 0; i <= itemsCount; i++)
            child[i] = NULL;
    }
    
} *TPTR;

struct TableEntry {
    int item;
    int support;
    int priority;   //0 -> first
    TreeNode *ptr;
};

//–––––––––––––––––––typedefs–––––––––––––––––––––––//
typedef vector<set<int> > VecSet;
typedef vector<vector<int> > VecVec;
typedef vector<TableEntry> Table;
typedef pair<vector<int>, int> PairVI;
typedef vector<pair<int, int> > VecPair;

//––––––––––––––––––––global–variables–––––––––––––//
string datafile = "tbtask.txt";
ifstream fin;
ofstream fout;
//TableEntry tempTE;
Table originalTable;

//––––––––––––––––––––functions––––––––––––––––––//

//utility function to print a set
void printSet(set<int> s)
{
    fout << "{";
    set<int> :: iterator sitr, sitr2;
    for (sitr = s.begin(); sitr != s.end(); sitr++) {
        sitr2 = next(sitr);
        sitr2 != s.end() ? fout << *sitr << ", " : fout << *sitr << "}";
    }
}

//utility function to print a VecVec
void printVecSet(VecSet VS)
{
    for (int i = 0; i < VS.size(); i++) {
        cout << "{";
        
        set<int> s = VS[i];
        set<int> :: iterator sitr, sitr2;
        for (sitr = s.begin(); sitr != s.end(); sitr++) {
            sitr2 = next(sitr);
            sitr2 != s.end() ? cout << *sitr << ", " : cout << *sitr << "}";
        }
        cout << "\n";
    }
}

//utility function to print a VecVec
void printVecVec(VecVec VV)
{
    for (int i = 0; i < VV.size(); i++) {
        cout << "{";
        for (int j = 0; j < VV[i].size(); j++) {
            cout << VV[i][j] << ", ";
        }
        cout << "}\n";
    }
}

//utility function used to sort a vector in custom manner
bool mycompare(const TableEntry &T1, const TableEntry &T2)
{
    return T1.support > T2.support;
}

bool compareWithPriority(const TableEntry &T1, const TableEntry &T2)
{
    if (T1.support == T2.support)
        return T1.priority < T2.priority;
    return T1.support > T2.support;
}

//utility function to check if the root of a tree has any children -> not empty
bool isNonEmpty(TPTR T)
{
    for (int i = 0; i < itemsCount; i++)
        if (T->child[i])
            return true;
    return false;
}

//count items and store as pairs with support - remove infrequent - sort
VecSet getSingleItems(Table &table)
{
    //Vector of Sets of transactions
    VecSet transVecSet;
    bool getTrans = true;    //used to make sure transactions are only stored once.
    
    //count items and only store frequent ones:
    for (int i = 1; i <= itemsCount; i++) {
        fin.open(datafile);
        
        bool skip = true;        //skipping first number of each row -> transID
        int curr;                //current item
        int count = 0;             //total number of transactions the item appears in
        
        //check each transaction
        set<int> trans;    //used to store the entire current transaction
        for (int j = 1; j <= transCount; ) {
            fin >> curr;
            
            if (skip)    //transaction number
                skip = false;
            else if (curr == 0) {    //end of line
                if (getTrans)    //only add each transaction once
                    transVecSet.push_back(trans);
                trans.clear();
                skip = true;
                j++;
            }
            else {    //item
                if (curr == i)
                    count++;
                trans.insert(curr);
            }
        }
        
        getTrans = false;
        fin.close();
        
        if (count >= minSup) {
            TableEntry tempTE;
            tempTE.item = i;
            tempTE.support = count;
            tempTE.ptr = NULL;
            table.push_back(tempTE);
        }
    }
    
    //sort table:
    sort(table.begin(), table.end(), mycompare);
    
    //store priorities
    for (int i = 0; i < table.size(); i++)
        table[i].priority = i;
    
    return transVecSet;
}

//returns VecVec containing each transaction consisting
//of only frequent items in decreasing order $$ - change method of comparison
VecVec getFrequentSortedTransactions(VecSet VS, Table table)
{
    VecVec res;
    
    //for each transaction in VS
    for (int i = 0; i < VS.size(); i++) {
        vector<int> trans;    //used to hold the required form of the transaction
        
        //for each item in the created table in order
        for (int j = 0; j < table.size(); j++)
            //if the item of the table is in the current transaction
            if (VS[i].find(table[j].item) != VS[i].end())
                //add the item to the required transaction
                trans.push_back(table[j].item);
        res.push_back(trans);
    }
    return res;
}

// returns index of item in table
int findItem(int item, Table table)
{
    for (int i = 0; i < table.size(); i++)
        if (table[i].item == item)
            return i;
    return -1;
}
    
// recursive function to insert items into the tree
// creates parent and next pointers as well during insertion
void insertTransaction(TPTR &T, Table &table, vector<int> transaction, int index, TPTR prev[])
{
    //for each element
    if (index < transaction.size()) {
        //find required node:
        TPTR tempNode = T->child[transaction[index]];
        
         //if it already exists simply increment support
        if (tempNode != NULL)
            tempNode->support++;
        else {
            tempNode = new TreeNode(transaction[index]);   //create the node with support = 1
            tempNode->parent = T;   //link parent pointer
            T->child[transaction[index]] = tempNode;
            
            //link next pointer:
            // if leftmost node of the item -> hasn't been linked to table yet
            if (prev[transaction[index]] == NULL)
                table[findItem(transaction[index], table)].ptr = tempNode;    //link to table
            // a node of the same item exists on the left
            else
                prev[transaction[index]]->next = tempNode;  //link the two nodes
            prev[transaction[index]] = tempNode;
            
        }
        
        //recursively go to next item and then link from parent on return from call
        insertTransaction(tempNode, table, transaction, index+1, prev);
    }
}

//function that simply inserts each transaction into the tree
void createTree(TPTR &T, Table &table, VecVec transactions, TPTR prev[])
{
    for (int i = 0; i < transactions.size(); i++)
        insertTransaction(T, table, transactions[i], 0, prev);
}

bool onlyOnePath(TPTR T)
{
    int numPaths = 0;   //returned at end
    TPTR next = NULL;       //used to traverse deeper
    
    //check each child
    for (int i = 0; i <= itemsCount; i++) {
        if (T->child[i] != NULL) {
            next = T->child[i]; //this is the child where we will go next
            numPaths++;
            if (numPaths > 1)   //more than one child -> more than one path
                return false;
        }
    }

    if (numPaths == 0)  // leaf node at end
        return true;
    else return onlyOnePath(next);  // go one level down
}

void getAllPossiblePaths(TPTR T, set<int> S, int minFreq)
{
    //still haven't reached end of leaf node
    if (T) {
        set<int> NT = S;
        int nextFreq = INT_MAX;
        
        if (T->item != 0) { //do for non-root
            NT.insert(T->item);
            nextFreq = min(minFreq, T->support);
        }
        
        //recur for each child - DFS
        bool childExists = false;
        for (int i = 1; i < itemsCount; i++) {
            if (T->child[i]) {
                childExists = true;
                if (T->item != 0)  //recur for non-root
                    getAllPossiblePaths(T->child[i], NT, nextFreq); //added
                getAllPossiblePaths(T->child[i], S, minFreq);       //not added
            }
        }
        
        //simply print both sets in next recursive call if leaf node
        if (!childExists) {
            if (T->item != 0)
                getAllPossiblePaths(NULL, NT, nextFreq);        //added
            getAllPossiblePaths(NULL, S, minFreq);              //not added
        }
    }
    //otherwise print if needed
    else if (minFreq != INT_MAX) {
        printSet(S);
        fout << " => " << minFreq << "\n";
    }
}

//returns vector of conditional pattern base with support
vector<PairVI> getConditionalPatternBase(int index, Table table)
{
    vector<PairVI> res;         //to be returned
    TPTR Y = table[index].ptr;  //to traverse tree from left to right
    
    //start from the leftmost node via the table
    while (Y) {
        //keep track of next node to go to (same item)
        TPTR nextY = Y->next;
        
        //current PairVI
        PairVI curr;
        curr.second = Y->support;   //support is that of the item itself
        Y = Y->parent;              //skip the item itself
        
        //store path in reverse order in PV
        while (Y) {
            if (Y->item != 0)   //if not null root
                curr.first.push_back(Y->item);  //store it in the path
            Y = Y->parent;      //move upwards
        }
        
        //move to right after storing curr in res if curr is valid
        if (curr.first.size() > 0)
            res.push_back(curr);
        Y = nextY;
    }
    return res;
}

//returns new header table from pattern base
Table getNewHeaderTable(vector<PairVI> CPB)
{
    Table table;
    
    //for each item
    for (int i = 1; i <= itemsCount; i++) {
        //count total support in all CPB
        int support = 0;
        for (int j = 0; j < CPB.size(); j++)    //for each CPB
            if (find(CPB[j].first.begin(), CPB[j].first.end(), i) != CPB[j].first.end())    //if i is found
                support += CPB[j].second;
        
        //add this item to the table if it has support >= minSupport
        if (support >= minSup) {
            TableEntry tempTE;
            tempTE.item = i;
            tempTE.support = support;
            tempTE.priority = findItem(i, originalTable);       //priority is basically its original index
            tempTE.ptr = NULL;
            table.push_back(tempTE);
        }
    }
    
    //sort and return the table
    sort(table.begin(), table.end(), compareWithPriority);
    return table;
}

//returns pointer to a new tree created using intermediate tables $$
TPTR createNewTree(Table &table, vector<PairVI> CPB)
{
    //create root to return later
    TPTR root = new TreeNode;
    
    //get a VecSet from CPB, then create a sorted VecVec from that
    VecSet VS;
    for (int i = 0; i < CPB.size(); i++) {       //for each pattern base
        set<int> tempSet(CPB[i].first.begin(), CPB[i].first.end()); //create a set with those frequent transactions
        for (int j = 0; j < CPB[i].second; j++)     //insert the set as many times as support
            VS.push_back(tempSet);
    }
    VecVec VV = getFrequentSortedTransactions(VS, table);
    
    //create and return the new tree
    TPTR previous[itemsCount+1];        //used to create next pointers
    for (int i = 0; i < itemsCount+1; i++)
        previous[i] = NULL;
    createTree(root, table, VV, previous);
    return root;
}

//actual mining function
void FPGrowthAlgorithm(TPTR T, Table table, set<int> S, int headerFreq)
{
    //output frequent item set except at start
    if (!S.empty()) {
        printSet(S);
        fout << " => " << headerFreq << "\n";
    }
    
    //check if tree contains multiple paths
    if (onlyOnePath(T))     //if not, print out all possible sets from the single path
        getAllPossiblePaths(T, S, INT_MAX);
    else {
        //go in reverse order in the header table
        for (int i = (int) table.size()-1; i >= 0; i--) {
            //create set Beta = header union S
            set <int> B = S;
            B.insert(table[i].item);
            int nextFreq = table[i].support;
            
            //get conditional pattern base
            vector<PairVI> CPB = getConditionalPatternBase(i, table);
            
            //get new table from CPB
            Table newTable = getNewHeaderTable(CPB);
            
            //create new tree using new table and recur with it if it is nonEmpty
            TPTR newTree = createNewTree(newTable, CPB);
            //if (isNonEmpty(newTree))  //removed so you can print out sets 2 and 2,3 -> no CPB hence no tree, but they are a path in the tree
            FPGrowthAlgorithm(newTree, newTable, B, nextFreq);
        }
    }
}


//–––––––––––––––––––––––MAIN–––––––––––––––––––––––//
int main()
{
    fout.open("frequent.txt");
    
    /* get original transactions */
    cout << "Original:\n";
    VecSet originalTransactions = getSingleItems(originalTable);
    printVecSet(originalTransactions);
    
    /* sort each transactions and remove infrequent items */
    cout << "\n\nSorted:\n";
    VecVec requiredTransactions = getFrequentSortedTransactions(originalTransactions, originalTable);
    printVecVec(requiredTransactions);
    
    /* create the tree: */
    TPTR root = new TreeNode();    // root node is null -> item = 0
    TPTR previous[itemsCount+1];    //used to create next pointers
    createTree(root, originalTable, requiredTransactions, previous);
    cout << "\nTree created\n";
    
    /* mine the tree */
    set <int> S;
    FPGrowthAlgorithm(root, originalTable, S, 0);
    
    fout.close();
}
