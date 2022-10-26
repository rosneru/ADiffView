#ifndef ADIFFVIEW_SETTINGS_H
#define ADIFFVIEW_SETTINGS_H

#include <string>
#include <exec/types.h>


/**
 * Class to store the settings of ADiffView.
 *
 * @author Uwe Rosner
 * @date 09/12/2019
 */
class ADiffViewSettings
{
public:

  /**
   * Creates the settings object.
   * NOTE: The settings have to be loaded before first use.
   */
  ADiffViewSettings();

  virtual ~ADiffViewSettings();

  bool Load();

  /**
   * Return the ColorArray as Amiga-LoadRGB32-compliant array
   */
  const ULONG* GetColorArray() const;

  /**
   * Return the start of the array for color red.
   * 
   * Use it with the indexes [0], [1], [2] for single rgb components.
   */
  const ULONG* GetColorRedArray() const;

  /**
   * Return the start of the array for color green.
   * 
   * Use it with the indexes [0], [1], [2] for single rgb components.
   */
  const ULONG* GetColorGreenArray() const;

  /**
   * Return the start of the array for color yellow.
   * 
   * Use it with the indexes [0], [1], [2] for single rgb components.
   */
  const ULONG* GetColorYellowArray() const;

  /**
   * Return the start of the array for color gray.
   * 
   * Use it with the indexes [0], [1], [2] for single rgb components.
   */
  const ULONG* GetColorGrayArray() const;

private:
  // It's useful to store the colors at a central place
  // This struct is a workaround for C++98 not allowing array 
  // initialization in th initializer list.
  //
  // It is defined to a size of 11 because:
  //   1..Header, 
  //   3x3=9..the 3 colors, 
  //   1..Termination
  //
  struct ColorArray
  { 
    ULONG elem[14];
  };

  // Declaring a static variable to hold the ColorArray data.
  // See top of the implementation file, there it is defined
  // and initialized
  static ColorArray m_sColors;

  // Declaring a member variable for the Color array. In the
  // constructor it is initialized with the static array data.
  ColorArray m_ColorArray;
};

#endif // ADIFFVIEW_SETTINGS_H
