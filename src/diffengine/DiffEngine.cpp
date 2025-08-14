#include <stdio.h>

#include "DiffEngine.h"

DiffEngine::DiffEngine(DiffInputFileBase& leftInFile,
                       DiffInputFileBase& rightInFile,
                       DiffOutputFileBase& leftOutFile,
                       DiffOutputFileBase& rightOutFile,
                       ProgressReporter& progress,
                       const char* pProgressDescription,
                       bool& isCancelRequested,
                       std::list<size_t>& diffIndices,
                       bool countDifferencesByLines)
  : m_DiffIndices(diffIndices),
    m_LeftInFile(leftInFile),
    m_RightInFile(rightInFile),
    m_LeftOutFile(leftOutFile),
    m_RightOutFile(rightOutFile),
    m_IsCancelRequested(isCancelRequested),
    m_Progress(progress),
    m_CountDifferencesByLines(countDifferencesByLines),
    m_NumInsertedB(0),
    m_NumDeletedA(0),
    m_NumChanged(0),
    m_Max(m_LeftInFile.getNumLines() + m_RightInFile.getNumLines() + 1),
    m_DownVector(2 * m_Max + 2),
    m_UpVector(2 * m_Max + 2)
{ 
  m_Progress.SetDescription(pProgressDescription);

  //
  // Set-up the progress reporting
  //
  const long NUM_NOTIFICATIONS = 18;
  long numPositions = m_LeftInFile.getNumLines();
  m_NotifyIncrement = numPositions / NUM_NOTIFICATIONS;
  m_PercentIncrement = 90 / NUM_NOTIFICATIONS;
  m_NextNotifyPosition = m_NotifyIncrement;
  m_Percent = 0;

  m_Progress.SetValue(0);

  m_NumInsertedB = 0;
  m_NumDeletedA = 0;
  m_NumChanged = 0;
}

bool DiffEngine::startCompare()
{
  //
  // Calculate the Longest common subsequence
  //   While calculating the lcs the deleted lines in left file m_A are
  //   marked with DiffLine::Deleted and the inserted lines in right
  //   file m_b are marked with DiffLine::Added
  lcs(0, m_LeftInFile.getNumLines(), 0, m_RightInFile.getNumLines());

  if(m_IsCancelRequested)
  {
    return false;
  }

  m_Progress.SetValue(90);

  // Optimize the output files for better readable diff blocks
  // TODO: Mustn't it be done after populateOutputFiles() ???
  optimize(m_LeftOutFile);
  optimize(m_RightOutFile);

  m_Progress.SetValue(95);

  // Calculate the output files that contain the differences. That means
  // inserting of empty lines in one side when in other side are
  // insertions or Deletions.
  populateOutputFiles();

  m_Progress.SetValue(100);

  // If user canceled the compare operation no success is reported.
  return !m_IsCancelRequested;
}

DiffEngine::~DiffEngine()
{

}


long DiffEngine::getNumDifferences() const
{
  return m_NumChanged + m_NumDeletedA + m_NumInsertedB;
}


long DiffEngine::getNumAdded() const
{
  return m_NumInsertedB;
}


long DiffEngine::getNumChanged() const
{
  return m_NumChanged;
}


long DiffEngine::getNumDeleted() const
{
  return m_NumDeletedA;
}


