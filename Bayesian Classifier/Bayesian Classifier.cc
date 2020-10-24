//BAYESIAN CLASSIFIER
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

//––––––––––––––-global variables–––––––––//
ifstream fin;
ofstream fout;
string inputfile = "tbtask.txt";
vector<DPTR> testTuples;
vector<DPTR> trainTuples;
vector<Classification> results;

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
void storeInformation()
{
    //create root and open file
    DPTR tempData = new dataEntry;
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
                    trainTuples.push_back(data);
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
}

//returns count of members of each class in D
vector<double> getClassCounts(vector<DPTR> D)
{
    vector<double> ans(M, 0);
    for (int i = 0; i < D.size(); i++)
        ans[D[i]->buysComputer]++;
    return ans;
}

//function to return conditional probability P(X|c) - use LAPLACIAN CORRECTION to avoid 0 error
double getCP(DPTR X, int c, int total)
{
    double CP = 0;
    
    //check each training tuple
    int matchingAge = 1, matchingIncome = 1, matchingStudent = 1, matchingCreditRating = 1;
    for (int i = 0; i < trainTuples.size(); i++) {
        //class label must be matching
        if (trainTuples[i]->buysComputer != c)
            continue;
            
        //then increment matching counts depening on data
        if (trainTuples[i]->age == X->age)
            matchingAge++;
        if (trainTuples[i]->income == X->income)
            matchingIncome++;
        if (trainTuples[i]->isStudent == X->isStudent)
            matchingStudent++;
        if (trainTuples[i]->creditRating == X->creditRating)
            matchingCreditRating++;
    }
    
    //calculate product of all
    CP = (matchingAge * matchingIncome * matchingStudent * matchingCreditRating) / (pow(total, 4));
    return CP;
}

//test each tuple
double getPercentageCorrect()
{
    double correct = 0;

    //get total class counts
    vector<double> counts = getClassCounts(trainTuples);
    
    //check for each test tuple
    for (int i = 0; i < testCount; i++) {
        fout << "ID-" << testTuples[i]->id << ":\n";
        string actual = ClassificationDict[testTuples[i]->buysComputer], obtained;
        fout << "Actual => " << actual << "\n";
        
        //check for each class
        int maxIndex = -1;
        double maxProbability = -1;
        for (int c = 0; c < M; c++) {
            //get conditional probability P(test[i]|c) and multiply with class P(c)
            double CP = getCP(testTuples[i], c, counts[c]);
            double P = CP * (counts[c]/trainTuples.size());
            
            //get max
            if (P > maxProbability) {
                P = maxProbability;
                maxIndex = c;
            }
        }
        
        //assign most probable as label
        obtained = ClassificationDict[maxIndex];
        
        //compare
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
    
    Label["Age"] = (int) AgeDict.size();
    Label["Income"] = (int)IncomeDict.size();
    Label["Student"] = (int)StudentDict.size();
    Label["Credit_Rating"] = (int)CreditRatingDict.size();
    
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
    
    //poll and test
    storeInformation();
    double correct = getPercentageCorrect();
    fout << "\nPercentage correctly classified => "<< correct << "%\n\n";
    
    fout.close();
    return 0;
}

