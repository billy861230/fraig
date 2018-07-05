/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"
#include <utility>
#include <hash_map>


using namespace std;
using namespace __gnu_cxx;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed

struct hash_pair{
   int operator()(const mypair &p)const{
      return p.getn0();
   }
};

struct equal_pair{
   bool operator()(const mypair &p1, const mypair &p2)const{
      return p1.getn0()==p2.getn0()&&p1.getn1()==p2.getn1();
   }
};

void
CirMgr::strash()
{
   hash_map<mypair,int,hash_pair,equal_pair> hash;
   hash_map<mypair,int,hash_pair,equal_pair>::iterator it;
   mypair p;
   CirGate gate;
   for(int i=0;i<_dfsList.size();++i){
      if(_gate[_dfsList[i]]->type==2){
         gate=*_gate[_dfsList[i]];
         p=gate();
         it=hash.find(p);
         if(it!=hash.end()){
            cout<<"Strashing: ";
            merge(it->second,_dfsList[i]*2);
         }else hash.insert(make_pair(p,_dfsList[i]));
      }
   }
   check_dfsList();
}

void
CirMgr::fraig()
{
   SatSolver solver;
   int merged=0;
   Var v;
   int j;
   bool b;
   genPfModel(solver);
   vector<int> vec;        //solved|_fecgrp|num
   for(int i=0,size=_fecGrps.size();i<size;++i){
      b=true;
      if(merged>1024){
         for(int m=0;m<vec.size();++m){
            delete _gate[vec[m]];
            _gate[vec[m]]=0;
         }
         check_dfsList();
         merged=0;
         genPfModel(solver);
         vec.clear();
      }
      for(j=1;j<_fecGrps[i].size();++j){
         v=solver.newVar();
         solver.addXorCNF(v,_gate[_fecGrps[i][0]/2]->_var, false,_gate[_fecGrps[i][j]/2]->_var,_fecGrps[i][j]%2);
         solver.assumeRelease();
         _gate[0]->_var=solver.newVar();
         solver.assumeProperty(_gate[0]->_var,false);
         solver.assumeProperty(v, true);
         if(solver.assumpSolve()){
            cout<<"fraig: ";
            merge_f(_fecGrps[i][0]/2,_fecGrps[i][j]);
            vec.push_back(_fecGrps[i][j]/2);
            _fecGrps[i].erase(_fecGrps[i].begin()+j);
            merged++;
            --j;
         }else if(_fecGrps[i].size()==2)_fecGrps[i].clear();//clear
      }
      _fecGrps[i].erase(_fecGrps[i].begin());//erase
   }
   for(int m=0;m<vec.size();++m){
      delete _gate[vec[m]];
      _gate[vec[m]]=0;
   }
   check_dfsList();
   checkfec();
   cout<<"Updating by UNSAT... Total #FEC Group = "<<_fecGrps.size()<<endl;
}



void CirMgr::merge(int x,int z){
   int y=z/2;
   int k;
   cout<<x<<" merging ";
   if(z%2)cout<<'!';
   cout<<y<<"..."<<endl;
   for(int i=0;i<_gate[y]->fanout.size();++i){
      if(_gate[y]->fanout[i].gate()->input[0]/2==y){
         _gate[y]->fanout[i].gate()->input[0]=x*2+(z+_gate[y]->fanout[i].gate()->input[0])%2;
         _gate[y]->fanout[i].gate()->fanin[0]=fCirGate(_gate[x],(z+_gate[y]->fanout[i].gate()->input[0])%2);
      }else{
         _gate[y]->fanout[i].gate()->input[1]=x*2+(z+_gate[y]->fanout[i].gate()->input[1])%2;
         _gate[y]->fanout[i].gate()->fanin[1]=fCirGate(_gate[x],(z+_gate[y]->fanout[i].gate()->input[0])%2);
      }
      k=z-_gate[y]->fanout[i].isinv();
      _gate[x]->fanout.push_back(fCirGate(_gate[y]->fanout[i].gate(),k%2));
   }
   for(int i=0;i<2;++i){
      for(int j=0;j<_gate[y]->fanin[i].gate()->fanout.size();++j){
         if(_gate[y]->fanin[i].gate()->fanout[j].gate()==_gate[y]){//?
            _gate[y]->fanin[i].gate()->fanout[j]=_gate[y]->fanin[i].gate()->fanout[_gate[y]->fanin[i].gate()->fanout.size()-1];
            _gate[y]->fanin[i].gate()->fanout.pop_back();
            if(_gate[y]->fanin[i].gate()->type==-2&&_gate[y]->fanin[i].gate()->fanout.empty()){
               k=_gate[y]->fanin[i].gate()->_id;
               delete _gate[k];
               _gate[k]=0;
            }
            break;
         }
      }
   }
   delete _gate[y];
   _gate[y]=0;
   --_data[4];
}


void CirMgr::merge_f(int x,int z){
   int y=z/2;
   int k;
   cout<<x<<" merging ";
   if(z%2)cout<<'!';
   cout<<y<<"..."<<endl;
   for(int i=0;i<_gate[y]->fanout.size();++i){
      if(_gate[y]->fanout[i].gate()->input[0]/2==y){
         _gate[y]->fanout[i].gate()->input[0]=x*2+(z+_gate[y]->fanout[i].gate()->input[0])%2;
         _gate[y]->fanout[i].gate()->fanin[0]=fCirGate(_gate[x],(z+_gate[y]->fanout[i].gate()->input[0])%2);
      }else{
         _gate[y]->fanout[i].gate()->input[1]=x*2+(z+_gate[y]->fanout[i].gate()->input[1])%2;
         _gate[y]->fanout[i].gate()->fanin[1]=fCirGate(_gate[x],(z+_gate[y]->fanout[i].gate()->input[0])%2);
      }
      k=z-_gate[y]->fanout[i].isinv();
      _gate[x]->fanout.push_back(fCirGate(_gate[y]->fanout[i].gate(),k%2));
   }
   for(int i=0;i<2;++i){
      for(int j=0;j<_gate[y]->fanin[i].gate()->fanout.size();++j){
         if(_gate[y]->fanin[i].gate()->fanout[j].gate()==_gate[y]){//?
            _gate[y]->fanin[i].gate()->fanout[j]=_gate[y]->fanin[i].gate()->fanout[_gate[y]->fanin[i].gate()->fanout.size()-1];
            _gate[y]->fanin[i].gate()->fanout.pop_back();
            if(_gate[y]->fanin[i].gate()->type==-2&&_gate[y]->fanin[i].gate()->fanout.empty()){
               k=_gate[y]->fanin[i].gate()->_id;
               delete _gate[k];
               _gate[k]=0;
            }
            break;
         }
      }
   }
   --_data[4];
}
/********************************************/
/*   Private member functions about fraig   */
/********************************************/
