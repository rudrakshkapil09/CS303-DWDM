//decision tree creation and testing
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
const int tupleCount = 14;
const int trainCount = 10;
const double testCount = tupleCount - trainCount;
int M = 2;                                      //number of classes
typedef unordered_map<string, int> ReverseDictionary;
typedef unordered_map<int, string> Dictionary;

//–––––––––––enums–for–data–––––––––––––––//
enum NodeTag        {NON_LEAF = 0, LEAF = 1};
enum Age            {YOUTH = 0, MIDDLE_AGED = 1, SENIOR = 2};
enum Income         {LOW = 0, MEDIUM = 1, HIGH = 2};
enum Student        {NO = 0, YES = 1};
enum CreditRating   {FAIR = 0, EXCELLENT = 1};
enum Classification {DOESNT = 0, BUYS = 1};

Dictionary AgeDict;//({{YOUTH, "Youth"}, {MIDDLE_AGED, "Middle_Aged"}, {SENIOR, "Senior"}});
Dictionary IncomeDict;//({{LOW, "Low"}, {MEDIUM, "Medium"}, {HIGH, "High"}});
Dictionary StudentDict;//({{NO, "No"}, {YES, "Yes"}});
Dictionary CreditRatingDict;//({{FAIR, "Fair"}, {EXCELLENT, "Excellent"}});
Dictionary ClassificationDict;//({{DOESNT, "No"}, {BUYS, "Yes"}});

ReverseDictionary Label;//({{"Age", 3}, {"Income", 3}, {"Student", 2}, {"Credit_Rating", 2}});
ReverseDictionary AgeDictRev;//({{"Youth", 0}, {"Middle_Aged", 1}, {"Senior", 2}});
ReverseDictionary IncomeDictRev;//({{"Low", 0}, {"Medium", 1}, {"High", 2}});
ReverseDictionary StudentDictRev;//({{"No", 0}, {"Yes", 1}});
ReverseDictionary CreditRatingDictRev;//({{"Fair", 0}, {"Excellent", 1}});
ReverseDictionary ClassificationDictRev;//({{"No", 0}, {"Yes", 1}});

//–––––––––––––structs––––––––––––––––––––//
typedef
struct dataEntry {
    int id;
    Age age;
    Income income;
    Student isStudent;
    CreditRating creditRating;
    Classification buysComputer;
} *DPTR;

typedef
struct decisionTreeNode {
    NodeTag tag;
    string information;
    vector<DPTR> D;                                 //data tuples
    vector<pair<string, struct decisionTreeNode *> > partition;
    
    decisionTreeNode()
    {
        tag = NON_LEAF;
        information = "NULL";
        partition.clear();
    }
} *TPTR;

//––––––––––––––-global variables–––––––––//
ifstream fin;
ofstream fout;
string inputfile = "tbtask.txt";
vector<DPTR> testTuples;

//––––––––––––utility–functions–––––––––––//
int stringToNum(string s)
{
    int ans = 0;
    for (int i = 0; i < s.size(); i++) {
        ans *= 10;
        ans += (s[i] - '0');
    }
    return ans;
}

//function to store all tuples into the root node N from file input
TPTR storeInformation()
{
    //create root and open file
    TPTR root = new decisionTreeNode();
    DPTR tempData = new dataEntry;
    root->tag = NON_LEAF;
    fin.open(inputfile);

    //take input one at a time
    string curr;
    int id = -1, currLabel = 0, count = -1; //keep track of how many tuples are used to train
    while (fin >> curr) {
        // new tuple beginning - get id
        if (curr[0] == 'R' || curr == "END") {
            count++;
            //add the old tuple first - skip first time
            if (id != -1) {
                DPTR data = tempData;
                if (count <= trainCount)
                    root->D.push_back(data);
                else
                    testTuples.push_back(data);
            }
            
            //reached eof
            if (curr == "END")
                break;
            
            //set new tuple id
            tempData = new dataEntry;
            id = stringToNum(curr.substr(1));
            tempData->id = id;
            currLabel = 0;
            continue;
        }
        
        //add all following data of the tuple
        switch (currLabel++) {
            case 0: tempData->age = (Age) AgeDictRev[curr];           break;
            case 1: tempData->income = (Income) IncomeDictRev[curr];        break;
            case 2: tempData->isStudent = (Student) StudentDictRev[curr];     break;
            case 3: tempData->creditRating = (CreditRating) CreditRatingDictRev[curr];  break;
            case 4: tempData->buysComputer = (Classification) ClassificationDictRev[curr];  break;
        }
    }
    
    //close file and return root
    fin.close();
    return root;
}

//returns count of members of each class in D
vector<double> getClassCounts(vector<DPTR> D)
{
    vector<double> ans(M, 0);
    for (int i = 0; i < D.size(); i++)
        ans[D[i]->buysComputer]++;
    return ans;
}

