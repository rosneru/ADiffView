#include "DiffOutputFileBase.h"

  DiffOutputFileBase::DiffOutputFileBase(const DiffInputFileBase& diffInputFile)
    : m_pEmptyLine(""),
      m_EmptyLineNumber(numDigits(diffInputFile.getNumLines()) + 1, ' ')
  {

  }

DiffOutputFileBase::~DiffOutputFileBase()
{

}


void DiffOutputFileBase::addEmptyLine()
{
  addLine(m_pEmptyLine, DiffLine::Normal, m_EmptyLineNumber.c_str());
}
