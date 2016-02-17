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

using namespace std;

/***************************************************************************/
/* CONSTANTS */
static const double CHOICE_VALUE = -1.0;
static const int POSITIVE = 1;
static const int NEGATIVE = -1;
static const int INVALID = 0; 

/***************************************************************************/

/***************************************************************************/
/* structs */
typedef struct varInfo{
    double value;               // the value of the variables
    vector <int> clauseMembers;  // the indexes of the clauses the var is in        
} varInfo;

//Son's comment on varInfo: I think we should turn clauseMembers into a map
//of <int, int> where the first one is the index of the clause and the value
//indicates what its assignment is in that corresponding clause. This modification
//would help a lot of with updating clauses when we assign a value to a variable

/***************************************************************************/
/* globals variables */

//probably will not use these vars...
int maximumClauseLength;
int minimumClauseLength;
double averageClauseLength;
int seed;

int numVars;
int numClauses;

map <int, varInfo> variables;
map <int, vector<int> > clauses;
map <int, int> assignment;

vector <vector <int> > solution;

double successProbability;
double solutionTime;

/***************************************************************************/
/* functions */
double SOLVESSAT(clauses, chace-var-probabilities);
void readFile(string input);
void tokenize(string str, vector<string> &token_v);
pair<bool, int> isPureChoice(int variable);
int variableSignInClause(int clauseEntry);
void updateSATclauses(int variable, int value);
void assign(int variable, int value);

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
    vector<int> vITemp;
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
                variables.insert(pair<int,varInfo>(count, var));
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
            vITemp.push_back(num);
            ((variables.at(abs(num))).clauseMembers).push_back(i);
        }
        clauses.insert(pair<int,vector<int> >(i,vITemp));
        vITemp.clear();
    }

    cout << "clauses size " << clauses.size() << endl;
    // map<int, vector <int> >::iterator it = clauses.begin();       
    // for(it = clauses.begin(); it!=clauses.end(); ++it){
    //         cout << it->first << ":";
    //     vector<int>::iterator iter;
    //     for(iter = (it->second).begin(); iter!=(it->second).end();++iter){
    //         cout << " " << (*iter);
    //     }
    //     cout << endl;
    // }

    // for(int leep = 1; leep < 4; leep++){
    //     for (int sneep = 0; sneep < ((variables.at(leep)).clauseMembers).size(); sneep++){
    //         cout << ((variables.at(leep)).clauseMembers).at(sneep) << " ";
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
double SOLVESSAT(clauses, chace-var-probabilities){


    //check for pure CHOICE variable

    map<int, varInfo>::iterator it = variables.begin();

    pair<bool, int> result;

    int v = 0;
    int value = 0;

    for (map<int, varInfo>::iterator it = variables.begin(); it != variables.end(); it++){
        v = it->first;
        result = isPureChoice(it->first);
        if (result.first == false){
            continue;
        }
        else {
            value = result.second;
            assign(v, value);
            updateSATclauses(v, value);
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

    while (start != string::npos){
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

    for (unsigned int i = 0; i < (*clauseVect).size(); i++){
        clauseEntry = (*clauseVect)[i];
        if (currentStatus != variableSignInClause(clauseEntry, variable)){
            if !(switchSign) {
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

    vector<int>* clause = clauses[clauseEntry];

    bool foundSign = false;

    for (unsigned int i = 0; i < (*clause).size(); i++){
        if (abs((*clause)[i]) == variable) {
            return (*clause[i] / variable);
        } 
    }

    return INVALID;
}

/***************************************************************************
 Function:  updateSATclauses
 Inputs:    int
 Returns:   int (positive or negative)
 Description:   indicates the sign of a variable within a given clause
 ***************************************************************************/
void updateSATclauses(int variable, int value) {

    varInfo info = variables[variable];

    vector<int>& clauseVect = &(info.clauseMembers);

    int clauseEntry = 0;

    for (unsigned int i = 0; i < (*clauseVect).size(); i++){
        clauseEntry = (*clauseVect)[i];

        //clause is satisfied
        if (variableSignInClause(clauseEntry, variable) == value){

            vector<int>* clause = clauses[clauseEntry];

            for (unsigned int i = 0; i < (*clause).size(); i++){
                varInfo currInfo = variables[i];

                //this should be much easier with a map! Otherwise I will have to
                //go through the whole list
                //need to erase the clause that is stored in the varInfo struct.vector/(map)
                currInfo.clauseMembers.erase(currInfo.clauseMembers.begin() + variable);
            }
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

/****** END OF FILE **********************************************************/
