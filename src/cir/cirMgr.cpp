/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stack>
#include <sstream>

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   ifstream ifs(fileName);
   ifs>>buf;
   for(int i=0;i<5;++i){
      ifs>>_data[i];
   }
   _gate.resize(_data[0]+_data[3]+1);
   _gate[0]=new CirGate(0,-1,-1);
   _input.resize(_data[1]);
   lineNo=2;
   int num1,num2,num3;
   for(int i=0;i<_data[1];++i){
      ifs>>num1;
      _gate[num1/2]=new CirGate(num1/2,0,lineNo);
      _input[i]=num1/2;
      ++lineNo;
   }
   aig.resize(_data[4]);
   for(int i=0;i<_data[3];++i){
      _gate[_data[0]+i+1]=new CirGate(_data[0]+i+1,1,lineNo);
      ifs>>_gate[_data[0]+i+1]->input[0];
      ++lineNo;
   }
   for(int i=0;i<_data[4];++i){
      ifs>>num1;ifs>>num2;ifs>>num3;
      _gate[num1/2]=new CirGate(num1/2,2,lineNo);
      _gate[num1/2]->input[0]=num2;
      _gate[num1/2]->input[1]=num3;
      aig[i]=num1/2;
      ++lineNo;
   }
   for(int i=0;i<_data[4];++i){                                          //unfinished
      num1=_gate[aig[i]]->input[0];
      num2=_gate[aig[i]]->input[1];
      if(_gate[num1/2]==0){
         _gate[num1/2]=new CirGate(num1/2,-2,0);
         undef.push_back(num1/2);
      }
      if(_gate[num2/2]==0){
         _gate[num2/2]=new CirGate(num2/2,-2,0);
         undef.push_back(num2/2);
      }
      _gate[aig[i]]->fanin.push_back(fCirGate(_gate[num1/2],num1%2));
      _gate[aig[i]]->fanin.push_back(fCirGate(_gate[num2/2],num2%2));
      _gate[num1/2]->fanout.push_back(fCirGate(_gate[aig[i]],num1%2));
      _gate[num2/2]->fanout.push_back(fCirGate(_gate[aig[i]],num2%2));
   }
   for(int i=0;i<_data[3];++i){
      num1=_gate[_data[0]+i+1]->input[0];
      _gate[_data[0]+i+1]->fanin.push_back(fCirGate(_gate[num1/2],num1%2));
      if(_gate[num1/2]==0){
         _gate[num1/2]=new CirGate(num1/2,-2,0);
         undef.push_back(num1/2);
      }
      _gate[num1/2]->fanout.push_back(fCirGate(_gate[_data[0]+i+1],num1%2));
   }
   check_dfsList();
   for(int i=0,size=_gate.size();i<size;++i){
      if(_gate[i]!=0)_gate[i]->setv(0);
   }
   vector<int> v;
   v.push_back(0);
   for(int i=0,size=_dfsList.size();i<size;++i)if(_gate[_dfsList[i]]!=0&&_gate[_dfsList[i]]->type==2)v.push_back(_dfsList[i]*2);
   _fecGrps.push_back(v);
   ifs.get(*buf);
   while(true){
      lineNo++;
      ifs.getline(buf,1024, ' ');
      if(buf[0]=='i'){
         myStr2Int(buf+1,num1);
         ifs.getline(buf,1024);
         _gate[_input[num1]]->symb=buf;
      }
      else if(buf[0]=='o'){
         myStr2Int(buf+1,num1);
         ifs.getline(buf,1024);
         _gate[_data[0]+1+num1]->symb=buf;
      }else return true;
   }
}


/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   cout<<"Circuit Statistics"<<endl;
   cout<<"=================="<<endl;
   cout<<"  "<<"PI"<<setw(12)<<_data[1]<<endl;
   cout<<"  "<<"PO"<<setw(12)<<_data[3]<<endl;
   cout<<"  "<<"AIG"<<setw(11)<<_data[4]<<endl;
   cout<<"------------------"<<endl;
   cout<<"  "<<"Total"<<setw(9)<<_data[1]+_data[3]+_data[4]<<endl;
}

