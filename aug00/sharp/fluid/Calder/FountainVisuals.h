#ifndef FOUNTAINVISUALS_H
#define FOUNTAINVISUALS_H

//
// FountainVisuals
//
// This is the class that deals with the various nastiness of drawing the Calder mercury fountain model.
// Whether it be hardcoded into the program or whether it be through a nice model loader nobody but this
// guy will ever know (and that's probably a good thing.)
//

class FountainVisuals
{
public:
  FountainVisuals();

  void draw();

protected:
  unsigned int fountainBaseTexture;
  unsigned int fountainTopTexture;
};

#endif //FOUNTAINVISUALS_H