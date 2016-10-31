// Stupid STL warning.
#pragma warning (disable: 4786)

#include <harness/TextureManager.h>
#include <harness/OpenGL.h>
#include <gl/glut.h>
#include <math/Math.h>

TextureManager* TextureManager::singleton = NULL;

TextureManager* TextureManager::instance()
{
  if (singleton == NULL)
  {
    singleton = new TextureManager();
  }
  return singleton;
}

// Loads the texture as a w-by-h texture, either 24 bit if there's no alpha, or 32-bit if
// there is.  Only works for .raw files right now.
int TextureManager::addTexture(const std::string& rawFile, int w, int h, bool alpha, bool repeat)
{
  if (textureMap.find(rawFile) != textureMap.end())
  {
    return textureMap[rawFile];
  }

  int depth = alpha ? 4 : 3;

  FILE* texFile = fopen( rawFile.c_str(), "rb" );
  char* texData = new char[ w * h * depth ];

  unsigned int texNum;
  glGenTextures( 1, &texNum ); 

  if ( texFile != NULL )
  {                                   
    fread( texData, sizeof( char ), w*h*depth, texFile );

    // Scale down textures to the largest size value; this is mainly for old 3dfx cards that only support
    // 256x256 textures.
    int maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    if (maxTextureSize < w || maxTextureSize < h)
    {
      int newW = Math::minOf(w, maxTextureSize);
      int newH = Math::minOf(h, maxTextureSize);
     
      char* newTexData = new char[newW*newH*depth];
      gluScaleImage((alpha ? GL_RGBA : GL_RGB), w, h, GL_UNSIGNED_BYTE, texData, newW, newH, GL_UNSIGNED_BYTE, newTexData);
      delete[] texData;
      texData = newTexData;
      w = newW;
      h = newH;
    }

    fclose( texFile );

    glBindTexture( GL_TEXTURE_2D, texNum );
  
    // Set the tiling mode
    if (repeat)
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    else
    {
      // Clamp in the best way available.
      if (OpenGL::getSupportsClampToEdge())
      {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGL::GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGL::GL_CLAMP_TO_EDGE);
      }
      else
      {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      }
    }
    
    // Set the filtering.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    gluBuild2DMipmaps( GL_TEXTURE_2D, depth, w, h, (alpha ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, texData );
  }

  delete[] texData;

  textureMap[rawFile] = texNum;

  return texNum;
}

// This just adds a texture given the data handed to it.  You should use this because then
// it can clean the texture up properly later, fuss-free.  If alpha is true, we assume it's
// GL_RGBA, if it's not, we assume GL_RGB.  Either way, we assume a byte-per-component,
// unpacked.
int TextureManager::addTexture(unsigned char* texDataIn, int w, int h, bool alpha, bool repeat)
{
  int depth = alpha ? 4 : 3;

  unsigned char* texData;

  // Scale down textures to the largest size value; this is mainly for old 3dfx cards that only support
  // 256x256 textures.
  int maxTextureSize;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
  if (maxTextureSize < w || maxTextureSize < h)
  {
    int newW = Math::minOf(w, maxTextureSize);
    int newH = Math::minOf(h, maxTextureSize);
   
    texData = new unsigned char[newW*newH*depth];
    gluScaleImage((alpha ? GL_RGBA : GL_RGB), w, h, GL_UNSIGNED_BYTE, texDataIn, newW, newH, GL_UNSIGNED_BYTE, texData);
    w = newW;
    h = newH;
  }
  else
  {
    texData = new unsigned char[w*h*depth];
    memcpy(texData, texDataIn, w*h*depth);
  }

  unsigned int texNum;
  glGenTextures( 1, &texNum ); 

  glBindTexture( GL_TEXTURE_2D, texNum );

  // Set the tiling mode
  if (repeat)
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
  else
  {
    // Clamp in the best way available.
    if (OpenGL::getSupportsClampToEdge())
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGL::GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGL::GL_CLAMP_TO_EDGE);
    }
    else
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
  }
  
  // Set the filtering.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

  gluBuild2DMipmaps( GL_TEXTURE_2D, depth, w, h, (alpha ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, texData );

  proceduralTextures.push_back(texNum);

  delete[] texData;

  return texNum;
}

int TextureManager::addTexture(float* texData, int w, int h, bool alpha, bool repeat)
{
  int depth = alpha ? 4 : 3;

  unsigned int texNum;
  glGenTextures( 1, &texNum ); 

  glBindTexture( GL_TEXTURE_2D, texNum );

  // Set the tiling mode
  if (repeat)
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
  else
  {
    // Clamp in the best way available.
    if (OpenGL::getSupportsClampToEdge())
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGL::GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGL::GL_CLAMP_TO_EDGE);
    }
    else
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
  }
  
  // Set the filtering.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

  gluBuild2DMipmaps( GL_TEXTURE_2D, depth, w, h, (alpha ? GL_RGBA : GL_RGB), GL_FLOAT, texData );

  proceduralTextures.push_back(texNum);

  return texNum;
}

void TextureManager::deleteTextures()
{
  std::map<std::string, unsigned int>::iterator it;
  for (it = textureMap.begin(); it != textureMap.end(); it++)
  {
    glDeleteTextures(1, &(it->second));
  }

  for (int x=0; x<proceduralTextures.size(); x++)
  {
    glDeleteTextures(1, &(proceduralTextures[x]));
  }
}
