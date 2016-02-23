/*****************************************************************************
 File:   ssat.cc
 Author: Adela Yang, Venecia Xu, Son Ngo
 Date:   February 2016

 Description:
 Implement a DPLL-based stochastic satisfiability (SSAT) solver to test the
    effectiveness of unit clause propagation, pure variable elimination, and 
    three different splitting heuristics
 
 Running instructions:
 Run this program with two arguments on the command line 
    the input file and the algorithm
    
    g++ -Wall -o ssat ssat.cc [file name]
    ./ssat [file name]

    Running on dover: g++ -std=c++11 -Wall -o ssat ssat.cc [file name]
    ./ssat [file name]


NOTES FROM ADELA FOR SPLITTING HEURISTIC:
https://goo.gl/Ax0bpc
On page 549+, it lists 6 splitting heuristics and their performances

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
#include <climits>
#include <algorithm>
#include <map>
#include <set>
#include <ctime>

using namespace std;

/***************************************************************************/
/* constants */
static const double CHOICE_VALUE = -1.0;
static const double SATISFIED = 1.0;
static const int POSITIVE = 1;
static const int NEGATIVE = -1;
static const int INVALID = 0; 
static const int UNIT_SIZE = 1;
static const int FAILURE = 0.0;
static const int SUCCESS = 1.0;

