//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "dependencyGraph"

STATISTIC(HelloCounter, "Counts number of functions greeted");

namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct ArrayReference{
     StringRef arrayName;
     APInt offset;
  };

  ArrayReference solveElementPtr(BasicBlock *BB, StringRef elementPtrID){
        StringRef arrayName;
        APInt offset;
        for(BasicBlock::reverse_iterator inst = BB->rbegin(),inst_e = BB->rend();
                inst != inst_e; ++inst){
            Instruction *I = &*inst;
            if(elementPtrID == I->getName()){
                Value *v = I->getOperand(0);
                arrayName=v->getName();
                Value *v1= I->getOperand(1);
                
                if(ConstantInt* CI = dyn_cast<ConstantInt>(v1)){
                    offset = CI->getValue();
                }
            }
                
        }
        ArrayReference arrayRefInfo = {arrayName, offset};
        return arrayRefInfo;
  }

  std::string replaceAll(StringRef inString, char toReplace, char replacement){
      std::string str_data = inString.str();
      std::replace(str_data.begin(), str_data.end(),toReplace,replacement);
      return str_data;
  }

  struct DependencyGraphGen : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    DependencyGraphGen() : FunctionPass(ID) {}
    
    bool runOnFunction(Function &F) override {
      ++HelloCounter;
      errs() << "//Data Dependency Graph Generator running on: ";
      errs().write_escaped(F.getName()) << '\n';
      int block=0;
      errs() << "//Iterating over basic blocks of "<< F.getName() << '\n';
      for(Function::iterator b = F.begin(), be = F.end(); b != be; ++b) {
        BasicBlock *BB = &*b;
        errs()<< "//Block: " << block << '\n';
        block++;      
        int inst_count =0;
        errs() << "digraph {"<<'\n';
        
        for(BasicBlock::iterator inst = BB->begin(),inst_e = BB->end(); inst != inst_e; ++inst){
            Instruction *I = &*inst;
            Value *destination = cast<Value>(I);
            errs()<< "//Instruction:" << inst_count <<", "<< I->getOpcodeName() <<" "<< I->getName()<< " " << destination->getName() <<'\n';
            //Only create node for operations and memory accesses
            if(I->getOpcodeName() != StringRef("getelementptr") && I->getOpcodeName() != StringRef("ret")){
                //Verify if there is a pointer indirection
                if(I->getOpcodeName() == StringRef("load") ||I->getOpcodeName() == StringRef("store")){
                    Value *op; 
                    if (I->getOpcodeName() == StringRef("load"))
                        op=I->getOperand(0);
                    else
                        op=I->getOperand(1);

                    ArrayReference a;
                    if( op->hasName() && op->getName().contains(StringRef("arrayidx"))){
                        a=solveElementPtr(BB,op->getName());
                    }else{
                        StringRef ArrayID = op->getName();
                        //a={ArrayID,0};    
                        a.arrayName = ArrayID;
                        a.offset = 0;
                    }
                    StringRef shape;
                    if(I->getOpcodeName() == StringRef("store")){
                        shape = StringRef("triangle");
                    }else{
                        shape=StringRef("invtriangle");
                    }
                    if (I->getOpcodeName() == StringRef("load"))
                        errs()<< replaceAll(I->getName(),'.','_')<< " " <<"[label=\""<< a.arrayName <<"["<<a.offset<<"]\";shape="<<shape<<"]"<<'\n';
                    else{
                        errs()<< a.arrayName<<"_"<<a.offset<< " " <<"[label=\""<< a.arrayName <<"["<<a.offset<<"]\";shape="<<shape<<"]"<<'\n';
                        Value *source = I->getOperand(0);
                        errs() << replaceAll(source->getName(),'.','_') << " -> " << a.arrayName<<"_"<<a.offset << '\n';
                    }
                }
                else{
                    errs() << replaceAll(I->getName(),'.','_') << " " <<"[label=\"" << I->getName() << "\";shape=\"ellipse\"]"<<'\n';
                    for(unsigned i=0;i<I->getNumOperands();i++){
                       Value *op = I->getOperand(i);
                        errs() <<replaceAll(op->getName(),'.','_') <<" -> " << replaceAll(I->getName(),'.','_')<<'\n';
                    }

                }
            }

                
            for(unsigned operandNb=0; operandNb< I->getNumOperands() ; operandNb++){
               Value *op = I->getOperand(operandNb);

               if(op->hasName()){
               StringRef name = op->getName();
               errs() << "//Opearand: " << operandNb <<" " << name  <<'\n';
               }else{

                
               if(ConstantInt* CI = dyn_cast<ConstantInt>(op))
                   errs() << "//Opearand: " << operandNb <<" " << CI->getValue()  <<'\n';
               }
            }
            inst_count++;
        
        }
        errs()<<"}\n";
      }
      return false;
    }
  };
}

char DependencyGraphGen::ID = 0;
static RegisterPass<DependencyGraphGen> X("dependencyGraph", "Pass to produce a Data Dependency Graph");


