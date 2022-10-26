#include "Pair.h"

Pair::Pair(long left, long top)
  : m_Left(left),
    m_Top(top),
    m_bNil(false)
{
}

Pair::Pair()
  : m_Left(0),
    m_Top(0),
    m_bNil(true)
{
}

Pair::~Pair()
{

}

long Pair::Left() const
{
  return m_Left;
}

long Pair::Top() const
{
  return m_Top;
}

void Pair::SetLeft(long left)
{
  m_Left = left;
}

void Pair::SetTop(long top)
{
  m_Top = top;
}

void Pair::Set(long left, long top)
{
  m_Left = left;
  m_Top = top;
}

bool Pair::IsNil() const
{
  return m_bNil;
}
