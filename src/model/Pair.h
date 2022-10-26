#ifndef PAIR_H
#define PAIR_H


/**
 * A class that represents a Pair.
 *
 * @author Uwe Rosner
 * @date 23/10/2019
 */
class Pair
{
public:
  Pair(long left, long top);
  Pair();
  virtual ~Pair();

  long Left() const;
  long Top() const;

  void SetLeft(long left);
  void SetTop(long top);
  void Set(long left, long top);

  bool IsNil() const;

private:
  long m_Left;
  long m_Top;
  bool m_bNil;
};

#endif
