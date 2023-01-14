#ifndef RECT_H
#define RECT_H

#include <stddef.h>

/**
 * Represents a simple, geometric rectangle.
 *
 * @author Uwe Rosner
 * @date 10/06/2020
 */
class Rect
{
public:
  Rect();
  Rect(unsigned long left, 
       unsigned long top, 
       unsigned long right, 
       unsigned long bottom);

  Rect(unsigned long left, unsigned long top);

  /**
   * Sets the rectangle to the given position.
   */
  void set(unsigned long left, 
           unsigned long top, 
           unsigned long right, 
           unsigned long bottom);

  /**
   * Sets the left top edge of the rectangle to the new position.
   *
   * NOTE: This is *not* a move. The right bottom edge is not changed,
   * so the width and height of the rectangle will be different after
   * this call.
   */
  void setPosition(unsigned long left, unsigned long top);

  /**
   * Sets the width and height of the rectangle to the given sizes.
   *
   * NOTE: The right bottom edge of the rectangle is changed by this
   * operation. The left top edge is not changed.
   */
  virtual void setSize(unsigned long width, unsigned long height);

  bool hasSize() const;

  /**
   * Returns true if the given point is inside the rectangle.
   */
  bool isPointInside(unsigned long pointX, unsigned long pointY) const;

  unsigned long getArea() const;

  unsigned long getLeft() const;
  unsigned long getRight() const;
  unsigned long getTop() const;
  unsigned long getBottom() const;
  
  unsigned long getHeight() const;
  unsigned long getWidth() const;
  unsigned long getWordWidth() const;

private:
  unsigned long m_Left;
  unsigned long m_Top;
  unsigned long m_Right;
  unsigned long m_Bottom;

  unsigned long m_WordWidth;
};

#endif
