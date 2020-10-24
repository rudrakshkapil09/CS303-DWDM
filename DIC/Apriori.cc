//NORMAL APRIORI
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <vector>
#include <string>
using namespace std;

//––––––––––––––––typedefs–––––––––––––––––––––––––––//
typedef set<set<int> > SetSet;
typedef map<set<int>, int> MapSet;
typedef vector<set<int> > VecSet;

//––––––––––––––––global variables–––––––––––––––––––//
MapSet :: iterator it, it2;
set<int> :: iterator sit;
SetSet :: iterator ssit, ssit2;
set<int> tempSet;
vector<int> temp;

int minSup = 2;
int transCount = 10;
int itemsCount = 5;
int M = 5;
string datafile = "tbtask.txt";

//–––––––––––––––––––––DIC––––––––––––––––––––––––––//
SetSet DC;	//dashed circle -> all 1 itemsets (suspected infrequent) 
SetSet DS;	//dashed square -> suspected frequent
SetSet SS;	//solid square -> confirmed frequent
SetSet SC;	//solid circle -> confirmed infrequent
ifstream fin("tbtask.txt");
ofstream out("frequent.txt");

//–––––––––––––––––––functions––––––––––––––––––––––//

//utility to print set for debugging
void printSet(set<int> v, MapSet ItemSets)
{
	out << "{";
	for (sit = v.begin(); sit != v.end(); sit++)
		out << *sit << ", ";
	out << "}\t\t => " << ItemSets[v] << "\n";
}

//move itemsets from - to if they've been counted all transCount amount of times
void move(SetSet &from, SetSet &to, MapSet &transCounted, MapSet &ItemSets)
{
	//to be deleted from from
	vector<set<int> > toDelete;
	for (ssit = from.begin(); ssit != from.end(); ssit++) {
		if (transCounted[*ssit] >= transCount) {
			//erase from both maps as well as no longer dashed
			transCounted.erase(*ssit);
			//ItemSets.erase(*ssit);
			toDelete.push_back(*ssit);
			to.insert(*ssit);
		}
	}
	
	//remove those to be deleted:  
	for (int i = 0; i < toDelete.size(); i++)
		from.erase(toDelete[i]);
}

//recursive function to generate all possible subsets of first parameter
void getAllSubsets(set<int> curr, int index, vector<int> A, vector<set<int> > &res)
{
	
	if (index >= A.size())
		res.push_back(curr);
	else {
		getAllSubsets(curr, index+1, A, res);
		curr.insert(A[index]);
		getAllSubsets(curr, index+1, A, res);
	}
}

//check if all subsets are present in either SS or DS
bool allSubsetsPresent(set<int> S)
{
	vector<int> A; //vector representation of S
	for (sit = S.begin(); sit != S.end(); sit++)
		A.push_back(*sit);
	
	//call above function
	vector<set<int> > allSubsets;
	set<int> Temp;
	getAllSubsets(Temp, 0, A, allSubsets);
	
	//should be present in either DS or SS if it's not null set or set S itself
	for (int i = 0; i < allSubsets.size(); i++)
		if (DS.find(allSubsets[i]) == DS.end() 
			&& SS.find(allSubsets[i]) == SS.end()
				&& allSubsets[i].size() != 0
					&& allSubsets[i].size() != S.size())
						return false;
	return true;
}

//returns true if all elements in A belong to B 
bool belongsTo(set<int> A, set<int> B)
{
	for (sit = A.begin(); sit != A.end(); sit++)
		if (B.find(*sit) == B.end())
			return false;
	return true;
}

//returns union of two sets of sets
SetSet Union(SetSet A, SetSet B)
{
	SetSet res = A;
	for (ssit = B.begin(); ssit != B.end(); ssit++)
		res.insert(*ssit);
	return res;
}

//Reading in data from input file - M transactions
VecSet readMTrans()
{
	int count = M;
	VecSet res;
	while (count--) {
		set<int> trans;
		int curr;
		fin >> curr;	//get transnumber
		
		while (true) {
			fin >> curr;
			if (curr == -1)
				break;
			else if (curr == -2) {
				fin.close();
				fin.open(datafile);
				break;
			}
			else trans.insert(curr);
		}
		res.push_back(trans);
	}
	return res;
}

//MAIN
int main()
{
	//to hold support and times encountered respectively
	MapSet ItemSets;
	MapSet transCounted;
	
	//put one item sets in DC
	for (int i = 0; i < itemsCount; i++) {
		tempSet.clear();
		tempSet.insert(i+1);
		DC.insert(tempSet);
	}
	
	//begin loop
	while (!DS.empty() || !DC.empty()) {
		//read M transactions
		VecSet mTrans = readMTrans();
		
		//increment respective counters of the itemsets marked with dash if they appear in either DC or DS:
		SetSet DCS = Union(DC,DS);
		for (int i = 0; i < mTrans.size(); i++) {
			for (ssit = DCS.begin(); ssit != DCS.end(); ssit++) {
                if (belongsTo(*ssit, mTrans[i]))
					ItemSets[*ssit]++;
                transCounted[*ssit]++;
			}
		}
		
		//to do insert into and delete from DC later:
		vector<set<int> > toInsert;
		vector<set<int> > toDelete;

		//for each in DC
		for (ssit = DC.begin(); ssit != DC.end(); ssit++) {
			//if sup > minSup
			if(ItemSets.find(*ssit)->second >= minSup) {
				//move out of DC and into DS
				toDelete.push_back(*ssit);
				DS.insert(*ssit);
				
				//if any immediate superset has all subsets in SS or DS...
				for (int i = 1; i <= itemsCount; i++) {
					//add a new item to the set to form the super set
					if ((*ssit).find(i) == (*ssit).end()) {
						set<int> super = *ssit;
						super.insert(i);
						
						//...add the superset to DC
						if (allSubsetsPresent(super))
							toInsert.push_back(super);
					}
				}
			}
		}
		
		//insert into DC
		for (int k = 0; k < toInsert.size(); k++)
			DC.insert(toInsert[k]);
		
		//delete from DC
		for (int k = 0; k < toDelete.size(); k++)
			DC.erase(toDelete[k]);
		
		//if itemsets in DS have been counted transCount times move them to SS
		move(DS, SS, transCounted, ItemSets);
		
		//Similarly move itemsets from DC to SC
		move(DC, SC, transCounted, ItemSets);
	}
	
	out << "\n\nFrequent Item Sets:\n";
	for(ssit = SS.begin(); ssit != SS.end(); ssit++)
		printSet(*ssit, ItemSets);
}







