#ifndef DIFF_ENGINE_H
#define DIFF_ENGINE_H

#include <list>

#include "DiffLine.h"
#include "DiffInputFileBase.h"
#include "DiffOutputFileBase.h"
#include "Pair.h"
#include "ProgressReporter.h"

/**
 * Class for generating and representing the difference of two files.
 *
 * This is an implementation of the Myers diff algorithm. Part of it is
 * ported from a c# solution I found in the web.
 *
 * The attached license of the source:
 *
 *      Copyright (c) 2005-2012 by Matthias Hertel, http://www.mathertel.de/
 *      All rights reserved.
 *
 *      Redistribution and use in source and binary forms, with or
 *      without modification, are permitted provided that the following
 *      conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 *      - Neither the name of the copyright owners nor the names of its
 *        contributors may be used to endorse or promote products
 *        derived from this software without specific prior written
 *        permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @author Uwe Rosner
 * @date 18/07/2019
 */
class DiffEngine
{

public:
  /**
   * Creates the DiffEngine with the given input and output files. On
   * creation the diff is calculated automatically.
   * 
   * Throws an exception on user abort.
   */
  DiffEngine(DiffInputFileBase& leftInFile,
             DiffInputFileBase& rightInFile,
             DiffOutputFileBase& leftOutFile,
             DiffOutputFileBase& rightOutFile,
             ProgressReporter& progress,
             const char* pProgressDescription,
             bool& isCancelRequested,
             std::list<size_t>& diffIndices,
             bool countDifferencesByLines = false);

  virtual ~DiffEngine();

  /**
   * Start comparing leftInFile and rightInFile. Generates the contents
   * of leftOutFile and rightOutFile.
   */
  bool startCompare();

  long getNumDifferences() const;

  long getNumAdded() const;
  long getNumChanged() const;
  long getNumDeleted() const;

protected:
  std::list<size_t>& m_DiffIndices;

private:
  DiffInputFileBase& m_LeftInFile;
  DiffInputFileBase& m_RightInFile;
  DiffOutputFileBase& m_LeftOutFile;
  DiffOutputFileBase& m_RightOutFile;

  bool& m_IsCancelRequested;
  ProgressReporter& m_Progress;

  bool m_CountDifferencesByLines;

  long m_NumInsertedB;
  long m_NumDeletedA;
  long m_NumChanged;

  long m_Max;
  std::vector<long> m_pDownVector;
  std::vector<long> m_pUpVector;

  // The next four fields are for progress notification
  long m_Percent;
  long m_PercentIncrement;
  long m_NotifyIncrement;
  long m_NextNotifyPosition;


  /**
   * Calculate the target DiffFiles from src diff files.
   *
   * That means inserting of empty lines in one side when in other
   * side are insertions or Deletions.
   *
   * Also, if a pDiffStartIdxsList were given in the constructor,
   * it is filled with the start position of each difference block
   * during this step.
   */
  void populateOutputFiles();


  /**
   * The divide-and-conquer implementation of the longest-common-
   * subsequence (LCS) algorithm. The published algorithm passes
   * recursively parts of the A and B sequences. To avoid copying
   * these arrays the lower and upper bounds are passed while the
   * sequences stay constant.
   */
  void lcs(long lowerA, long upperA, long lowerB, long upperB);


  /**
   * This is the algorithm to find the Shortest Middle Snake (SMS)
   */
  Pair sms(long lowerA, long upperA, long lowerB, long upperB);


  /**
   * If a sequence of modified lines starts with a line that
   * contains the same content as the line that appends the
   * changes, the difference sequence is modified so that the
   * appended line and not the starting line is marked as modified.
   *
   * This leads to more readable diff sequences when comparing
   * text files.
   */
  void optimize(DiffFileBase& diffFile);
};

#endif
