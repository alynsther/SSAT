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
#include <algorithm>


/***************************************************************************/
/* globals variables */
using namespace std;

//probably will not use these vars...
int maximumClauseLength;
int minimumClauseLength;
double averageClauseLength;
int seed;

int numVars;
int numClauses;

vector <double> variables;
vector < vector <int> > clauses;

vector <vector <int> > solution;
double successProbability;
double solutionTime;

/***************************************************************************/
/* functions */
void readFile(string input);
void tokenize(string str, vector<string> &token_v);

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
    vector <string> vSTemp;
    vector <int> vITemp;
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
    while(i >0 ){
        getline(inFile, sTemp);
        vSTemp.clear();
        tokenize(sTemp, vSTemp);

        for(int b = 0; b < vSTemp.size(); b++){
            if(b%2 == 1) {
                variables.push_back(stod(vSTemp.at(b)));
            }
        }

        i--;    
    }

    cout << "variables size " <<variables.size() << endl;
    for(int b = 0; b < variables.size(); b++){
        cout << variables.at(b) << " ";
    }
    cout << "\n";

    getline(inFile, sTemp);
    getline(inFile, sTemp);
    i = numClauses;
    while(i >0 ){
        vSTemp.clear();
        getline(inFile, sTemp);
        tokenize(sTemp, vSTemp);
        vSTemp.pop_back();
        for(int j = 0; j < vSTemp.size(); j++){
            int num = stoi(vSTemp.at(j).c_str());
            vITemp.push_back(num);
        }
        clauses.push_back(vITemp);
        vITemp.clear();

        i--;
    }

    cout << "clauses size " << clauses.size() << endl;
    for(int a = 0; a < clauses.size(); a++){  
        for(int c = 0; c < (clauses.at(a)).size(); c++){
            cout << (clauses.at(a)).at(c) << " ";
        }
        cout << "\n";
    }

    inFile.close();
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
 Function:
 Inputs:
 Returns:
 Description:
 ***************************************************************************/


/****** END OF FILE **********************************************************/