//returns expected information needed to classify a tuple in D
double getInfoD(vector<DPTR> D)
{
    vector<double> counts = getClassCounts(D);
    int total = (int) counts.size();
    
    double ans = 0;
    for (int i = 0; i < M; i++) //for each class:
        if (counts[i] != 0)
            ans -= (counts[i]/total) * log2(counts[i]/total);

    return ans;
}

//returns expected information needed to classify a tuple in D if the tuples are partitioned according to any one attribute: secondTerm is basically info(Dj)
double getInfoAD(vector<DPTR> D, string attribute)
{
    double infoAD = 0;
                            
    //for each value of the attribute
    int numberOfValues = Label[attribute];
    for (int i = 0; i < numberOfValues; i++) {
        //create a new data set containing only those tuples having attribute value as i
        vector<DPTR> newD;
        for (int j = 0; j < D.size() && D[j] != NULL; j++) {
            //check if attribute is matching, and add to new data table if it is
            if (attribute == "Age" && D[j]->age == i)
                newD.push_back(D[j]);
            else if (attribute == "Income" && D[j]->income == i)
                newD.push_back(D[j]);
            else if (attribute == "Student" && D[j]->isStudent == i)
                newD.push_back(D[j]);
            else if (attribute == "Credit_Rating" && D[j]->creditRating == i)
                newD.push_back(D[j]);
        }
        
        //apply formula:
        infoAD += (newD.size()/D.size()) * getInfoD(newD);
    }
    return infoAD;
}

//returns the best splitting criterion as a Label
string getBestAttribute(vector<DPTR> D, vector<string> &attributes)
{
    double maxGain = -999999;
    int maxGainIndex = 0;
    double infoD = getInfoD(D), infoAD, currGain;
    
    //check each label
    for (int i = 0; i < attributes.size(); i++) {
        infoAD = getInfoAD(D, attributes[i]);
        currGain = infoD - infoAD;
        
        //update if we get a better value
        if (currGain > maxGain) {
            maxGainIndex = i;
            maxGain = currGain;
        }
    }
    
    //return best attribute after removing it from the list of remaining
    string toReturn = attributes[maxGainIndex];
    attributes.erase(attributes.begin() + maxGainIndex);
    return toReturn;
}

//checks if tuples in D are all of the same class
bool allSameClass(vector<DPTR> D)
{
    bool same = true;
    int first = D[0]->buysComputer;
    for (int i = 1; i < D.size(); i++)
        if (D[i]->buysComputer != first)
            return false;
    return same;
}

//returns majority vote in the case that attribute list is empty
string getMajority(vector<DPTR> D)
{
    //first count all occurences
    vector<double> counts = getClassCounts(D);
    
    //find index of class with majority vote
    int max = INT_MIN, maxIndex = -1;
    for (int i = 0; i < counts.size(); i++) {
        if (max < counts[i]) {
            max = counts[i];
            maxIndex = i;
        }
    }

    //return getStringRepresentation(Classification, maxIndex);
    return ClassificationDict[maxIndex];
}

//main algorithm for creating the decision tree
TPTR createTree(bool firstTime, vector<DPTR> D, vector<string> &attributes)
{
    //create node
    TPTR N;
    if (firstTime) {
        N = storeInformation();
        D = N->D;
    }
    else {
        N = new decisionTreeNode;
        N->D = D;
    }
    
    //if all in same class, make it leaf and return
    if (allSameClass(N->D)) {
        N->tag = LEAF;
        N->information = ClassificationDict[N->D[0]->buysComputer];
        return N;
    }
    
    //if attribute list is empty, take majority vote and return as leaf
    if (attributes.size() == 0) {
        N->tag = LEAF;
        N->information = getMajority(D);
        return N;
    }
    
    //get best splitting criterion and label as such
    N->information = getBestAttribute(D, attributes);
    
    //partition the tuples and grow subtrees for each outcome of the attribute:
    int numberOfValues = Label[N->information];
    for (int i = 0; i < numberOfValues; i++) {
        //get current branch outcome as a string
        string currentOutcome;
        if (N->information == "Age")
            currentOutcome = AgeDict[i];
        else if (N->information == "Income")
            currentOutcome = IncomeDict[i];
        else if (N->information == "Student")
            currentOutcome = StudentDict[i];
        else if (N->information == "Credit_Rating")
            currentOutcome = CreditRatingDict[i];
        
        //create a new data set containing only those tuples having attribute value as i
        vector<DPTR> newD;
        for (int j = 0; j < D.size(); j++) {
            //check if attribute is matching, and add to new data table if it is
            if (N->information == "Age" && D[j]->age == i)
                newD.push_back(D[j]);
            else if (N->information == "Income" && D[j]->income == i)
                newD.push_back(D[j]);
            else if (N->information == "Student" && D[j]->isStudent == i)
                newD.push_back(D[j]);
            else if (N->information == "Credit_Rating" && D[j]->creditRating == i)
                newD.push_back(D[j]);
        }
        
        //if partition is empty, attach a leaf labeled with majority class in D to N
        if (newD.size() == 0) {
            TPTR temp = new decisionTreeNode;
            temp->tag = LEAF;
            temp->information = getMajority(D);
            N->partition.push_back(make_pair(currentOutcome, temp));
        }
        //else recur for the partition and attach to N on return
        else N->partition.push_back(make_pair(currentOutcome, createTree(false, newD, attributes)));
    }
    return N;
}