/***************************************************************************/
/* structs */
typedef struct varInfo{
    double quantifier;              // choice/chance (probability if chance)
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
map <int, set<int> > clauses;
map <int, int> assignment;

bool UNSATclauseExists = false;
bool headHasValue = false;

//A: we also have to record the following for every run
//S: what do we need this for?????
int numUnitClausePropagations;
int numPureVariableEliminations;
int numVariableSplits;
double percentageVariableSplits; // out of 2^n-1 splits

/***************************************************************************/
/* functions */
<<<<<<< HEAD
double SOLVESSAT(int splittingHeuristic);
double UCPSOLVESSAT(int splittingHeuristic);
double PVESOLVESSAT(int splittingHeuristic);
double UCPPVESOLVESSAT(int splittingHeuristic);
=======
double SOLVESSAT();
double UCPSOLVESSAT();
double PVESOLVESSAT();
double UCPPVESOLVESSAT();
double HEURISTICSOLVESSAT();
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
void readFile(string input);
void tokenize(string str, vector<string> &token_v);
pair<bool, int> isPureChoice(int variable);

int variableSignInClause(int clauseEntry, int variable);
void updateClausesAndVariables(int variable, int value, 
                                map<int, set<int> >* savedSATClausesPtr, 
                                vector<int>* savedFalseLiteralClausePtr,
                                map<int, double>* savedInactiveVariables);
void assign(int variable, int value);
void undoChanges(int variable, int value, varInfo* savedInfo, 
                map<int, set<int> >* savedSATClausesPtr, 
                vector<int>* savedFalseLiteralClausePtr,
                map<int, double>* savedInactiveVariables);
int unassigned_var();
void printVariables();
void printClauses();
int randomSH();
int maximumSH();
//int undeterminedSH();
vector<int> helperSH();
void resultsPrint(int num, string name);

/*****************************************************************************
 Function:  main
 Inputs:    argv
 Returns:   nothing
 Description:
 *****************************************************************************/
int main(int argc, char* argv[]) {

    //open the file for reading
<<<<<<< HEAD
    string input = argv[1];
    readFile(input);
    // double start, end, solutionTime;
    vector <int> splittingHeuristic;
    splittingHeuristic.push_back(0); //random
    splittingHeuristic.push_back(1); //maximum


    vector <int>::iterator it;
    for(it=splittingHeuristic.begin(); it!= splittingHeuristic.end(); it++) {
        switch ((*it)){
            case 0:
                resultsPrint(0, "random");
                break;
            case 1:
                resultsPrint(1, "maximum");
                break;
            default:
                resultsPrint(-1, "vos");
                break;
        }
    }

    // //UCP & PVE
    // start = clock();
    // cout << "RESULT OF UCPPVESOLVESSAT " << UCPPVESOLVESSAT(splittingHeuristic) << endl;
    // end = clock();
    // solutionTime = double(end-start)/CLOCKS_PER_SEC;
    // cout << "Algorithm Running Time with UCP & PVE: " << solutionTime << endl;

    // //UCP ONLY
    // start = clock();
    // cout << "RESULT OF UCPSOLVESSAT " << UCPSOLVESSAT(splittingHeuristic) << endl;
    // end = clock();
    // solutionTime = double(end-start)/CLOCKS_PER_SEC;
    // cout << "Algorithm Running Time with UCP only: " << solutionTime << endl;

    // //PVE ONLY
    // start = clock();
    // cout << "RESULT OF PVESOLVESSAT " << PVESOLVESSAT(splittingHeuristic) << endl;
    // end = clock();
    // solutionTime = double(end-start)/CLOCKS_PER_SEC;
    // cout << "Algorithm Running Time with PVE only : " << solutionTime << endl;

    // //NO PVE OR UCP
    // start = clock();
    // cout << "RESULT OF PVESOLVESSAT " << SOLVESSAT(splittingHeuristic) << endl;
    // end = clock();
    // solutionTime = double(end-start)/CLOCKS_PER_SEC;
    // cout << "Algorithm Running Time with no UCP or PVE: " << solutionTime << endl;
    
    return 0;
}

/*****************************************************************************
 Function:  resultsPrint
 Inputs:    number and name of splitting heuristic
 Returns:   nothing
 Description:   prints information to terminal and files
 *****************************************************************************/
void resultsPrint(int num, string name) {
    double start, end, solutionTime;
    ofstream file;

    //UCP & PVE
    start = clock();
    cout << "RESULT OF UCPPVESOLVESSAT" + name + " " << UCPPVESOLVESSAT(num) << endl;
    end = clock();
    solutionTime = double(end-start)/CLOCKS_PER_SEC;
    cout << "Algorithm Running Time with UCP & PVE: " + name + " " << solutionTime << endl;

    file.open ("ucppvesolvessat" + name + ".csv");
    file << "solutionTime" << "," << solutionTime << "\n";
    file << "numUnitClausePropagations" << "," << numUnitClausePropagations << "\n";
    file << "numPureVariableEliminations" << "," << numPureVariableEliminations << "\n";
    file << "numVariableSplits" << "," << numVariableSplits << "\n";
    file << "percentageVariableSplits" << "," << percentageVariableSplits << "\n"; 
    file.close();

    //UCP ONLY
    start = clock();
    cout << "RESULT OF UCPSOLVESSAT " + name + " " << UCPSOLVESSAT(num) << endl;
    end = clock();
    solutionTime = double(end-start)/CLOCKS_PER_SEC;
    cout << "Algorithm Running Time with UCP only: " + name + " " << solutionTime << endl;

    file.open ("ucpsolvessat" + name + ".csv");
    file << "solutionTime" << "," << solutionTime << "\n";
    file << "numUnitClausePropagations" << "," << numUnitClausePropagations << "\n";
    file << "numPureVariableEliminations" << "," << numPureVariableEliminations << "\n";
    file << "numVariableSplits" << "," << numVariableSplits << "\n";
    file << "percentageVariableSplits" << "," << percentageVariableSplits << "\n"; 
    file.close();

    //PVE ONLY
    start = clock();
    cout << "RESULT OF PVESOLVESSAT " + name + " " << PVESOLVESSAT(num) << endl;
    end = clock();
    solutionTime = double(end-start)/CLOCKS_PER_SEC;
    cout << "Algorithm Running Time with PVE only : " + name + " " << solutionTime << endl;

    file.open ("pvesolvessat" + name + ".csv");
    file << "solutionTime" << "," << solutionTime << "\n";
    file << "numUnitClausePropagations" << "," << numUnitClausePropagations << "\n";
    file << "numPureVariableEliminations" << "," << numPureVariableEliminations << "\n";
    file << "numVariableSplits" << "," << numVariableSplits << "\n";
    file << "percentageVariableSplits" << "," << percentageVariableSplits << "\n"; 
    file.close();

    //NO PVE OR UCP
    start = clock();
    cout << "RESULT OF SOLVESSAT " + name + " " << SOLVESSAT(num) << endl;
    end = clock();
    solutionTime = double(end-start)/CLOCKS_PER_SEC;
    cout << "Algorithm Running Time with no UCP or PVE: " + name + " " << solutionTime << endl;

    file.open ("solvessat" + name + ".csv");
    file << "solutionTime" << "," << solutionTime << "\n";
    file << "numUnitClausePropagations" << "," << numUnitClausePropagations << "\n";
    file << "numPureVariableEliminations" << "," << numPureVariableEliminations << "\n";
    file << "numVariableSplits" << "," << numVariableSplits << "\n";
    file << "percentageVariableSplits" << "," << percentageVariableSplits << "\n"; 
    file.close();
=======

    string input = argv[1];
    readFile(input);
    double start, end, solutionTime;

    //UCP & PVE
    start = clock();
    cout << "RESULT OF UCPPVESOLVESSAT " << UCPPVESOLVESSAT() << endl;
    end = clock();
    solutionTime = double(end-start)/CLOCKS_PER_SEC;
    cout << "Algorithm Running Time with UCP & PVE: " << solutionTime << endl;

    //UCP ONLY
    start = clock();
    cout << "RESULT OF UCPSOLVESSAT " << UCPSOLVESSAT() << endl;
    end = clock();
    solutionTime = double(end-start)/CLOCKS_PER_SEC;
    cout << "Algorithm Running Time with UCP only: " << solutionTime << endl;

    //PVE ONLY
    start = clock();
    cout << "RESULT OF PVESOLVESSAT " << PVESOLVESSAT() << endl;
    end = clock();
    solutionTime = double(end-start)/CLOCKS_PER_SEC;
    cout << "Algorithm Running Time with PVE only : " << solutionTime << endl;

    //NO PVE OR UCP
    start = clock();
    cout << "RESULT OF PVESOLVESSAT " << SOLVESSAT() << endl;
    end = clock();
    solutionTime = double(end-start)/CLOCKS_PER_SEC;
    cout << "Algorithm Running Time with no UCP or PVE: " << solutionTime << endl;

    return 0;
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
}

/***************************************************************************
 Function:  UCPPVESOLVESSAT
 Inputs:    nothing
 Returns:   double
 Description:
            THE MAIN ALGORITHM IMPLEMENTATION
            clauses, assignment, chance-var-probabilities
 ***************************************************************************/
<<<<<<< HEAD
double UCPPVESOLVESSAT(int splittingHeuristic){
=======
double UCPPVESOLVESSAT(){
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
    if(clauses.empty()){
        return SUCCESS;
    }

    if (UNSATclauseExists || variables.empty() == true) {
        return FAILURE;
    }
    
    pair<bool, int> result;
    varInfo savedInfo;
    map<int, set <int> > savedSATClauses;
    vector<int> savedFalseLiteralClause;
    map<int, double> savedInactiveVariables;
    
    int v = 0;
    int value = 0;

    //BEGIN UNIT CLAUSES PROPAGATION
    for(map<int, set<int> >::iterator it = clauses.begin(); it != clauses.end(); it++){

        if(it->second.size() == UNIT_SIZE){
            value = POSITIVE;
            set<int>::iterator se = it->second.begin();
            v = *se;

            if(v < 0){
                value = NEGATIVE;
                v *= NEGATIVE;
            }

            assign(v, value);

            savedInfo.quantifier = variables[v].quantifier;
            savedInfo.clauseMembers = variables[v].clauseMembers;
            
            updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);

<<<<<<< HEAD
            double probSAT = UCPPVESOLVESSAT(splittingHeuristic);
=======
            double probSAT = UCPPVESOLVESSAT();
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
            
            undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
            
            if(variables[v].quantifier == CHOICE_VALUE){
                return probSAT;
            }
            
            if(value == NEGATIVE){
                return probSAT * (1 - variables[v].quantifier);
            }
            
            return probSAT * variables[v].quantifier;
        }
    }
    //END UNIT CLAUSES PROPAGATION
    
    //BEGIN PURE CHOICE ELIMINATION
    for (map<int, varInfo>::iterator it = variables.begin(); it != variables.end(); it++){

        result = isPureChoice(it->first);
        v = it->first;        

        if(result.first == false) {
            continue;
        } 
        else {
            savedInfo.quantifier = variables[v].quantifier;
            savedInfo.clauseMembers = variables[v].clauseMembers;
            value = result.second;

            assign(v, value);

            updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);

<<<<<<< HEAD
            double probSSAT = UCPPVESOLVESSAT(splittingHeuristic);
=======
            double probSSAT = UCPPVESOLVESSAT();
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172

            undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);

            return probSSAT;
        }
    }
    //END PURE CHOICE ELIMINATION

    //BEGIN VARIABLE SPLITS