void
CirMgr::printNetlist() const
{
   cout << endl;
   for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      _gate[_dfsList[i]]->printGate();
   }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(int i=0;i<_data[1];++i){
      cout<<' '<<_input[i];
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(int i=_data[0]+1;i<_gate.size();++i){
      cout<<' '<<i;
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   cout<<"Gates with floating fanin(s):";
   for(int i=0;i<=_data[0];++i){
      if(_gate[i]!=0&&_gate[i]->type==2){
         if(_gate[i]->fanin[0].gate()->type==-2)cout<<' '<<i;
         else if(_gate[i]->fanin[1].gate()->type==-2)cout<<' '<<i;
      }
   }
   for(int i=_data[0]+1;i<_gate.size();++i){
      if(_gate[i]->fanin[0].gate()->type==-2)cout<<' '<<i;
   }
   cout<<endl;
   cout<<"Gates defined but not used  :";
   for(int i=0;i<_gate.size();++i){
      if(_gate[i]!=0&&_gate[i]->fanout.empty()){
         if((_gate[i]->type==0||_gate[i]->type==2)){
            cout<<' '<<_gate[i]->_id;
         }
      }
   }
   cout<<endl;
}

static string int2string(int &i){
   string s;
   stringstream ss(s);
   ss<<i;
   return ss.str();
}

struct v_s{
   bool operator()(const vector<int> v1,const vector<int> v2){
      return v1[0]<v2[0];
   }
};

void
CirMgr::printFECPairs() const
{
   vector<vector<int>> v=_fecGrps;
   for(int i=0;i<v.size();++i){
      sort(v[i].begin(),v[i].end());
      if(v[i][0]%2){
         for(int j=0;j<v[i].size();++j){
            if(v[i][j]%2)--v[i][j];
            else ++v[i][j];
         }
      }
   }
   v_s s;
   sort(v.begin(),v.end(),s);

   

   for(int i=0,size=v.size();i<size;++i){
      cout<<'['<<i<<']';
      for(int j=0;j<v[i].size();++j){
         cout<<' ';
         if(v[i][j]%2)cout<<'!';
         cout<<v[i][j]/2;
      }
      cout<<endl;
   }
}

void
CirMgr::writeAag(ostream& outfile) const
{
   for(int i=0;i<=_data[0];++i)if(_gate[i]!=0)_gate[i]->visited=false;
   string s="aag ";
   for(int i=0;i<4;++i){
      int j=_data[i];
      s+=int2string(j);
      s+=" ";
   }
   int n=0;
   for(int i=0;i<_dfsList.size();++i){
      if(_gate[_dfsList[i]]->type==2)++n;
   }
   s+=int2string(n);
   s+='\n';
   for(int i=0;i<_data[1];++i){
      int j=_input[i]*2;
      s+=int2string(j);
      s+='\n';
   }
   for(int i=_data[0]+1;i<=_data[0]+_data[3];++i){
      int j=_gate[i]->input[0];
      s+=int2string(j);
      s+='\n';
   }
   for(int i=0;i<_dfsList.size();++i){
      if(_gate[_dfsList[i]]->type==2){
         n=_dfsList[i]*2;
         s+=int2string(n);
         s+=' ';
         n=_gate[_dfsList[i]]->input[0];
         s+=int2string(n);
         s+=' ';
         n=_gate[_dfsList[i]]->input[1];
         s+=int2string(n);
         s+='\n';
      }
   }
   for(int i=0;i<_input.size();++i){
      if(!_gate[_input[i]]->symb.empty()){
         s+='i';
         int j=i;
         s+=int2string(j);
         s+=' ';
         s+=_gate[_input[i]]->symb;
         s+='\n';
      }
   }
   for(int i=0;i<_data[3];++i){
      if(!_gate[_data[0]+1+i]->symb.empty()){
         s+='o';
         int j=i;
         s+=int2string(j);
         s+=' ';
         s+=_gate[_data[0]+i]->symb;
         s+='\n';
      }
   }
   streamsize size=s.size();
   outfile.write(s.c_str(),size);
}

void CirMgr::push(CirGate* g,vector<CirGate*> &v)const{
   if(g->type!=2)return;
   if(g->visited)return;
   g->visited=true;
   for(int i=0;i<2;++i){
      if(g->fanin[i].gate()!=0)push(g->fanin[i].gate(),v);
   }
   v.push_back(g);
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
   for(int i=0;i<=_data[0];++i)if(_gate[i]!=0)_gate[i]->visited=false;
   vector<int> n;
   n.clear();
   vector<int> n_input;
   visit(g,n,true);
   int new_header[5];
   new_header[0]=_data[0];
   new_header[2]=0;
   new_header[3]=1;
   new_header[1]=0;
   new_header[4]=0;
   for(int i=0;i<n.size();++i){
      if(_gate[n[i]]->type==0){
         new_header[1]++;
         n_input.push_back(n[i]);
      }
      else if(_gate[n[i]]->type==2)new_header[4]++;
   }
   string s="aag ";
   int j;
   for(int i=0;i<5;++i){
      j=new_header[i];
      s+=int2string(j);
      s+=" ";
   }
   s+='\n';
   for(int i=0;i<n_input.size();++i){
      j=n_input[i]*2;
      s+=int2string(j);
      s+='\n';
   }
   j=_gate[n[n.size()-1]]->_id*2;
   s+=int2string(j);
   s+='\n';
   for(int i=0;i<n.size();++i){
      if(_gate[n[i]]->type==2){
         j=n[i]*2;
         s+=int2string(j);
         s+=' ';
         j=_gate[n[i]]->input[0];
         s+=int2string(j);
         s+=' ';
         j=_gate[n[i]]->input[1];
         s+=int2string(j);
         s+='\n';
      }
   }
   for(int i=0;i<n_input.size();++i){
      if(!_gate[n_input[i]]->symb.empty()){
         s+='i';
         j=i;
         s+=int2string(j);
         s+=' ';
         s+=_gate[n_input[i]]->symb;
         s+='\n';
      }
   }
   if(!_gate[n[n.size()-1]]->symb.empty()){
      s+='o';
      j=0;
      s+=int2string(j);
      s+=' ';
      s+=_gate[n[n.size()-1]]->symb;
      s+='\n';
   }
   for(int i=_data[0]+1;i<=_data[0]+_data[3];++i){
      visit(_gate[i],n,false);
   }
   streamsize size=s.size();
   outfile.write(s.c_str(),size);
}

void CirMgr::reportgate(int g)const{
   for(int i=0;i<80;++i)cout<<'=';
	cout<<endl<<"= ";
	string s=_gate[g]->getTypeStr();
	s+=+"(";
	int i=g;
	s+=int2string(i);
	s+=")";
	if(!_gate[g]->symb.empty()){
		s+='"';
		s+=_gate[g]->symb;
		s+='"';
	}
	s+=", line ";
	int l=_gate[g]->line;
	s+=int2string(l);
	cout<<s<<endl;
   cout<<"= FECs:";
   if(_gate[g]->type==2){
      for(int x=0,size=_fecGrps.size();x<size;++x){
         for(int y=0;y<_fecGrps[x].size();++y){
            if(_fecGrps[x][y]/2==_gate[g]->_id){
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
               break;
            }
         }
      }
   }
   cout<<endl;
   cout<<"= Value:";
   for(int i=0;i<8;++i){
      if((_gate[g]->value)&bit(63-i))cout<<'1';
      else cout<<'0';
   }
   
   for(int i=1;i<8;++i){
      cout<<'_';
      for(int j=0;j<8;++j){
         if((_gate[g]->value)&bit(63-8*i-j))cout<<'1';
         else cout<<'0';
      }
   }
   cout<<endl;
	for(int i=0;i<80;++i)cout<<'=';
}

