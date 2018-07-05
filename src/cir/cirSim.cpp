/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <string.h>
#include <vector>
#include <stdio.h>
#include <cstddef>
#include <errno.h> 
#include <stdlib.h>
#include <vector>
#define private public
#define protected public
#include <fstream>
#undef private
#undef protected
#include <fcntl.h>
#define VS std::vector<std::size_t>
#define bit(x) (1ULL<<x)

using namespace std;
using namespace __gnu_cxx;

struct read_file{
	char c,S[65536]; // temp char
	long unsigned input,err_n,last;
	FILE *fin;
	read_file(const char *file_name,long unsigned I){
		if(!(fin = fopen(file_name , "r"))){
			perror("fopen");
			exit(0);
		}
		input = I;
	}
	VS get_row(){
		last = 0;
		if(feof(fin))
			return VS(0);
		VS row(input,0);
		for(int i=0; i<64 && !feof(fin) ; i++){
			fscanf(fin,"%s",S);
			if(strlen(S) != input)	// check type-I Error
				goto err;
			for(err_n =0;err_n <input;err_n++) 	// check type-II Error
				if(S[err_n] != '0' && S[err_n] != '1')
					goto err;

			// fill the row
			for(int j=0 ; j < input ; j++)
				if(S[j] == '1')
					row[j] |= (1ULL<<i);
			last ++;
			//printf("%d : [%s]\n",last, S);
			while( (c = fgetc(fin))!=EOF && (c==' ' || c=='\n'));
			if(!feof(fin))	ungetc(c,fin);
		}
		return row;
		///error handling///
		err:
			fprintf(stderr,"Error: Pattern(%s) ",S);
			if(strlen(S) != input)
				fprintf(stderr,"length(%lu) does not match the number of inputs(%lu) in a circuit!!\n",strlen(S),input);
			else
				fprintf(stderr,"contains a non-0/1 character('%c').\n",S[err_n]);
		return VS(0);
	}
};
struct hash_pair_sim{
   int operator()(const mypair_sim &p)const{
      return p.getvalue();
   }
};

struct equal_pair_sim{
   bool operator()(const mypair_sim &p1, const mypair_sim &p2)const{
      return p1.getvalue()==p2.getvalue();
   }
};



// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
   int max_fail=2*log(_data[1])*pow(1.5,log(_dfsList.size()));
   int count=0;
   int fails=0;
   size_t v;
   while(fails<max_fail){
      if(count>0&&_fecGrps.empty())break;
      ++count;
      for(int i=0,in=_data[1];i<in;++i){
         v=rnGen(pow(2,64)-1);
         _gate[_input[i]]->setv(v);
      }
      sim();
      collfecgrp(fails);
   }
   cout<<count*64<<" patterns simulated."<<endl;
}

void
CirMgr::fileSim(const char *patternFile)
{
   vector<string> input_value;
   int count=0;
   VS v;
   int i;
   read_file f(patternFile,_data[1]);
   while(1){
      int i=0;
      v=f.get_row();
      if(v.empty())break;
      count+=f.last;
      for(int i=0;i<_data[1];++i)_gate[_input[i]]->setv(v[i]);
      sim();
      collfecgrp(i);
   }
   cout<<count<<" patterns simulated."<<endl;
}

void CirMgr::sim(){
   for(int i=0,size=_dfsList.size();i<size;++i){
      if(_gate[_dfsList[i]]->type==2){
         if(_gate[_dfsList[i]]->fanin[0].isinv()){
            if(_gate[_dfsList[i]]->fanin[1].isinv()){
               _gate[_dfsList[i]]->setv((~(_gate[_dfsList[i]]->fanin[0].gate()->getv()))&(~(_gate[_dfsList[i]]->fanin[1].gate()->getv())));
               //cout<<i<<endl;             //debug
            }
            else {
               _gate[_dfsList[i]]->setv((~(_gate[_dfsList[i]]->fanin[0].gate()->getv()))&(_gate[_dfsList[i]]->fanin[1].gate()->getv()));
               //cout<<i<<endl;             //debug
            }
         }else {
            if(_gate[_dfsList[i]]->fanin[1].isinv()){
               _gate[_dfsList[i]]->setv((_gate[_dfsList[i]]->fanin[0].gate()->getv())&(~(_gate[_dfsList[i]]->fanin[1].gate()->getv())));
               //cout<<i<<endl;             //debug
            }
            else{
               _gate[_dfsList[i]]->setv((_gate[_dfsList[i]]->fanin[0].gate()->getv())&(_gate[_dfsList[i]]->fanin[1].gate()->getv()));
               //cout<<i<<endl;             //debug
            }
         }
      }
   }
   if(_simLog!=0){
      for(int j=0;j<64;++j){
         for(int i=0,size=_data[1];i<size;++i)(*_simLog)<<_gate[_input[j]]->getbit(i);
         (*_simLog)<<' ';
         for(int i=_data[0],size=_gate.size();i<size;++i)(*_simLog)<<_gate[j]->getbit(i);
      }
   }
   /*for(int i=0;i<_dfsList.size();++i){     ////////////////////debug/////////////////////////////
      cout<<i<<' '<<!!_gate[_dfsList[i]]->getsim().getvalue()&(1ULL<<0)<<endl;
   }*/
}

struct v_s{
   bool operator()(const vector<int> v1,const vector<int> v2){
      return v1[0]<v2[0];
   }
};

void CirMgr::collfecgrp(int &fail){
   mypair_sim p;
   vector<vector<int>> n_fec;
   bool b=true;
   for(int i=0,size=_fecGrps.size();i<size;++i){
      hash_map<mypair_sim,vector<int>,hash_pair_sim,equal_pair_sim> hash;
      hash_map<mypair_sim,vector<int>,hash_pair_sim,equal_pair_sim>::iterator it;
      for(int j=0,s=_fecGrps[i].size();j<s;++j){
         p=_gate[_fecGrps[i][j]/2]->getsim();
         it=hash.find(p);
         if(it==hash.end()){
            vector<int> v;
            v.push_back(_fecGrps[i][j]/2*2+p.getinv());
            hash.insert(make_pair(p,v));
            if(j>0)b=false;
         }else{
            it->second.push_back(_fecGrps[i][j]/2*2+p.getinv());
         }
      }
      for(it=hash.begin();it!=hash.end();++it){
         if(it->second.size()>1){
            if(it->second[0]%2){
               for(int i=0,size=it->second.size();i<size;++i){
                  if(it->second[i]%2)--it->second[i];
                  else ++it->second[i];
               }
            }
            n_fec.push_back(it->second);
         }
      }
   }
   if(b)++fail;
   else {
      _fecGrps=n_fec;
   }
}

void CirMgr::checkfec(){
   for(int i=0;i<_fecGrps.size();++i){
      while(i<_fecGrps.size()&&_fecGrps[i].size()<2)_fecGrps.erase(_fecGrps.begin()+i);
      if(i<_fecGrps.size()&&_fecGrps[i][0]%2){
         for(int j=0;j<_fecGrps[i].size();++j){
            if(_fecGrps[i][j]%2)--_fecGrps[i][j];
            else ++_fecGrps[i][j];
         }
      }
   }
}



/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