<<<<<<< HEAD
    switch (splittingHeuristic){
        case 0: 
            v = randomSH();
            break;
        case 1:
            v = maximumSH();
            break;
        default:
            v = unassigned_var();
            break;
    }
    // v = unassigned_var();
=======
    v = unassigned_var();
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172

    if (v == INVALID) {
        cout << "The variable is invalid" << endl;
        return FAILURE;
    }
    
    //try setting v to FALSE

    assign(v, NEGATIVE);
    value = NEGATIVE;

    
    savedInfo.quantifier = variables[v].quantifier;
    savedInfo.clauseMembers = variables[v].clauseMembers;
    
    updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
    
<<<<<<< HEAD
    double probSATWithFalse = UCPPVESOLVESSAT(splittingHeuristic);
=======
    double probSATWithFalse = UCPPVESOLVESSAT();
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
    
    undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);

    //end setting v to FALSE

    
    //try setting v to TRUE

    assign(v, POSITIVE);
    value = POSITIVE;

    savedInfo.quantifier = variables[v].quantifier;
    savedInfo.clauseMembers = variables[v].clauseMembers;
    
    updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
    
<<<<<<< HEAD
    double probSATWithTrue = UCPPVESOLVESSAT(splittingHeuristic);
=======
    double probSATWithTrue = UCPPVESOLVESSAT();
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
    
    undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);

    //end setting v to TRUE

    if(variables[v].quantifier == CHOICE_VALUE){
        return max(probSATWithFalse, probSATWithTrue);
    }
    
    return probSATWithFalse * (1 - variables[v].quantifier) + probSATWithTrue * variables[v].quantifier;

    //END VARIABLE SPLITS
}