void DiffEngine::populateOutputFiles()
{
  size_t lineA = 0;
  size_t lineB = 0;

  // Clearing these variables as from now on they should not count
  //single lines anymore. Instead they should count difference blocks
  // of according type. They will be set anew below.
  m_NumInsertedB = 0;
  m_NumDeletedA = 0;
  m_NumChanged = 0;

  while (lineA < m_LeftInFile.getNumLines() || lineB < m_RightInFile.getNumLines())
  {
    //
    // Handle the equal lines
    //
    while( (lineA < m_LeftInFile.getNumLines())
     && (m_LeftInFile[lineA]->getState() == DiffLine::Normal)
     && (lineB < m_RightInFile.getNumLines())
     && (m_RightInFile[lineB]->getState() == DiffLine::Normal))
    {
      if(m_IsCancelRequested)
      {
        return;
      }

      const DiffLine* pA = m_LeftInFile[lineA++];
      const DiffLine* pB = m_RightInFile[lineB++];

      m_LeftOutFile.addLine(pA->getText(), DiffLine::Normal, pA->getLineNumText());
      m_RightOutFile.addLine(pB->getText(), DiffLine::Normal, pB->getLineNumText());
    }

    //
    // Handle changed, deleted, inserted lines
    //
    bool bBlockAlreadyAdded = false;
    while((lineA < m_LeftInFile.getNumLines())
      && (lineB < m_RightInFile.getNumLines())
      && (m_LeftInFile[lineA]->getState() != DiffLine::Normal)
      && (m_RightInFile[lineB]->getState() != DiffLine::Normal))
    {
      if(m_IsCancelRequested)
      {
        return;
      }
      const DiffLine* pA = m_LeftInFile[lineA++];
      const DiffLine* pB = m_RightInFile[lineB++];

      long idx = m_LeftOutFile.addLine(pA->getText(), DiffLine::Changed, pA->getLineNumText());
      m_RightOutFile.addLine(pB->getText(), DiffLine::Changed, pB->getLineNumText());

      if(!bBlockAlreadyAdded)
      {
        // Add start of this block of CHANGED lines to differences list
        m_NumChanged++;
        m_DiffIndices.push_back(idx); // -2 ... has been decremented above

        if(!m_CountDifferencesByLines)
        { 
          bBlockAlreadyAdded = true;
        }
      }
    }

    bBlockAlreadyAdded = false;
    while((lineA < m_LeftInFile.getNumLines())
       && (lineB >= m_RightInFile.getNumLines() || (m_LeftInFile[lineA]->getState() != DiffLine::Normal)))
    {
      if(m_IsCancelRequested)
      {
        return;
      }

      const DiffLine* pA = m_LeftInFile[lineA++];
      long idx = m_LeftOutFile.addLine(pA->getText(), DiffLine::Deleted, pA->getLineNumText());
      m_RightOutFile.addEmptyLine();

      if(!bBlockAlreadyAdded)
      {
        // Add start of this block of DELETED lines to differences list
        m_NumDeletedA++;
        m_DiffIndices.push_back(idx);

        if(!m_CountDifferencesByLines)
        { 
          bBlockAlreadyAdded = true;
        }
      }
    }

    bBlockAlreadyAdded = false;
    while((lineB < m_RightInFile.getNumLines())
      && (lineA >= m_LeftInFile.getNumLines() || (m_RightInFile[lineB]->getState() != DiffLine::Normal)))
    {
      if(m_IsCancelRequested)
      {
        return;
      }

      const DiffLine* pB = m_RightInFile[lineB++];
      m_LeftOutFile.addEmptyLine();
      long idx = m_RightOutFile.addLine(pB->getText(), DiffLine::Added, pB->getLineNumText());

      if(!bBlockAlreadyAdded)
      {
        // Add start of this block of ADDED lines to differences list
        m_NumInsertedB++;
        m_DiffIndices.push_back(idx);

        if(!m_CountDifferencesByLines)
        { 
          bBlockAlreadyAdded = true;
        }
      }
    }
  }
}



void DiffEngine::lcs(long lowerA, long upperA, long lowerB, long upperB)
{
  //
  // Notify
  //
  if((m_NotifyIncrement > 0)
   &&(lowerA > m_NextNotifyPosition))
  {
    if(m_IsCancelRequested)
    {
      return;
    }

    m_Percent += m_PercentIncrement;
    m_NextNotifyPosition += m_NotifyIncrement;
    m_Progress.SetValue(m_Percent);
  }

  // Fast walkthrough equal lines at the start
  while((lowerA < upperA) && (lowerB < upperB)
     && (m_LeftInFile[lowerA]->getToken() == m_RightInFile[lowerB]->getToken()))
  {
    lowerA++;
    lowerB++;
  }

  // Fast walkthrough equal lines at the end
  while((lowerA < upperA) && (lowerB < upperB)
     && (m_LeftInFile[upperA - 1]->getToken() == m_RightInFile[upperB - 1]->getToken()))
  {
    --upperA;
    --upperB;
  }

  if(lowerA == upperA)
  {
    while(lowerB < upperB)
    {
      m_RightInFile[lowerB++]->setState(DiffLine::Added);
      m_NumInsertedB++;
    }
  }
  else if(lowerB == upperB)
  {
    while(lowerA < upperA)
    {
      m_LeftInFile[lowerA++]->setState(DiffLine::Deleted);
      m_NumDeletedA++;
    }
  }
  else
  {
    if(m_IsCancelRequested)
    {
      return;
    }

    Pair smsrd = sms(lowerA, upperA, lowerB, upperB);

    if(m_IsCancelRequested)
    {
      return;
    }

    lcs(lowerA, smsrd.Left(), lowerB, smsrd.Top());
    lcs(smsrd.Left(), upperA, smsrd.Top(), upperB);
  }
}


