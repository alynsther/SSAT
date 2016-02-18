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
    double quantifier;                   // choice/chance (probability if chance)
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

//Son: I changed the data structure of clauses to map <int, set> because if 
//we want to remove a literal from a clause we wouldn't have to go through
//the whole clause but we can access it in constant time using a set
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
void updateClausesAndVariables(int variable, int value, 
                                map<int, set<int> >* savedSATClausesPtr, 
                                vector<int>* savedFalseLiteralClausePtr);
void assign(int variable, int value);
void undoChanges(int variable, varInfo* savedInfo, map<int, set<int> >* savedSATClausesPtr, 
                    vector<int>* savedFalseLiteralClausePtr);
int unassigned_var();

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

    //solving SSAT
    double start, end, solutionTime;
    start = clock();
    //SOLVESSAT();
    end = clock();
    solutionTime = double(start-end);
    
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
    set<int> vITemp;

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
                var.quantifier = stod(vSTemp.at(b));
                variables.insert(pair<int, varInfo>(count, var));
            }
        }

        count++;
        i--;    
    }
    cout << "\n";

    cout << "variables size " << variables.size() << endl;
    // for(int leep = 1; leep < 4; leep++){
    //     cout << (variables.at(leep)).quantifier << " ";
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
        clauses.insert(pair<int, set<int> >(i,vITemp));
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

    if(clauses.empty()){
        return 1.0;
    }
    
    pair<bool, int> result;
    varInfo savedInfo;
    map<int, set <int> > savedSATClauses;
    vector<int> savedFalseLiteralClause;
    
    int v = 0;
    
    //check for unit clauses
    for(map<int, set<int> >::iterator it = clauses.begin(); it != clauses.end(); it++){
        if(it->second.size() == 1){
            bool isPositive = true;
            set<int>::iterator se = it->second.begin();
            v = *se;
            if(v < 0){
                isPositive = false;
                v *= -1;
            }
            updateClausesAndVariables(v, isPositive, &savedSATClauses, &savedFalseLiteralClause);
            double probSAT = SOLVESSAT();
            undoChanges(v, &savedInfo, &savedSATClauses, &savedFalseLiteralClause);
            if(variables[v].quantifier == -1.0){
                return probSAT;
            }
            if(!isPositive){
                //return probSAT * prob[v=FALSE];
            }
            //return probSAT * prob[v=TRUE];
        }
    }
    
    //begin checking for pure CHOICE variable
    int value = 0;

    for (map<int, varInfo>::iterator it = variables.begin(); it != variables.end(); it++){
        v = it->first;
        result = isPureChoice(it->first);
        if(result.first == false) {
            continue;
        }
        else {
            savedInfo.quantifier = variables[v].quantifier;
            savedInfo.clauseMembers = variables[v].clauseMembers;
            value = result.second;
            assign(v, value);
            updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause);
            double probSSAT = SOLVESSAT();
            undoChanges(v, &savedInfo, &savedSATClauses, &savedFalseLiteralClause);
            return probSSAT;
        }
    }
    //end checking for pure CHOICE variable

    //begin choosing the next variable to assign value (no UCP or PCE)

    v = unassigned_var();

    //end choosing the next variable to assign value (no UCP or PCE)

    return 0.0;
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
    if (info.quantifier != CHOICE_VALUE) {
        return pair<bool, int>(false, INVALID);
    }

    map<int, int>* clauseInfo = &(info.clauseMembers);
    int status = POSITIVE;
    bool signSwitch = false;

    for (map<int, int>::iterator it = (*clauseInfo).begin(); it != (*clauseInfo).end(); it++){
        if (!signSwitch) {
            status = it->second;
            signSwitch = true;
        }
        else {
            if (status != it->second) {
                return pair<bool, int>(false, INVALID);
            }
        }
    }

    return pair<bool, int>(true, status);
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
                                map<int, set<int> >* savedSATClausesPtr, 
                                vector<int>* savedFalseLiteralClausePtr) {
    varInfo info = variables[variable];

    map<int, int>* clauseSet = &(info.clauseMembers);

    for(map<int, int>::iterator it = (*clauseSet).begin(); it != (*clauseSet).end(); ) {
        
        int clauseEntry = it->first;
        int clauseEntryValue = it->second;

        //remove clauses that have the true-value variables
        if (clauseEntryValue == value){
            (*savedSATClausesPtr).insert(pair<int, set<int> >(clauseEntry, clauses[clauseEntry]));
            clauses.erase(clauseEntry);
        }

        //remove the false variable in the clauses
        else {
            (*savedFalseLiteralClausePtr).push_back(clauseEntry);
            clauses[clauseEntry].erase(variable);
        }
    }

    variables.erase(variable);
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
 Function:  unassigned_var
 Inputs:    
 Returns:   int (what variable)
 Description:   return the next variable in the same block with no assigned value
 ***************************************************************************/
int unassigned_var(){
    return variables.begin()->first;
}

/***************************************************************************
 Function:  undoChanges
 Inputs:    
 Returns:   
 Description:   undo changes made before call to SOLVESSAT
 ***************************************************************************/
void undoChanges(int variable, varInfo* savedInfo, map<int, set<int> >* savedSATClausesPtr, 
                    vector<int>* savedFalseLiteralClausePtr){

    //put back the assigned variable to variables list
    variables.insert(pair<int, varInfo>(variable, *savedInfo));

    //put back SAT clauses to clauses list
    for (map<int, set<int> >::iterator it = (*savedSATClausesPtr).begin(); it != (*savedSATClausesPtr).end(); it++){
        clauses[it->first] = it->second;
    }

    //put back FALSE literals to their original clauses
    for (vector<int>::iterator it = (*savedFalseLiteralClausePtr).begin(); it != (*savedFalseLiteralClausePtr).end(); it++){
        clauses[*it].insert(variable);
    }
}
/****** END OF FILE **********************************************************/
