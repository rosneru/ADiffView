#ifndef DIFF_FILE_PARTITION_H
#define DIFF_FILE_PARTITION_H

#include "stdio.h"
#include <vector>

#include "DiffLine.h"


/**
 * Base class for storing a file line by line with diff information.
 *
 * @author Uwe Rosner
 * @date 28/09/2018
 */
class DiffFileBase
{
public:
  DiffFileBase();

  size_t getNumLines() const;

  const DiffLine* operator[](unsigned long index) const;
  DiffLine* operator[](unsigned long index);

  const std::vector<DiffLine*>& getLines() const;

protected:
  std::vector<DiffLine*> m_Lines;

  size_t numDigits(size_t number);


private:
  const char m_EmptyText;
};

#endif