/***************************************************************************
 Function:  UCPSOLVESSAT
 Inputs:    nothing
 Returns:   double
 Description:
            THE MAIN ALGORITHM IMPLEMENTATION 
            using ONLY UCP
 ***************************************************************************/
<<<<<<< HEAD
double UCPSOLVESSAT(int splittingHeuristic){
=======
double UCPSOLVESSAT(){
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
    if(clauses.empty()){
        return SUCCESS;
    }

    if (UNSATclauseExists || variables.empty() == true) {
        return FAILURE;
    }
    
    pair<bool, int> result;
    varInfo savedInfo;
    map<int, set <int> > savedSATClauses;
    vector<int> savedFalseLiteralClause;
    map<int, double> savedInactiveVariables;
    
    int v = 0;
    int value = 0;

    //BEGIN UNIT CLAUSES PROPAGATION
    for(map<int, set<int> >::iterator it = clauses.begin(); it != clauses.end(); it++){

        if(it->second.size() == UNIT_SIZE){
            value = POSITIVE;
            set<int>::iterator se = it->second.begin();
            v = *se;

            if(v < 0){
                value = NEGATIVE;
                v *= NEGATIVE;
            }

            assign(v, value);
<<<<<<< HEAD

            savedInfo.quantifier = variables[v].quantifier;
            savedInfo.clauseMembers = variables[v].clauseMembers;
            
            updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);

            double probSAT = UCPSOLVESSAT(splittingHeuristic);
            
            undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
            
            if(variables[v].quantifier == CHOICE_VALUE){
                return probSAT;
            }
            
            if(value == NEGATIVE){
                return probSAT * (1 - variables[v].quantifier);
            }
            
            return probSAT * variables[v].quantifier;
        }
    }
    //END UNIT CLAUSES PROPAGATION

    //BEGIN VARIABLE SPLITS

    switch (splittingHeuristic){
        case 0: 
            v = randomSH();
            break;
        case 1:
            v = maximumSH();
            break;
        default:
            v = unassigned_var();
            break;
    }
    // v = unassigned_var();

    if (v == INVALID) {
        cout << "The variable is invalid" << endl;
        return FAILURE;
    }
    
    //try setting v to FALSE

=======

            savedInfo.quantifier = variables[v].quantifier;
            savedInfo.clauseMembers = variables[v].clauseMembers;
            
            updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);

            double probSAT = UCPSOLVESSAT();
            
            undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
            
            if(variables[v].quantifier == CHOICE_VALUE){
                return probSAT;
            }
            
            if(value == NEGATIVE){
                return probSAT * (1 - variables[v].quantifier);
            }
            
            return probSAT * variables[v].quantifier;
        }
    }
    //END UNIT CLAUSES PROPAGATION

    //BEGIN VARIABLE SPLITS

    v = unassigned_var();

    if (v == INVALID) {
        cout << "The variable is invalid" << endl;
        return FAILURE;
    }
    
    //try setting v to FALSE

