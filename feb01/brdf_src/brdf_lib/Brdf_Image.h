#ifndef BRDF_IMAGE_H
#define BRDF_IMAGE_H

#include "global.h"
#include "Image.h"

class ifstream;
class ofstream;

class Brdf_Image : public Image {
  public:
    enum SaveRange { range_m1_1 = 0, range_0_1 = 1 };

    Brdf_Image(int width, int height);
    Brdf_Image(char *filename);
    Brdf_Image(ifstream &stream);
    ~Brdf_Image();

    void construct(ifstream &stream);
    void setSaveRange(SaveRange sr);

    virtual bool save(char *name);
    bool save(ofstream &out);

  protected:
    SaveRange saveRange;
};

#endif // BRDF_IMAGE_H