// Pair DiffEngine::sms(long lowerA, long upperA, long lowerB, long upperB)
// {
//   Pair result;

//   // the k-line to start the forward search
//   long downK = lowerA - lowerB;

//   // the k-line to start the reverse search
//   long upK = upperA - upperB;

//   long delta = (upperA - lowerA) - (upperB - lowerB);
//   bool bOddDelta = (delta & 1) != 0;

//   // The vectors in Myers' publication accept negative indexes.
//   // The vectors implemented here are 0-based and are accessed using
//   // a specific offset: UpOffset for m_pUpVector and DownOffset for
//   // m_pDownVector
//   long downOffset = m_Max - downK;
//   long upOffset = m_Max - upK;

//   long maxD = ((upperA - lowerA + upperB - lowerB) / 2) + 1;

//   // init vectors
//   m_DownVector[downOffset + downK + 1] = lowerA;
//   m_UpVector[upOffset + upK - 1] = upperA;

//   for (long D = 0; D <= maxD; D++)
//   {
//     if(m_IsCancelRequested)
//     {
//       return result;
//     }

//     // Extend the forward path
//     for (long k = downK - D; k <= downK + D; k += 2)
//     {
//       // find the only or better starting point
//       long x, y;
//       const long dk = downOffset + k;
//       const long dk_plus_1 = dk + 1;

//       if (k == downK - D)
//       {
//         x = m_DownVector[dk_plus_1];  // down
//       }
//       else
//       {
//         x = m_DownVector[dk - 1] + 1; // a step to the right

//         if ((k < downK + D) && (m_DownVector[dk_plus_1] >= x))
//         {
//           x = m_DownVector[dk_plus_1]; // down
//         }
//       }

//       y = x - k;

//       // find the end of the furthest reaching forward D-path in diagonal k.
//       while ((x < upperA) && (y < upperB) &&
//              (m_LeftInFile[x]->getToken() == m_RightInFile[y]->getToken()))
//           //&& (m_A[x]->Text() == m_B[y]->Text()))
//       {
//         x++;
//         y++;
//       }

//       m_DownVector[dk] = x;

//       // overlap ?
//       if (bOddDelta && (upK - D < k) && (k < upK + D))
//       {
//         if (m_UpVector[upOffset + k] <= m_DownVector[dk])
//         {
//           result.Set(m_DownVector[dk], m_DownVector[dk] - k);
//           return result;
//         }
//       }
//     }

//     // Extend the reverse path.
//     for (long k = upK - D; k <= upK + D; k += 2)
//     {
//       // find the only or better starting point
//       long x, y;
//       const long dk = downOffset + k;
//       const long uk = upOffset + k;
//       const long uk_minus_1 = uk - 1;

//       if (k == upK + D)
//       {
//         x = m_UpVector[uk_minus_1];   // up
//       }
//       else
//       {
//         x = m_UpVector[uk + 1] - 1;   // left

//         if ((k > upK - D) && (m_UpVector[uk_minus_1] < x))
//         {
//           x = m_UpVector[uk_minus_1]; // up
//         }
//       }

//       y = x - k;

//       while ((x > lowerA) && (y > lowerB) &&
//              (m_LeftInFile[x - 1]->getToken() == m_RightInFile[y - 1]->getToken()))
//           //&& (m_A[x - 1]->Text() == m_B[y - 1]->Text()))
//       {
//         // diagonal
//         x--;
//         y--;
//       }

//       m_UpVector[uk] = x;

