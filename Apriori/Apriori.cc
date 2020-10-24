//NORMAL APRIORI
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
using namespace std;

//––––––––––––––––global variables–––––––––––––––––––//
map<vector<int>, int> itemsets[10]; 		         //Lk
map<vector<int>, int> :: iterator it, it2;
vector<int> temp;
int minSup = 2;
int transCount = 9;
int itemsCount = 5;
string datafile = "tbtask.txt";

//––––––––––––––––functions–––––––––––––––––––––––––//

//utility to print vector for debugging
void printVector(vector<int> v)
{
	for (int i = 0; i < v.size(); i++)
		cout << v[i] << ", ";
	cout << "\n";
}

//generate one item sets at the start from data
void generateOneItemSets()
{
	//check data for each item
	for (int i = 1; i <= itemsCount; i++) {
		ifstream in(datafile);
		
		bool skip = true;		//skipping first number of each row -> transID
		int curr;				//current item
		int count = 0; 			//total number of transactions the item appears in
		
		//check each transaction
		for (int j = 1; j <= transCount; ) {
			in >> curr;	
			if (skip) {	
				skip = false;
				continue;
			}
			
			if (curr == i)
				count++;
			else if (curr == 0) {	//end of line
				skip = true;
				j++;
			}
		}
		
		//insert item into itemsets with count
		temp.clear();
		temp.push_back(i);
		itemsets[1][temp] = count;
		in.close();
	}
	return;
}

//removes infrequent itemsets from itemsets
void removeInfrequent(int level)
{
	for (it = itemsets[level].begin(); it != itemsets[level].end();) {
		if (it->second < minSup) {
			temp = it->first;
			it++;
			itemsets[level].erase(temp);
		}
		else it++;
	}
	return;
}

//checks if the subset has any infrequent subsets
bool hasInfrequentSubset(vector<int> v, int level)
{
	for (int i = 0; i < v.size(); i++) {
		temp = v;
		temp.erase(temp.begin()+i, temp.begin()+i);
		if (itemsets[level].find(temp) == itemsets[level].end())
			return false;
	}
	return true;
}

//print output of given level to given file
void outputToFile(string filename, int level)
{
	ofstream out(filename);
	
	//for each item set, print as {x, y} => #
	for (it = itemsets[level].begin(); it != itemsets[level].end(); it++) {
		out << "{";
		temp = it->first;
		for (int i = 0; i < temp.size(); i++) {
			out << temp[i];
			if (i != temp.size()-1)
				out << ", ";
		}
		out << "} => " << it->second << "\n";
	}
	
	out.close();
	return;
}

//checks if two item sets can be joined
bool areJoinable(vector<int> v1, vector<int> v2)
{
	if (v1.size() != v2.size())
		return false;
	for (int i = 0; i < v1.size()-1; i++)
		if (v1[i] != v2[i])
			return false;
	return true;	//all but last are same
}

//scan data to find # of transactions with all items of v
int getItemSetCount(vector<int> v)
{	
	bool skip = true;		//skipping first number of each row -> transID
	int curr;				//current item
	int count = 0; 			//total number of transactions the item appears in
	int vindex = 0;	
	
	ifstream in(datafile);
	//scan each transaction to check if the full itemset exists
	while (in >> curr) {
		if (skip)
			skip = false;
		else if (curr == v[vindex] && vindex < v.size()) {
			vindex++;
		}
		else if (curr == 0) {	//end of line
			if (vindex == v.size())
				count++;
			vindex = 0;
			skip = true;
		}
		else continue;
	}
	
	in.close();
	
	cout << "VECTOR: "; 
	printVector(v);
	cout << "appeared " << count << " times.\n\n";
	return count;
}

//generate the next (level+1) itemsets from level size itemsets
void generateItemSet(int level)
{
	//take first itemset
	for (it = itemsets[level].begin(); it != itemsets[level].end(); it++) {
		it2 = next(it, 1);
		//take second item set
		for (; it2 != itemsets[level].end(); it2++) {
			//skip if they cannot be joined correctly
			if (!areJoinable(it->first, it2->first))
				continue;
			
			//make the new itemset by appending last of second to first
			temp = it->first;
			temp.push_back(it2->first.back());
			
			//use apriori priority: if it has infrequent skip, else add to itemsets
			if (hasInfrequentSubset(temp, level)) {
				cout << "HAS INFREQUENT: ";
				printVector(temp);
				continue;
			}
			else itemsets[level+1][temp] = getItemSetCount(temp);
		}
	}
	return;
}

//MAIN
int main()
{
	//filenames
	string files[] = {"zero.txt", "one.txt", "two.txt", "three.txt", "four.txt", "five.txt"};
	
	//generate frequent one item sets:
	generateOneItemSets();
	removeInfrequent(1);
	outputToFile(files[1], 1);
	
	for (int i = 2; itemsets[i-1].size() > 0; i++) {
		generateItemSet(i-1);	
		removeInfrequent(i);
		outputToFile(files[i], i);
	}
	
	cout << "hello";
}
