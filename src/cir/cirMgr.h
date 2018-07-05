/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>
#include "cirGate.h"
#include <set>
#include <sat.h>
#include <utility>
#include <hash_map>

using namespace std;
using namespace __gnu_cxx;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr() {}
   ~CirMgr() {} 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const { 
      for(int i=0;i<_gate.size();++i)if(_gate[i]!=0)if(_gate[i]->_id==gid)return _gate[i];
      return 0; 
   }
   void check_dfsList(){
      _dfsList.clear();
      for(int i=0,n=_gate.size();i<n;++i){
         if(_gate[i]!=0)_gate[i]->visited=false;
      }
      for(int i=_data[0]+1;i<_data[0]+1+_data[3];++i){
         visit(_gate[i],_dfsList,true);
      }
   }

   // Member functions about circuit construction
   bool readCircuit(const string&);
   void visit(const CirGate* g,vector<int> &n,bool b)const{
   	for(int i=0;i<g->fanin.size();++i){
   		if(!(g->fanin[i].gate()->visited))visit(g->fanin[i].gate(),n,b);
   	}
   	if(b&&g->type!=-2)n.push_back(g->_id);
   	g->visited=true;
   }

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(const char *patternFile);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }
   void sim();

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();
   void merge(int ,int );
   void collfecgrp(int&);
   void checkfec();

   // Member functions about circuit reporting
   void reportgate(int)const;
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;
   void push(CirGate* ,vector<CirGate*> &)const;
   void merge_f(int ,int );
   void genPfModel(SatSolver &s){
      s.initialize();
      for(int i=0;i<_data[1];++i){
         _gate[_input[i]]->_var=s.newVar();
      }
      for(int i=0,size=_dfsList.size();i<size;++i){
         if(_gate[_dfsList[i]]->type==2){
            _gate[_dfsList[i]]->_var=s.newVar();
            s.addAigCNF(_gate[_dfsList[i]]->_var,_gate[_dfsList[i]]->fanin[0].gate()->_var,_gate[_dfsList[i]]->fanin[0].isinv(),_gate[_dfsList[i]]->fanin[1].gate()->_var,_gate[_dfsList[i]]->fanin[1].isinv());
         }
      }
   }

private:
   ofstream           *_simLog;
   vector<CirGate*> _gate;
   int _data[5];
   vector<int> aig;
   vector<int> undef;
   mutable vector<int> _dfsList;
   vector<vector<int>> _fecGrps;
	std::vector<int> _input;
};

#endif // CIR_MGR_H