>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
    assign(v, NEGATIVE);
    value = NEGATIVE;

    
    savedInfo.quantifier = variables[v].quantifier;
    savedInfo.clauseMembers = variables[v].clauseMembers;
    
    updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
    
<<<<<<< HEAD
    double probSATWithFalse = UCPSOLVESSAT(splittingHeuristic);
    
    undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);

    //end setting v to FALSE

    
    //try setting v to TRUE

    assign(v, POSITIVE);
    value = POSITIVE;

=======
    double probSATWithFalse = UCPSOLVESSAT();
    
    undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);

    //end setting v to FALSE

    
    //try setting v to TRUE

    assign(v, POSITIVE);
    value = POSITIVE;

>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
    savedInfo.quantifier = variables[v].quantifier;
    savedInfo.clauseMembers = variables[v].clauseMembers;
    
    updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
    
<<<<<<< HEAD
    double probSATWithTrue = UCPSOLVESSAT(splittingHeuristic);
=======
    double probSATWithTrue = UCPSOLVESSAT();
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
    
    undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);

    //end setting v to TRUE

    if(variables[v].quantifier == CHOICE_VALUE){
        return max(probSATWithFalse, probSATWithTrue);
    }
    
    return probSATWithFalse * (1 - variables[v].quantifier) + probSATWithTrue * variables[v].quantifier;

    //END VARIABLE SPLITS
}

/***************************************************************************
 Function:  PVESOLVESSAT
 Inputs:    nothing
 Returns:   double
 Description:
            THE MAIN ALGORITHM IMPLEMENTATION 
            using ONLY UCP
 ***************************************************************************/
<<<<<<< HEAD
double PVESOLVESSAT(int splittingHeuristic){
=======
double PVESOLVESSAT(){
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
    if(clauses.empty()){
        return SUCCESS;
    }

    if (UNSATclauseExists || variables.empty() == true) {
        return FAILURE;
    }
    
    pair<bool, int> result;
    varInfo savedInfo;
    map<int, set <int> > savedSATClauses;
    vector<int> savedFalseLiteralClause;
    map<int, double> savedInactiveVariables;
    
    int v = 0;
    int value = 0;
    
    //BEGIN PURE CHOICE ELIMINATION
    for (map<int, varInfo>::iterator it = variables.begin(); it != variables.end(); it++){

        result = isPureChoice(it->first);
        v = it->first;        

        if(result.first == false) {
            continue;
        } 
        else {
            savedInfo.quantifier = variables[v].quantifier;
            savedInfo.clauseMembers = variables[v].clauseMembers;
            value = result.second;

            assign(v, value);

            updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);

<<<<<<< HEAD
            double probSSAT = PVESOLVESSAT(splittingHeuristic);
=======
            double probSSAT = PVESOLVESSAT();
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172

            undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);

            return probSSAT;
        }
    }
    //END PURE CHOICE ELIMINATION
<<<<<<< HEAD

    //BEGIN VARIABLE SPLITS

    switch (splittingHeuristic){
        case 0: 
            v = randomSH();
            break;
        case 1:
            v = maximumSH();
            break;
        default:
            v = unassigned_var();
            break;
    }
    // v = unassigned_var();

=======

    //BEGIN VARIABLE SPLITS

    v = unassigned_var();

>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
    if (v == INVALID) {
        cout << "The variable is invalid" << endl;
        return FAILURE;
    }
    
    //try setting v to FALSE

    assign(v, NEGATIVE);
    value = NEGATIVE;

    
    savedInfo.quantifier = variables[v].quantifier;
    savedInfo.clauseMembers = variables[v].clauseMembers;
    
    updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
    
<<<<<<< HEAD
    double probSATWithFalse = PVESOLVESSAT(splittingHeuristic);
=======
    double probSATWithFalse = PVESOLVESSAT();
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
    
    undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);

    //end setting v to FALSE

    
    //try setting v to TRUE

    assign(v, POSITIVE);
    value = POSITIVE;

    savedInfo.quantifier = variables[v].quantifier;
    savedInfo.clauseMembers = variables[v].clauseMembers;
    
    updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
    
<<<<<<< HEAD
    double probSATWithTrue = PVESOLVESSAT(splittingHeuristic);
=======
    double probSATWithTrue = PVESOLVESSAT();
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
    
    undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);

    //end setting v to TRUE

    if(variables[v].quantifier == CHOICE_VALUE){
        return max(probSATWithFalse, probSATWithTrue);
    }
    
    return probSATWithFalse * (1 - variables[v].quantifier) + probSATWithTrue * variables[v].quantifier;

    //END VARIABLE SPLITS
}