//––––––––––––––––––––TESTING––––––––––––––––––––//
string traverseTree(TPTR T, DPTR D)
{
    //if leaf return the class label
    if (T->tag == LEAF)
        return T->information;
    
    //if non leaf, recur for next appropriate node
    TPTR nextBranch = NULL;
    if (T->information == "Age") {
        for (int i = 0; i < T->partition.size(); i++) {
            if (T->partition[i].first == AgeDict[D->age]) {
                nextBranch = T->partition[i].second;
                break;
            }
        }
    }
    else if (T->information == "Income") {
        for (int i = 0; i < T->partition.size(); i++) {
            if (T->partition[i].first == IncomeDict[D->income]) {
                nextBranch = T->partition[i].second;
                break;
            }
        }
    }
    else if (T->information == "Student") {
        for (int i = 0; i < T->partition.size(); i++) {
            if (T->partition[i].first == StudentDict[D->isStudent]) {
                nextBranch = T->partition[i].second;
                break;
            }
        }
    }
    else if (T->information == "Credit_Rating") {
        for (int i = 0; i < T->partition.size(); i++) {
            if (T->partition[i].first == CreditRatingDict[D->creditRating]) {
                nextBranch = T->partition[i].second;
                break;
            }
        }
    }
    return traverseTree(nextBranch, D);
}

double getPercentageCorrect(TPTR T)
{
    double correct = 0;
    
    //check for each test tuple
    for (int i = 0; i < testCount; i++) {
        fout << "ID-" << testTuples[i]->id << ":\n";
        string actual = ClassificationDict[testTuples[i]->buysComputer];
        string obtained = traverseTree(T, testTuples[i]);
        
        fout << "Actual => " << actual << "\n";
        
        if (actual == obtained) {
            correct++;
            fout << "CORRECTLY classified as " << obtained << ".\n";
        }
        else fout << "INCORRECTLY classified as " << obtained << ". ==> [X]\n";
        
        fout << "––––––––––––––//––––––––––––\n";
    }
    
    return (correct / testCount) * 100;
}

//––––––––––––––––––––MAIN–––––––––––––––––––––––//
int main()
{
	//set up
    fout.open("classifcation.txt");
    vector<DPTR> dummy;
    vector<string> attributes; 
	attributes.push_back("Age");
	attributes.push_back("Income");
	attributes.push_back("Student");
	attributes.push_back("Credit_Rating");
	    
	AgeDict[YOUTH] = "Youth";
	AgeDict[MIDDLE_AGED] = "Middle_Aged";
	AgeDict[SENIOR] = "Senior";
	IncomeDict[LOW] = "Low";
	IncomeDict[MEDIUM] = "Medium";
	IncomeDict[HIGH] = "High";
	StudentDict[NO] = "No";
	StudentDict[YES] = "Yes";
	CreditRatingDict[FAIR] = "Fair";
	CreditRatingDict[EXCELLENT] = "Excellent";
	ClassificationDict[DOESNT] = "No";
	ClassificationDict[BUYS] = "Yes";
	
	Label["Age"] = AgeDict.size();
	Label["Income"] = IncomeDict.size();
	Label["Student"] = StudentDict.size();
	Label["Credit_Rating"] = CreditRatingDict.size();
	
	AgeDictRev["Youth"] = 0;
	AgeDictRev["Middle_Aged"] = 1;
	AgeDictRev["Senior"] = 2;
	IncomeDictRev["Low"] = 0;
	IncomeDictRev["Medium"] = 1;
	IncomeDictRev["High"] = 2;
	StudentDictRev["No"] = 0;
	StudentDictRev["Yes"] = 1;
	CreditRatingDictRev["Fair"] = 0;
	CreditRatingDictRev["Excellent"] = 1;
	ClassificationDictRev["No"] = 0;
	ClassificationDictRev["Yes"] = 1;	
	
    //create tree and then test
    TPTR root = createTree(true, dummy, attributes);
    double correct = getPercentageCorrect(root);
    fout << "\nPercentage correctly classified => "<< correct << "%\n\n";
    
    fout.close();
    return 0;
}

