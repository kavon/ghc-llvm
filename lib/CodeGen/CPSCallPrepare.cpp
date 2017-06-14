//===- CPSCallPrepare.cpp - Prepare cpscall intrinsic for code generation -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Describe
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

#define DEBUG_TYPE "cpscallprep"

STATISTIC(CallsDone, "Number of cpscall intrinsics processed");

namespace {
  struct CPSCallPrep : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    CPSCallPrep() : FunctionPass(ID) {}

    bool skipFunction(Function &F) {
      return F.isDeclaration() || F.empty();
    }

    bool runOnFunction(Function &F) override {
      if (skipFunction(F))
        return false;

      return doIt(F);
    }

    bool doIt(Function &F) {
      bool Changed = false;
      // find and split at each cpscall
      for (auto BBI = F.begin(), BBE = F.end(); BBI != BBE; ++BBI) {
        BasicBlock *BB = &*BBI;
        auto II = BB->begin();
        auto IIE = BB->end();
        
        while (II != IIE) {
          Instruction &Cursor = *II++;

            if (isCPSCall(Cursor)) {
                // perform the split
                Instruction &NextInstr = *II++;
                BasicBlock *NewBB = SplitBlock(BB, &NextInstr);

                // update iterators to continue processing
                // the rest of the split block. 
                BBI = NewBB->getIterator();
                II = NewBB->begin();
                IIE = NewBB->end();
                BB = NewBB;

                ++CallsDone;
                Changed = true;
            }
        }
      }

      return Changed;
    }

    bool isCPSCall(Instruction &Cursor) {
      if (auto CS = CallSite(&Cursor))
        if (IntrinsicInst *II = dyn_cast<IntrinsicInst>(CS.getInstruction()))
          if (II->getIntrinsicID() == Intrinsic::experimental_cpscall)
            return true;

      return false;
    }

  };
}

char CPSCallPrep::ID = 0;
static RegisterPass<CPSCallPrep> X("cpscallprep", "CPS Call codegen preparation");

FunctionPass *llvm::createCPSCallPrepPass() { return new CPSCallPrep(); }
