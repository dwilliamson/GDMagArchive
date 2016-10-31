#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <map>
#include <string>
#include <vector>

#pragma warning (disable: 4786)

class TextureManager
{
public:
  static TextureManager* instance();

  // Loads the texture as a w-by-h texture, either 24 bit if there's no alpha, or 32-bit if
  // there is.  Only works for .raw files right now.
  int addTexture(const std::string&, int w, int h, bool alpha, bool repeat = true);

  // This just adds a texture given the data handed to it.  You should use this because then
  // it can clean the texture up properly later, fuss-free.  If alpha is true, we assume it's
  // GL_RGBA, if it's not, we assume GL_RGB.  Either way, we assume a byte-per-component,
  // unpacked.
  int addTexture(unsigned char* data, int w, int h, bool alpha, bool repeat = true);
  int addTexture(float* data, int w, int h, bool alpha, bool repeat = true);

  // This deletes all the textures from OpenGL (invalidating all of them).
  void deleteTextures();

protected:
  std::map<std::string, unsigned int> textureMap;
  std::vector<unsigned int> proceduralTextures;

  static TextureManager* singleton;
};

#endif //TEXTUREMANAGER_H