/***************************************************************************
 Function:  SOLVESSAT
 Inputs:    nothing
 Returns:   double
 Description:
            THE MAIN ALGORITHM IMPLEMENTATION
            clauses, assignment, chance-var-probabilities
 ***************************************************************************/
<<<<<<< HEAD
double SOLVESSAT(int splittingHeuristic){
=======
double SOLVESSAT(){
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
    if(clauses.empty()){
        return SUCCESS;
    }

    if (UNSATclauseExists || variables.empty() == true) {
        return FAILURE;
    }
    
    pair<bool, int> result;
    varInfo savedInfo;
    map<int, set <int> > savedSATClauses;
    vector<int> savedFalseLiteralClause;
    map<int, double> savedInactiveVariables;
    
    int v = 0;
    int value = 0;

    //BEGIN VARIABLE SPLITS

<<<<<<< HEAD
    switch (splittingHeuristic){
        case 0: 
            v = randomSH();
            break;
        case 1:
            v = maximumSH();
            break;
        default:
            v = unassigned_var();
            break;
    }
    // v = unassigned_var();
=======
    v = unassigned_var();
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172

    if (v == INVALID) {
        cout << "The variable is invalid" << endl;
        return FAILURE;
    }
    
    //try setting v to FALSE

    assign(v, NEGATIVE);
    value = NEGATIVE;

    
    savedInfo.quantifier = variables[v].quantifier;
    savedInfo.clauseMembers = variables[v].clauseMembers;
    
    updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
    
<<<<<<< HEAD
    double probSATWithFalse = SOLVESSAT(splittingHeuristic);
=======
    double probSATWithFalse = SOLVESSAT();
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
    
    undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);

    //end setting v to FALSE

    //try setting v to TRUE

    assign(v, POSITIVE);
    value = POSITIVE;

    savedInfo.quantifier = variables[v].quantifier;
    savedInfo.clauseMembers = variables[v].clauseMembers;
    
    updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
    
<<<<<<< HEAD
    double probSATWithTrue = SOLVESSAT(splittingHeuristic);
=======
    double probSATWithTrue = SOLVESSAT();
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
    
    undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);

    //end setting v to TRUE

    if(variables[v].quantifier == CHOICE_VALUE){
        return max(probSATWithFalse, probSATWithTrue);
    }
    
    return probSATWithFalse * (1 - variables[v].quantifier) + probSATWithTrue * variables[v].quantifier;

    //END VARIABLE SPLITS
}

/***************************************************************************
 Function:  updateClausesAndVariables
 Inputs:    int
 Returns:   int (positive or negative)
 Description:   remove all satisfactory clauses after a given assignment
 ***************************************************************************/
void updateClausesAndVariables(int variable, int value, 
                                map<int, set<int> >* savedSATClausesPtr, 
                                vector<int>* savedFalseLiteralClausePtr,
                                map<int, double>* savedInactiveVariables) {
    varInfo* info = &(variables[variable]);
    map<int, int>* clauseSet = &((*info).clauseMembers);

    //cout << "UPDATING VARIABLES AND CLAUSES WITH " << variable << " of value " << value << endl;

    for(map<int, int>::iterator it = (*clauseSet).begin(); it != (*clauseSet).end(); it++) {
        int clauseEntry = it->first;
        int clauseEntryValue = it->second;

        //remove clauses that have the true-value variables
        if (clauseEntryValue == value){

            //save true clause for undochanges
            (*savedSATClausesPtr).insert(pair<int, set<int> >(clauseEntry, clauses[clauseEntry]));  
            
            //going through the satisfied clause to update its variables' clauseMembers
            for (set<int>::iterator iter = clauses[clauseEntry].begin(); iter != clauses[clauseEntry].end(); iter++){
     
                int removedVar = abs(*iter);

                //skip the being examined variable because we will delete them from
                //variables map later
                if (removedVar == variable){
                    continue;
                }

                //if variable we need to update is still active (i.e in the variables map) then remove the 
                //clauseEntry from that variable
                if (variables.find(removedVar) != variables.end()){
                    variables[removedVar].clauseMembers.erase(clauseEntry);

                    //if the variable after update is in no active clause then remove that variable from variables map
                    if (variables[removedVar].clauseMembers.empty() == true){
                        (*savedInactiveVariables).insert(pair<int, double>(removedVar, variables[removedVar].quantifier));
                        variables.erase(removedVar);
                    }
                }

            }

            //clauseEntry becomes inactive, removed from clauses
            clauses.erase(clauseEntry);
        }

        //remove the falsely assigned variable in the clauses
        else {
            (*savedFalseLiteralClausePtr).push_back(clauseEntry);
            clauses[clauseEntry].erase(NEGATIVE * value * variable);

            if (clauses[clauseEntry].empty() == true){
                UNSATclauseExists = true;
            }
        }
    }

    //variable becomes inactive, removed from variables map
    variables.erase(variable);
}


