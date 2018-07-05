/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <algorithm>

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed

void CirMgr::sweep(){
   for(int i=0;i<undef.size();++i)
      while(i<undef.size()&&(!_gate[undef[i]]->visited)){
         cout<<"Sweeping: UNDEF"<<'('<<undef[i]<<") removed..."<<endl;
         for(int j=0;j<_gate[undef[i]]->fanout.size();++j){
            if(_gate[undef[i]]->fanout[j].gate()->fanin[0].gate()==_gate[undef[i]]){
               _gate[undef[i]]->fanout[j].gate()->fanin[0]=_gate[undef[i]]->fanout[j].gate()->fanin[1];
               _gate[undef[i]]->fanout[j].gate()->fanin.pop_back();
            }else _gate[undef[i]]->fanout[j].gate()->fanin.pop_back();
         }
         delete _gate[undef[i]];
         _gate[undef[i]]=0;
         undef[i]=undef[aig.size()-1];
         undef.pop_back();
      }
   for(int i=0;i<=_data[0];++i)
      while(_gate[i]!=0&&_gate[i]->type==2&&(!_gate[i]->visited)){
         cout<<"Sweeping: AIG"<<'('<<i<<") removed..."<<endl;
         for(int x=0;x<2;++x){
            for(int y=0;y<_gate[i]->fanin[x].gate()->fanout.size();++y){
               if(_gate[i]->fanin[x].gate()->fanout[y].gate()==_gate[i]){
                  _gate[i]->fanin[x].gate()->fanout[y]=_gate[i]->fanin[x].gate()->fanout[_gate[i]->fanin[x].gate()->fanout.size()-1];
                  _gate[i]->fanin[x].gate()->fanout.pop_back();
                  break;
               }
            }
         }
         delete _gate[i];
         _gate[i]=0;
         --_data[4];
      }
   
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
   for(int i=0;i<_dfsList.size();++i){
      if(_gate[_dfsList[i]]->type==2){
         for(int x=0;x<2;++x){
            if(_gate[_dfsList[i]]->input[x]==0){
               cout<<"Simplifying: ";
               merge(0,2*_dfsList[i]);
               break;
            }else if(_gate[_dfsList[i]]->input[x]==1){
               cout<<"Simplifying: ";
               /*for(int j=0;j<_gate[_dfsList[i]]->fanout.size();++j){
                  if(_gate[_dfsList[i]]->fanout[j].gate()->fanin[0].gate()==_gate[_dfsList[i]]){
                     _gate[_dfsList[i]]->fanout[j].gate()->fanin[0]=fCirGate(_gate[_dfsList[i]]->fanin[(x+1)%2].gate(),(_gate[_dfsList[i]]->input[(x+1)%2]+_gate[_dfsList[i]]->fanout[j].gate()->input[0])%2);
                     _gate[_dfsList[i]]->fanout[j].gate()->input[0]=(_gate[_dfsList[i]]->input[(x+1)%2]/2)*2+_gate[_dfsList[i]]->fanout[j].gate()->fanin[0].isinv();
                  }else{
                     _gate[_dfsList[i]]->fanout[j].gate()->fanin[1]=fCirGate(_gate[_dfsList[i]]->fanin[(x+1)%2].gate(),(_gate[_dfsList[i]]->input[(x+1)%2]+_gate[_dfsList[i]]->fanout[j].gate()->input[1])%2);
                     _gate[_dfsList[i]]->fanout[j].gate()->input[1]=(_gate[_dfsList[i]]->input[(x+1)%2]/2)*2+_gate[_dfsList[i]]->fanout[j].gate()->fanin[1].isinv();
                  }
               }
               delete _gate[_dfsList[i]];
               _gate[_dfsList[i]]=0;
               --_data[4];*/
               merge(_gate[_dfsList[i]]->input[(x+1)%2]/2,2*_dfsList[i]+_gate[_dfsList[i]]->input[(x+1)%2]%2);
               break;
            }
         }
         if(_gate[_dfsList[i]]!=0){
            if(_gate[_dfsList[i]]->input[0]/2==_gate[_dfsList[i]]->input[1]/2&&_gate[_dfsList[i]]->input[0]%2!=_gate[_dfsList[i]]->input[1]%2){
               cout<<"Simplifying: ";
               merge(0,2*_dfsList[i]);
            }else if(_gate[_dfsList[i]]->input[0]/2==_gate[_dfsList[i]]->input[1]/2&&_gate[_dfsList[i]]->input[0]%2==_gate[_dfsList[i]]->input[1]%2){
               cout<<"Simplifying: ";
               merge(_gate[_dfsList[i]]->input[0]/2,2*_dfsList[i]+_gate[_dfsList[i]]->input[0]%2);
            }
         }
      }
   }
   check_dfsList();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
