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

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h" // todo: remove later
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

    bool isCPSCall(const CallSite &CS) {
      Instruction *Inst = CS.getInstruction();
      if (IntrinsicInst *II = dyn_cast<IntrinsicInst>(Inst))
        if (II->getIntrinsicID() == Intrinsic::experimental_cpscall)
          return true;

      return false;
    }

    bool doIt(Function &F) {
      bool Changed = false;

      // find and split at each cpscall
      for (auto BB = F.begin(), BBE = F.end(); BB != BBE; ++BB)
        for (auto II = BB->begin(), IIE = BB->end(); II != IIE; ++II)
          if (auto CS = CallSite(&*II))
            if (isCPSCall(CS)) {
              split(CS);
              Changed = true;
              ++CallsDone;
            }

      return Changed;
    }

    void split(CallSite &CS) {
      CS->dump();
    }
    
  };
}

char CPSCallPrep::ID = 0;
static RegisterPass<CPSCallPrep> X("cpscallprep", "CPS Call codegen preparation");
