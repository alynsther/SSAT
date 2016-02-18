/*****************************************************************************
 File:   ssat.cc
 Author: Adela Yang, Venecia Xu, Son Ngo
 Date:   February 2016

 Description:
 Implement a DPLL-based stochastic satisfiability (SSAT) solver to test the
    effectiveness of unit clause propagation, pure variable elimination, and 
    three different splitting heuristics
 
 Notes: 
 Run this program with 2 arguments on the command line
 
 Running instructions:
 Run this program with two arugments on the command line 
    the input file
    
    g++ -Wall -o ssat ssat.cc
    ./ssat [file name]

    Running on dover: g++ -std=c++11 -Wall -o ssat ssat.cc
    ./ssat [file name]

 Data Structures:

******************************************************************************/

/*****************************************************************************/
/* include files */
#include <iostream>
#include <vector>
#include <string> 
#include <fstream> 
#include <sstream>
#include <cstdlib>
#include <cmath> 
#include <algorithm>
#include <map>
#include <set>

using namespace std;

/***************************************************************************/
/* constants */
static const double CHOICE_VALUE = -1.0;
static const int POSITIVE = 1;
static const int NEGATIVE = -1;
static const int INVALID = 0; 

/***************************************************************************/

/***************************************************************************/
/* structs */
typedef struct varInfo{
    double value;                   // the value of the variables
    map<int, int> clauseMembers;    // key: index of the clauses the var is in
                                    // value: POS/NEG in its respective clauses        
} varInfo;

/***************************************************************************/
/* globals variables */
int maximumClauseLength;
int minimumClauseLength;
double averageClauseLength;
int seed;

int numVars;
int numClauses;

map <int, varInfo> variables;

//Adela
//map <int, vector<int> > clauses;

//Son
map <int, set<int> > clauses;
map <int, int> assignment;

//what happened to the solution tree variable?

/***************************************************************************/
/* functions */
double SOLVESSAT();
void readFile(string input);
void tokenize(string str, vector<string> &token_v);
pair<bool, int> isPureChoice(int variable);
int variableSignInClause(int clauseEntry);
void updateClausesAndVariables(int variable, int value);
void assign(int variable, int value);
void undoChanges();

/*****************************************************************************
 Function:  main
 Inputs:    argv
 Returns:   nothing
 Description:
 *****************************************************************************/
int main(int argc, char* argv[]) {
    //open the file for reading
    // string input  = "smallTEST.ssat"; //same as small1
    string input = argv[1];
    readFile(input);
    
	return 0;
}

/***************************************************************************
 Function:  readFile
 Inputs:    file name
 Returns:   nothing
 Description:
            reads the content of file and stores the information as global
                variables
 ***************************************************************************/
void readFile(string input) {
    string sTemp;
    ifstream inFile;
    vector<string> vSTemp;
    
    //Adela's version
    //vector<int> vITemp;

    //Son's version: 
    set <int> vITemp;

    int i;

    inFile.open(input.c_str());
    
    // file does not exist, then do nothing
    if(!inFile.is_open()){
        cout << "File is not valid" << endl;
        exit(1);
    }
    
    i = 4;
    while(i >= 0) {
        getline(inFile, sTemp);
        i--;
    }
    
    cout << "first line " << sTemp << endl;
    //the comments
    tokenize(sTemp, vSTemp);
    maximumClauseLength = stoi(vSTemp.back());
    cout << "maximumClauseLength " << maximumClauseLength << endl;

    getline(inFile, sTemp);
    tokenize(sTemp, vSTemp);
    minimumClauseLength = stoi(vSTemp.back());
    cout << "minimumClauseLength " << minimumClauseLength << endl;

    getline(inFile, sTemp);
    tokenize(sTemp, vSTemp);
    averageClauseLength = stod(vSTemp.back());
    cout << "averageClauseLength " <<averageClauseLength << endl;

    getline(inFile, sTemp);
    tokenize(sTemp, vSTemp);
    seed = stoi(vSTemp.back());
    cout << "seed " << seed << endl;

    getline(inFile, sTemp); 
    
    //v
    getline(inFile, sTemp);
    cout << "v " << sTemp << endl;
    tokenize(sTemp, vSTemp);
    numVars = stoi(vSTemp.back());
    cout << numVars << endl;

    //c
    getline(inFile, sTemp);
    cout << "c " << sTemp << endl;
    vSTemp.clear();
    tokenize(sTemp, vSTemp);
    numClauses = stoi(vSTemp.back());
    cout << numClauses << endl;

    getline(inFile, sTemp); 
    getline(inFile, sTemp); 

    cout << "vars " << sTemp << endl;
    //variables
    i = numVars;
    unsigned int count = 1;
    while(i > 0 ){
        getline(inFile, sTemp);
        vSTemp.clear();
        tokenize(sTemp, vSTemp);

        varInfo var;
        for(unsigned int b = 0; b <= vSTemp.size(); b++){
            if(b%2 == 1) {
                var.value = stod(vSTemp.at(b));
                variables.insert(pair<int, varInfo>(count, var));
            }
        }

        count++;
        i--;    
    }
    cout << "\n";

    cout << "variables size " << variables.size() << endl;
    // for(int leep = 1; leep < 4; leep++){
    //     cout << (variables.at(leep)).value << " ";
    // }
    // cout << "\n";

    getline(inFile, sTemp);
    getline(inFile, sTemp);
    for(i = 0; i < numClauses; i++){
        vSTemp.clear();
        getline(inFile, sTemp);
        tokenize(sTemp, vSTemp);
        vSTemp.pop_back();
        for(unsigned int j = 0; j < vSTemp.size(); j++){
            int num = stoi(vSTemp.at(j).c_str());
            //Adela's version: vITemp.push_back(num);
            //Son's version:
            vITemp.insert(num); 
            if(num > 0){
                ((variables.at(abs(num))).clauseMembers).insert(pair<int, int>(i, POSITIVE));
            }
            else{
                ((variables.at(abs(num))).clauseMembers).insert(pair<int, int>(i, NEGATIVE));
            }

        }

        //Adela
        //clauses.insert(pair<int, vector<int> >(i,vITemp));

        //Son
        clauses.insert(pair<int, vector<int> >(i,vITemp));
        vITemp.clear();
    }

    cout << "clauses size " << clauses.size() << endl;
    // map<int, vector <int> >::iterator it = clauses.begin();       
    // for(it = clauses.begin(); it!=clauses.end(); ++it){
    //     cout << it->first << ":";
    //     vector<int>::iterator iter;

    //     for(iter = (it->second).begin(); iter!=(it->second).end();++iter){
    //         cout << " " << (*iter);
    //     }
    //     cout << endl;
    // }

    // for(int leep = 1; leep < 4; leep++){
    //     map<int, int>::iterator it1 = ((variables.at(leep)).clauseMembers).begin();       
    //     for(it1 = ((variables.at(leep)).clauseMembers).begin(); it1!=((variables.at(leep)).clauseMembers).end(); it1++){
    //         cout << leep << " " << it1->first << " =>"<< it1->second << endl;
    //     }
    //     cout << "\n";
    // }
    // cout << "\n";



    inFile.close();
}

