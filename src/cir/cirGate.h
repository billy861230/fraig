/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"
#include <set>
#include <utility>
#include <algorithm>

using namespace std;
#define bit(x) (1ULL<<x)

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;
class fCirGate;
class mypair;
class mypair_sim;
class tuple;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------

class mypair_sim{
public:
   mypair_sim(size_t v=0,bool b=false):value(v),inv(b){}
   ~mypair_sim(){}
   size_t getvalue()const{return value;}
   bool getinv()const{return inv;}
private:
   size_t value;
   bool inv;
};

/*class tuple{
public:
   tuple(bool bo=false,int i=0,int j=0):b(bo){n[0]=i;n[1]=j;}
   ~tuple(){}
   bool get(){return b;}
   int getint(int i){return n[i];}
private:
   int n[2];
   bool b;
};*/

class CirGate
{
   friend class CirMgr;
public:
   CirGate(int id=0,int t=0,int l=0):
   _id(id),type(t),line(l),symb(""),visited(false){
      fanin.clear();
      fanout.clear();
   }
   ~CirGate() {}

   // Basic access methods
   string getTypeStr() const {
      if(type==0)return "PI"; 
      if(type==1)return "PO";
      if(type==2)return "AIG";
      if(type==-1)return "CONST";
      if(type==-2)return "UNDEF";
   }
   mypair operator () () const;
   int getid()const{return _id;}
   int gettype(){return type;}
   int getbit(int &b)const{
      return value&bit(b);
   }
   unsigned getLineNo() const { return line; }
   bool isAig() const { return type==2; }
   void setVar(const Var &v){_var=v;}
   void setv(size_t t){value=t;}
   size_t getv(){return value;}

   // Printing functions
   void printGate() const;
   //void printGate(int &n)const;
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;
   void report(int l,set<int> &v,int level,bool b,bool inv)const;

   mypair_sim getsim(){
      int n=0;
      if(getbit(n))return mypair_sim(~value,true);
      return mypair_sim(value,false);
   }
   
private:
   int _id;
   mutable bool visited;
   int type;
   int line;
   vector<fCirGate> fanin;
   vector<fCirGate> fanout;
   string symb;
   int input[2];
   size_t value;
   bool inv;
   Var _var;

protected:
};

class fCirGate
{
   #define NEG 0x1
public:
   fCirGate (CirGate* g=0, size_t inv=0) :
      _gate(size_t(g) + inv) {} 
   CirGate* gate()const{
      return (CirGate*)(_gate & ~(size_t)(NEG));
   }
   bool isinv()const{ return (_gate & NEG); }
private:
   size_t _gate;
};

class mypair{
public:
   mypair(int x=0,int y=0){
      if(x>y){
         n[0]=x;
         n[1]=y;
      }else{
         n[0]=y;
         n[1]=x;
      }
   }
   int getn0()const{return n[0];}
   int getn1()const{return n[1];}
   ~mypair(){}
   bool operator == (const mypair& p) const{
      return n[0]==p.n[0]&&n[1]==p.n[1];
   }
   //int operator %(int x){return (n[1]+n[0])%x;}
private:
   int n[2];
};



#endif // CIR_GATE_H
