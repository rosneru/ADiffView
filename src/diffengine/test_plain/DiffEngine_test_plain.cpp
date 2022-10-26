#include "DiffFileBase.h"
#include "DiffFileLinux.h"
#include "DiffEngine.h"
#include "DiffLine.h"

#include <stdio.h>

int main()
{
  bool cancelRequested = false;
  bool diffOk = false;

  DiffFileLinux srcA(cancelRequested);
  srcA.PreProcess("../../../testfiles/testcase_25_startup-sequence_left.txt");
//  srcA.PreProcess("../../../testfiles/testcase_13_6000_left.cpp");

  DiffFileLinux srcB(cancelRequested);
  srcB.PreProcess("../../../testfiles/testcase_25_startup-sequence_right.txt");
//  srcB.PreProcess("../../../testfiles/testcase_13_6000_right.cpp");

  DiffFileLinux targetA(cancelRequested);
  DiffFileLinux targetB(cancelRequested);

  DiffEngine diffEngine(srcA, srcB, targetA, targetB, cancelRequested);
  diffOk = diffEngine.Diff();
  
  if(!diffOk)
  {
    printf("Fehler beim Berechnen des Diff.\n");
    return -30;
  }

  printf("Diff erfolgreich\n");

  return 0;
}