/***************************************************************************
 Function:  undoChanges
 Inputs:    
 Returns:   
 Description:   undo changes made before call to SOLVESSAT
 ***************************************************************************/
void undoChanges(int variable, int value, varInfo* savedInfo, 
                map<int, set<int> >* savedSATClausesPtr, 
                vector<int>* savedFalseLiteralClausePtr,
                map<int, double>* savedInactiveVariables){

    //put back the assigned variable to variables list
    for (map<int, double>:: iterator it = (*savedInactiveVariables).begin(); it != (*savedInactiveVariables).end();){
        variables[it->first].quantifier = it->second;
        (*savedInactiveVariables).erase(it++);
    }

    variables[variable].quantifier = (*savedInfo).quantifier;
    variables[variable].clauseMembers = (*savedInfo).clauseMembers;

    //put back SAT clauses to clauses list
    for (map<int, set<int> >::iterator it = (*savedSATClausesPtr).begin(); it != (*savedSATClausesPtr).end(); it++){
        clauses[it->first] = it->second;
        
        //put back the satisfied clauses to their corresponding variables
        for (set<int>::iterator iter = (it->second).begin(); iter != (it->second).end(); iter++){
            int savedVariable = abs(*iter);
            
            if (*iter > 0){
                (variables[savedVariable].clauseMembers).insert(pair<int, int>(it->first, POSITIVE));
            }
            else {
                (variables[savedVariable].clauseMembers).insert(pair<int, int>(it->first, NEGATIVE));
            }
        }
    }

    //put back FALSE literals to their original clauses
    for (vector<int>::iterator it = (*savedFalseLiteralClausePtr).begin(); it != (*savedFalseLiteralClausePtr).end(); it++){

        if ((variables[variable].clauseMembers)[*it] > 0){
            clauses[*it].insert(variable);
        }
        else {
            clauses[*it].insert(NEGATIVE * variable);
        }
    }

    UNSATclauseExists = false;
}

/***************************************************************************/
/* SPLITTING HEURISTICS */

/***************************************************************************
 Function:  undeterminedSH
 Inputs:    none
 Returns:   variable 
 Description:   
 ***************************************************************************/
// int maximumSH(){
//     if (variables.empty() == true) {
//         return INVALID;
//     }

//     vector <int> temp = helperSH();

//     

//     return 
// }

/***************************************************************************
 Function:  randomSH
 Inputs:    none
 Returns:   variable
 Description: picks a random variable from the block
 ***************************************************************************/
int randomSH(){
    if (variables.empty() == true) {
        return INVALID;
    }
    
    vector <int> temp = helperSH();

    srand(time(NULL));
    int randNum = rand() % (temp.size());
        
    return temp[randNum];
}

/***************************************************************************
 Function:  maximumSH
 Inputs:    none
 Returns:   variable 
 Description:   picks the variable that appears in the most clauses
 ***************************************************************************/
int maximumSH(){
    if (variables.empty() == true) {
        return INVALID;
    }

    vector <int> temp = helperSH();

    int max = INT_MIN;
    int maxIndex;
    vector <int>::iterator it;
    for(it = temp.begin(); it!=temp.end(); it++){
        if ((int)(variables[(*it)].clauseMembers).size() > max){
            max = (variables[(*it)].clauseMembers).size();
            maxIndex = (*it);
        }
    }

    return maxIndex;
}

/***************************************************************************
 Function:  helperSH
 Inputs:    none
 Returns:   vector of same block variables
 Description: determines the variables of the same block  
 ***************************************************************************/