//       // overlap ?
//       if (!bOddDelta && (downK - D <= k) && (k <= downK + D))
//       {
//         if (m_UpVector[uk] <= m_DownVector[dk])
//         {
//           result.Set(m_DownVector[dk], m_DownVector[dk] - k);
//           return result;
//         }
//       }
//     }
//   }

//   // The algorithm should never come here
//   Pair resultInvalid;
//   return resultInvalid;
// }
Pair DiffEngine::sms(long lowerA, long upperA, long lowerB, long upperB)
{
    Pair result;

    long downK   = lowerA - lowerB;
    long upK     = upperA - upperB;
    long delta   = (upperA - lowerA) - (upperB - lowerB);
    bool bOddDelta = (delta & 1) != 0;

    long downOffset = m_Max - downK;
    long upOffset   = m_Max - upK;

    long maxD = ((upperA - lowerA + upperB - lowerB) / 2) + 1;

    // Raw pointers auf die Vektor-Daten mit Offset
    long* downData = m_DownVector.data() + downOffset;
    long* upData   = m_UpVector.data()   + upOffset;

    // Initialisierung
    downData[downK + 1] = lowerA;
    upData[upK - 1]     = upperA;

    for (long D = 0; D <= maxD; D++)
    {
        if (m_IsCancelRequested)
            return result;

        // Vorwärts-Erweiterung
        for (long k = downK - D; k <= downK + D; k += 2)
        {
            long x, y;

            if (k == downK - D)
            {
                x = downData[k + 1];
            }
            else
            {
                // Schritt nach rechts
                x = downData[k - 1] + 1;

                // Schritt nach unten, falls besser
                if ((k < downK + D) && (downData[k + 1] >= x))
                    x = downData[k + 1];
            }

            y = x - k;

            // Diagonales Vorankommen
            while (x < upperA && y < upperB &&
                   m_LeftInFile[x]->getToken() == m_RightInFile[y]->getToken())
            {
                ++x;
                ++y;
            }

            downData[k] = x;

            // Überlappung prüfen (odd Delta)
            if (bOddDelta && (upK - D < k) && (k < upK + D))
            {
                if (upData[k] <= downData[k])
                {
                    result.Set(downData[k], downData[k] - k);
                    return result;
                }
            }
        }

        // Rückwärts-Erweiterung
        for (long k = upK - D; k <= upK + D; k += 2)
        {
            long x, y;

            if (k == upK + D)
            {
                x = upData[k - 1];
            }
            else
            {
                // Schritt nach links
                x = upData[k + 1] - 1;

                // Schritt nach oben, falls besser
                if ((k > upK - D) && (upData[k - 1] < x))
                    x = upData[k - 1];
            }

            y = x - k;

            // Diagonales Zurücklaufen
            while (x > lowerA && y > lowerB &&
                   m_LeftInFile[x - 1]->getToken() == m_RightInFile[y - 1]->getToken())
            {
                --x;
                --y;
            }

            upData[k] = x;

            // Überlappung prüfen (even Delta)
            if (!bOddDelta && (downK - D <= k) && (k <= downK + D))
            {
                if (upData[k] <= downData[k])
                {
                    result.Set(downData[k], downData[k] - k);
                    return result;
                }
            }
        }
    }

    // sollte nie erreicht werden
    return Pair();
}



void DiffEngine::optimize(DiffFileBase& diffFile)
{
  size_t dataLength = diffFile.getNumLines();
  size_t startPos = 0;
  size_t endPos = 0;

  while(startPos < diffFile.getNumLines())
  {
    while((startPos < dataLength)
       && (diffFile[startPos]->getState() == DiffLine::Normal))  // normal
    {
      startPos++;
    }

    endPos = startPos;

    while((endPos < dataLength)
      && (diffFile[startPos]->getState() != DiffLine::Normal)) // modified
    {
      endPos++;
    }

    if((endPos < dataLength)
     && (diffFile[startPos]->getToken() == diffFile[endPos]->getToken()))
    {
      diffFile[endPos]->setState(diffFile[startPos]->getState());
      diffFile[startPos]->setState(DiffLine::Normal);
    }
    else
    {
      startPos = endPos;
    }
  }
}