/***************************************************************************
 Function:  SOLVESSAT
 Inputs:    clauses, assignment, chance-var-probabilities
 Returns:   double
 Description:
            THE MAIN ALGORITHM IMPLEMENTATION
 ***************************************************************************/
double SOLVESSAT(){


    //check for pure CHOICE variable

    pair<bool, int> result;

    int v = 0;
    int value = 0;

    for (map<int, varInfo>::iterator it = variables.begin(); it != variables.end(); it++){
        v = it->first;
        result = isPureChoice(it->first);
        if(result.first == false) {
            continue;
        }
        else {
            varInfo savedInfo;
            savedInfo.value = variables[v].value;
            savedInfo.
            value = result.second;
            assign(v, value);
            updateClausesAndVariables(v, value);
            double probSSAT = SOLVESSAT();
            undoChanges();
            return probSSAT;
        }
    }
}

/***************************************************************************
 Function:  tokenize
 Inputs:    string and vector
 Returns:   vector
 Description:
            splits a string by whitespace
 ***************************************************************************/
void tokenize(string str, vector<string> &token_v){
    size_t start = str.find_first_not_of(' '), end=start;

    while(start != string::npos) {
        // Find next occurence of delimiter
        end = str.find(' ', start);
        // Push back the token found into vector
        token_v.push_back(str.substr(start, end-start));
        // Skip all occurences of the delimiter to find new start
        start = str.find_first_not_of(' ', end);
    }
}

 
/***************************************************************************
 Function:  isPureChoice
 Inputs:    int
 Returns:   True if the variable is a pure choice variable
 Description:
 ***************************************************************************/
pair<bool, int> isPureChoice(int variable) {
    varInfo info = variables[variable];
    if info.value != CHOICE_VALUE {
        return false;
    }

    vector<int>* clauseVect = &(info.clauseMembers);

    int clauseEntry = 0;
    int currentStatus = POSITIVE;
    bool switchSign = false;

    for(unsigned int i = 0; i < (*clauseVect).size(); i++) {
        clauseEntry = (*clauseVect)[i];
        if(currentStatus != variableSignInClause(clauseEntry, variable)) {
            if!(switchSign) {
                currentStatus = NEGATIVE;
                switchSign = true;
            }
            else {
                return pair<bool, int>(false, INVALID);
            }
        } 
    }

    return pair<bool, int>(true, currentStatus);
}

/***************************************************************************
 Function:  variableSignInClause
 Inputs:    int
 Returns:   int (positive or negative)
 Description:   indicates the sign of a variable within a given clause
 ***************************************************************************/
int variableSignInClause(int clauseEntry, int variable) {
    return variables[variable].clauseMembers[clauseEntry];
}

/***************************************************************************
 Function:  updateClausesAndVariables
 Inputs:    int
 Returns:   int (positive or negative)
 Description:   remove all satisfactory clauses after a given assignment
 ***************************************************************************/
void updateClausesAndVariables(int variable, int value, 
                                vector<int>* savedSATClause, 
                                vector<int>* savedFalseLiteralClause) {
    varInfo info = variables[variable];

    set<int>* clauseSet = &(info.clauseMembers);

    for(set<int>::iterator it = (*clauseSet).begin(); it != (*clauseSet).end(); ) {
        
        int clauseEntry = it->first;
        int clauseEntryValue = it->second;

        //remove clauses that have the true-value variables
        if (clauseEntryValue == value){
            (*saveSATClause).push_back(clauseEntry);
            clauses.erase(clauseEntry);
        }

        //remove the false variable in the clauses
        else {
            clauses[clauseEntry].erase(variable);
        }
    }
}

/***************************************************************************
 Function:  assign
 Inputs:    int
 Returns:   int (positive or negative)
 Description:   indicates the sign of a variable within a given clause
 ***************************************************************************/
void assign(int variable, int value) {
    assignment[variable] = value;
    //variables.erase(variable);
}

/***************************************************************************
 Function:  undoChanges
 Inputs:    
 Returns:   
 Description:   undo changes made before call to SOLVESSAT
 ***************************************************************************/
void undoChanges(){

}
/****** END OF FILE **********************************************************/
