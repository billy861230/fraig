/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include <string>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"
#include <vector>
#include <sstream>
#include <set>
#include <utility>
#include <algorithm>

using namespace std;
#define bit(x) (1ULL<<x)

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

/**************************************/
/*   class CirGate member functions   */
/**************************************/

string int2string(int &i){
	string s;
	stringstream ss(s);
	ss<<i;
	return ss.str();
}

mypair CirGate::operator () () const{
   return mypair(input[0],input[1]);
}

/*void
CirGate::reportGate() const               //unfinished
{
   for(int i=0;i<80;++i)cout<<'=';
	cout<<endl<<"= ";
	string s=getTypeStr();
	s+=+"(";
	int i=_id;
	s+=int2string(i);
	s+=")";
	if(!symb.empty()){
		s+='"';
		s+=symb;
		s+='"';
	}
	s+=", line ";
	int l=line;
	s+=int2string(l);
	cout<<s<<endl;
   cout<<"FECs:";
	for(int x=0,size=_fecGrps.size();x<size;++x){
      for(int y=0;y<_fecGrps[x].size();++y){
         if(_fecGrps[x][y]/2==_id){
            if(_fecGrps[x][y]%2){
               for(int z=0;z<_fecGrps[x].size();++z){
                  if(z!=y){
                     cout<<' ';
                     if(_fecGrps[x][z]%2);
                     else cout<<'!';
                     cout<<_fecGrps[x][z]/2;
                  }
               }
            }else{
               for(int z=0;z<_fecGrps[x].size();++z){
                  if(z!=y){
                     cout<<' ';
                     if(_fecGrps[x][z]%2)cout<<'!';
                     cout<<_fecGrps[x][z]/2;
                  }
               }
            }
            cout<<endl;
            break;
         }
      }
   }
   cout<<"= Value:";
   for(int i=0;i<8;++i){
      cout<<!!value&bit(63-i);
   }
   
   for(int i=1;i<8;++i){
      cout<<'_';
      for(int j=0;j<8;++j){
         cout<<!!value&bit(63-8*i-j);
      }
   }
   cout<<endl;
	for(int i=0;i<80;++i)cout<<'=';
}*/

void CirGate::printGate() const {
   if(type==0)cout<<"PI  "<<_id;
   else if(type==-1)cout<<"CONST0";
   else if(type==-2)cout<<"UNDEF"<<_id;      //unfinished
   else{ 
      if(type==1)
         cout<<"PO  "<<_id;
      else if(type==2)cout<<"AIG "<<_id;
      for(int i=0;i<fanin.size();i++){
         cout<<' ';
         if(fanin[i].gate()->type==-2){
            cout<<'*';
         }
         if(input[i]%2)cout<<'!';
         cout<<input[i]/2;
      }
   }
   if(!symb.empty())cout<<' '<<'('<<symb<<')';
   cout<<endl;
}

void
CirGate::reportFanin(int level) const              //unfinished
{
   assert (level >= 0);
   cout<<getTypeStr()<<' '<<_id<<endl;
   std::set<int> v;
   v.insert(_id);
   CirGate* g;
   for(int i=0;i<fanin.size();++i){
   	//if(fanin[i].gate()!=0){
   		g=fanin[i].gate();
      	g->report(level,v,1,true,fanin[i].isinv());
   	//}
   }
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   cout<<getTypeStr()<<' '<<_id;
   cout<<endl;
   CirGate* g;
   set<int> v;
   v.insert(_id);
   for(int i=0;i<fanout.size();++i){
   	g=fanout[i].gate();
      g->report(level,v,1,false,fanout[i].isinv());
   }
}

/*void CirGate::printGate(int &n)const{
	cout<<'['<<n<<"] ";
   if(type==0)cout<<"PI  "<<_id;
   else if(type==-1)cout<<"CONST0";
	else{ 
		if(type==1)
			cout<<"PO  "<<_id<<' ';
		else if(type==2)cout<<"AIG "<<_id<<' ';
		for(int i=0;i<fanin.size();i++){
			if(fanin[i].gate()==0){
				cout<<'*';
			}
			if(fanin[i].isinv())cout<<'!';
			if(fanin[i].gate()==0)cout<<input[i]/2;
			else cout<<fanin[i].gate()->_id<<' ';
		}
	}
	if(!symb.empty())cout<<'('<<symb<<')';
	cout<<endl;
	++n;
}*/

void CirGate::report(int l,set<int> &v,int level,bool b,bool inv)const{
      for(int i=0;i<2*level;++i)cout<<' ';
      if(inv)cout<<'!';
      cout<<getTypeStr()<<' '<<_id<<' ';
      if(v.find(_id)!=v.end()&&type==2){
      	cout<<"(*)"<<endl;
      	return;
      }
      if(l==1){
      	cout<<endl;
      	return;
      }
      v.insert(_id);
      cout<<endl;
      CirGate* g;
      if(b){
         for(int i=0;i<fanin.size();++i){
		   	//if(fanin[i].gate()!=0){
		   		g=fanin[i].gate();
		      	g->report(l-1,v,level+1,true,fanin[i].isinv());
		   	//}
		   }
      }else{
         for(int i=0;i<fanout.size();++i){
            g=fanout[i].gate();
            g->report(l-1,v,level+1,false,fanout[i].isinv());
         }
      }
   }