vector<int> helperSH(){
    vector <int> temp;
    double previous;
    bool started = false;

    map<int, varInfo>::iterator it;
    for(it = variables.begin(); it!=variables.end(); ++it){
        if(!started){
            started = true;
            previous = (it->second).quantifier;
            temp.push_back(it->first);
        }
        else{
            if(previous * (it->second).quantifier < 0){
                break;
            }
            else{
                temp.push_back(it->first);
                previous = (it->second).quantifier;
            }
        }      
    }

    return temp;
}

/***************************************************************************/
/* UTILITY FUNCTIONS */

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
 Function:  assign
 Inputs:    int
 Returns:   int (positive or negative)
 Description:   indicates the sign of a variable within a given clause
 ***************************************************************************/
void assign(int variable, int value) {
    assignment[variable] = value * variable;
}

/***************************************************************************
 Function:  unassigned_var
 Inputs:    
 Returns:   int (what variable)
 Description:   return the next variable in the same block with no assigned value
 ***************************************************************************/
int unassigned_var(){
    if (variables.empty() == true) {
        return INVALID;
    }
    return variables.begin()->first;
}

<<<<<<< HEAD
/***************************************************************************/
/* PRINTING FUNCTIONS */

=======
>>>>>>> 367009e5861f6f885bf4bdc959b6d8cc7e5a7172
/***************************************************************************
 Function:  printClauses
 Inputs:    nothing
 Returns:   nothing
 Description:   prints the clauses
 ***************************************************************************/
void printClauses() {
    cout << "printing clauses " << endl;
    map<int, set <int> >::iterator it;
    for(it = clauses.begin(); it!=clauses.end(); ++it){
        cout << it->first << ":";
        set<int>::iterator iter;

        for(iter = (it->second).begin(); iter!=(it->second).end();++iter){
            cout << " " << (*iter);
        }
        cout << endl;
    }
    cout << "\n";
}

/***************************************************************************
 Function:  printVariables
 Inputs:    nothing
 Returns:   nothing
 Description:   prints the variables
 ***************************************************************************/
void printVariables() {
    
    map<int, varInfo>::iterator it;

    cout << "printing variable quantifiers " << endl;
    for(it = variables.begin(); it != variables.end(); it++){
        cout << it->first << " => " << (it->second).quantifier << endl;
    }
    cout << "\n";
    
    cout << "printing varInfo" << endl; 
    for (it = variables.begin(); it != variables.end(); it++){
        map<int, int>::iterator itClause = (it->second).clauseMembers.begin();
        cout << "Clause Set of variable " << it->first << endl;
        for (; itClause != (it->second).clauseMembers.end(); itClause++){
            cout << itClause->first << " =>"<< itClause->second << endl;
        }
        cout << endl;
    }
}

/***************************************************************************/
/* READING FILE FUNCTIONS */

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
    
    //the comments
    tokenize(sTemp, vSTemp);
    maximumClauseLength = stoi(vSTemp.back());

    getline(inFile, sTemp);
    tokenize(sTemp, vSTemp);
    minimumClauseLength = stoi(vSTemp.back());

    getline(inFile, sTemp);
    tokenize(sTemp, vSTemp);
    averageClauseLength = stod(vSTemp.back());

    getline(inFile, sTemp);
    tokenize(sTemp, vSTemp);
    seed = stoi(vSTemp.back());

    getline(inFile, sTemp); 
    
    //v
    getline(inFile, sTemp);
    tokenize(sTemp, vSTemp);
    numVars = stoi(vSTemp.back());

    //c
    getline(inFile, sTemp);
    vSTemp.clear();
    tokenize(sTemp, vSTemp);
    numClauses = stoi(vSTemp.back());

    getline(inFile, sTemp); 
    getline(inFile, sTemp); 

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

    getline(inFile, sTemp);
    getline(inFile, sTemp);
    
    for(i = 0; i < numClauses; i++){
        vSTemp.clear();
        getline(inFile, sTemp);
        tokenize(sTemp, vSTemp);
        vSTemp.pop_back();
        
        for(unsigned int j = 0; j < vSTemp.size(); j++){
            int num = stoi(vSTemp[j].c_str());
            vITemp.insert(num); 
            
            if(num > 0){
                ((variables[abs(num)]).clauseMembers).insert(pair<int, int>(i, POSITIVE));
            }
            else{
                ((variables[abs(num)]).clauseMembers).insert(pair<int, int>(i, NEGATIVE));
            }
        }

        clauses.insert(pair<int, set<int> >(i,vITemp));
        vITemp.clear();
    }
    
    inFile.close();
    cout << "File read successfully" << endl;
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


/****** END OF FILE **********************************************************/