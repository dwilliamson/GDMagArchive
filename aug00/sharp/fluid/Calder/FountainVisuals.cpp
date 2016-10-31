#pragma warning (disable: 4786)

#include <fluid/calder/FountainVisuals.h>
#include <harness/TextureManager.h>
#include <math/Vector.h>
#include <gl/glut.h>

#include <fluid/calder/visualBase.cpp>
#include <fluid/calder/visualImmobileTop.cpp>
#include <fluid/calder/visualMobileTop.cpp>

const int fountainNumPieces = 14;

int fountainNumVerts[fountainNumPieces] = {fountainNumVerts0, fountainNumVerts1, fountainNumVerts2, 
                                           fountainNumVerts3, fountainNumVerts4, fountainNumVerts5, 
                                           fountainNumVerts6, fountainNumVerts7, fountainNumVerts8, 
                                           fountainNumVerts9, fountainNumVerts10, fountainNumVerts11,
                                           fountainNumVerts12, visualBaseNumVerts };

float* fountainVerts[fountainNumPieces] = {fountainVerts0, fountainVerts1, fountainVerts2, fountainVerts3,
                                           fountainVerts4, fountainVerts5, fountainVerts6, fountainVerts7,
                                           fountainVerts8, fountainVerts9, fountainVerts10, fountainVerts11,
                                           fountainVerts12, visualBaseVerts };

float* fountainNorms[fountainNumPieces] = {fountainNorms0, fountainNorms1, fountainNorms2, fountainNorms3,
                                           fountainNorms4, fountainNorms5, fountainNorms6, fountainNorms7,
                                           fountainNorms8, fountainNorms9, fountainNorms10, fountainNorms11,
                                           fountainNorms12, visualBaseNorms };

unsigned char* fountainColrs[fountainNumPieces] = {fountainColrs0, fountainColrs1, fountainColrs2, 
                                                   fountainColrs3, fountainColrs4, fountainColrs5, 
                                                   fountainColrs6, fountainColrs7, fountainColrs8, 
                                                   fountainColrs9, fountainColrs10, fountainColrs11,
                                                   fountainColrs12, visualBaseColrs };

int fountainNumIndices[fountainNumPieces] = {fountainNumIndices0, fountainNumIndices1, fountainNumIndices2,
                                             fountainNumIndices3, fountainNumIndices4, fountainNumIndices5,
                                             fountainNumIndices6, fountainNumIndices7, fountainNumIndices8,
                                             fountainNumIndices9, fountainNumIndices10, fountainNumIndices11,
                                             fountainNumIndices12, visualBaseNumIndices };

unsigned int* fountainIndices[fountainNumPieces] = {fountainIndices0, fountainIndices1, fountainIndices2,
                                                    fountainIndices3, fountainIndices4, fountainIndices5,
                                                    fountainIndices6, fountainIndices7, fountainIndices8,
                                                    fountainIndices9, fountainIndices10, fountainIndices11,
                                                    fountainIndices12, visualBaseIndices };

FountainVisuals::FountainVisuals()
{
  fountainBaseTexture = TextureManager::instance()->addTexture(std::string("data/fountainBase.raw"), 512, 512, false, false);
  fountainTopTexture = TextureManager::instance()->addTexture(std::string("data/fountainTop.raw"), 512, 512, false, false);

  // All the faces are backwards.  Fix them.
  for (int y=0; y<fountainNumPieces; y++)
  {
    for (int x=0; x<fountainNumIndices[y]/3; x++)
    {
      int swap = fountainIndices[y][3*x+0];
      fountainIndices[y][3*x+0] = fountainIndices[y][3*x+2];
      fountainIndices[y][3*x+2] = swap;
    }
  }

  for (int x=0; x<fountainNumVerts0; x++)
  {
    fountainVerts0[3*x+2] += fountainVerts0Offset;
  }
  for (x=0; x<fountainNumVerts7; x++)
  {
    fountainVerts7[3*x+2] += fountainVerts7Offset;
  }

  for (y=0; y<fountainNumPieces; y++)
  {
    for (x=0; x<fountainNumVerts[y]*3; x++)
    {
      fountainVerts[y][x] *= 7.0f;
      fountainNorms[y][x] = 0;
    }
  }

  // Generate normals for lighting.
  for (y=0; y<fountainNumPieces; y++)
  {
    for (x=0; x<fountainNumIndices[y]/3; x++)
    {
      // Calc the facet normal.
      Vector p0(fountainVerts[y][3*fountainIndices[y][3*x+0]+0], fountainVerts[y][3*fountainIndices[y][3*x+0]+1], fountainVerts[y][3*fountainIndices[y][3*x+0]+2]);
      Vector p1(fountainVerts[y][3*fountainIndices[y][3*x+1]+0], fountainVerts[y][3*fountainIndices[y][3*x+1]+1], fountainVerts[y][3*fountainIndices[y][3*x+1]+2]);
      Vector p2(fountainVerts[y][3*fountainIndices[y][3*x+2]+0], fountainVerts[y][3*fountainIndices[y][3*x+2]+1], fountainVerts[y][3*fountainIndices[y][3*x+2]+2]);

      Vector facetNormal;
      (p1-p0).cross(p2-p0, facetNormal);

      fountainNorms[y][3*fountainIndices[y][3*x+0]+0] += facetNormal.x;
      fountainNorms[y][3*fountainIndices[y][3*x+0]+1] += facetNormal.y;
      fountainNorms[y][3*fountainIndices[y][3*x+0]+2] += facetNormal.z;

      fountainNorms[y][3*fountainIndices[y][3*x+1]+0] += facetNormal.x;
      fountainNorms[y][3*fountainIndices[y][3*x+1]+1] += facetNormal.y;
      fountainNorms[y][3*fountainIndices[y][3*x+1]+2] += facetNormal.z;

      fountainNorms[y][3*fountainIndices[y][3*x+2]+0] += facetNormal.x;
      fountainNorms[y][3*fountainIndices[y][3*x+2]+1] += facetNormal.y;
      fountainNorms[y][3*fountainIndices[y][3*x+2]+2] += facetNormal.z;
    }
  }

  for (y=0; y<fountainNumPieces; y++)
  {
    for (x=0; x<fountainNumVerts[y]; x++)
    {
      Vector vertNormal(fountainNorms[y][3*x+0], fountainNorms[y][3*x+1], fountainNorms[y][3*x+2]);
      vertNormal.normalize();

      fountainNorms[y][3*x+0] = vertNormal.x;
      fountainNorms[y][3*x+1] = vertNormal.y;
      fountainNorms[y][3*x+2] = vertNormal.z;

      Vector light0 = Vector(-3, -5, 3) * 7;
      Vector light0ToVert = Vector(fountainVerts[y][3*x+0], fountainVerts[y][3*x+1], fountainVerts[y][3*x+2]) - light0;
      light0ToVert.normalize();
      float light0Color[3] = {255*0.75, 180*0.75, 155*0.75};
      float light0Intensity = vertNormal.dot(light0ToVert);
      light0Intensity = Math::maxOf(light0Intensity*-1, 0);
      fountainColrs[y][4*x+0] = (unsigned char)(light0Intensity * light0Color[0]);
      fountainColrs[y][4*x+1] = (unsigned char)(light0Intensity * light0Color[1]);
      fountainColrs[y][4*x+2] = (unsigned char)(light0Intensity * light0Color[2]);
      fountainColrs[y][4*x+3] = 255;

      Vector light1 = Vector(4,-1,4) * 7;
      Vector light1ToVert = Vector(fountainVerts[y][3*x+0], fountainVerts[y][3*x+1], fountainVerts[y][3*x+2]) - light1;
      light1ToVert.normalize();
      float light1Color[3] = {140*0.75, 210*0.75, 255*0.75};
      float light1Intensity = vertNormal.dot(light1ToVert);
      light1Intensity = Math::maxOf(light1Intensity*-1, 0);
      fountainColrs[y][4*x+0] = Math::minOf(fountainColrs[y][4*x+0] + (unsigned char)(light1Intensity * light1Color[0]), 255);
      fountainColrs[y][4*x+1] = Math::minOf(fountainColrs[y][4*x+1] + (unsigned char)(light1Intensity * light1Color[1]), 255);
      fountainColrs[y][4*x+2] = Math::minOf(fountainColrs[y][4*x+2] + (unsigned char)(light1Intensity * light1Color[2]), 255);

      Vector light2 = Vector(1,5,3) * 7;
      Vector light2ToVert = Vector(fountainVerts[y][3*x+0], fountainVerts[y][3*x+1], fountainVerts[y][3*x+2]) - light2;
      light2ToVert.normalize();
      float light2Color[3] = {255*0.5, 255*0.5, 255*0.5};
      float light2Intensity = vertNormal.dot(light2ToVert);
      light2Intensity = Math::maxOf(light2Intensity*-1, 0);
      fountainColrs[y][4*x+0] = Math::minOf(fountainColrs[y][4*x+0] + (unsigned char)(light2Intensity * light2Color[0]), 255);
      fountainColrs[y][4*x+1] = Math::minOf(fountainColrs[y][4*x+1] + (unsigned char)(light2Intensity * light2Color[1]), 255);
      fountainColrs[y][4*x+2] = Math::minOf(fountainColrs[y][4*x+2] + (unsigned char)(light2Intensity * light2Color[2]), 255);
    }
  }
}

void FountainVisuals::draw()
{
  glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT | GL_POINT_BIT);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);

  glEnableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glColor3f(1,1,1);

  glBindTexture(GL_TEXTURE_2D, fountainBaseTexture);

  glVertexPointer(3,GL_FLOAT,12,visualBaseVerts);
  glTexCoordPointer(2,GL_FLOAT,8,visualBaseTexCoords);
  glColorPointer(4,GL_UNSIGNED_BYTE,4,visualBaseColrs);
  glDrawElements(GL_TRIANGLES, visualBaseNumIndices, GL_UNSIGNED_INT, visualBaseIndices);

  glVertexPointer(3,GL_FLOAT,12,fountainVerts5);
  glTexCoordPointer(2,GL_FLOAT,8,fountainTexCoords5);
  glColorPointer(4,GL_UNSIGNED_BYTE,4,fountainColrs5);
  glDrawElements(GL_TRIANGLES, fountainNumIndices5, GL_UNSIGNED_INT, fountainIndices5);

  glVertexPointer(3,GL_FLOAT,12,fountainVerts6);
  glTexCoordPointer(2,GL_FLOAT,8,fountainTexCoords6);
  glColorPointer(4,GL_UNSIGNED_BYTE,4,fountainColrs6);
  glDrawElements(GL_TRIANGLES, fountainNumIndices6, GL_UNSIGNED_INT, fountainIndices6);

  glBindTexture(GL_TEXTURE_2D, fountainTopTexture);

  glVertexPointer(3,GL_FLOAT,12,fountainVerts0);
  glTexCoordPointer(2,GL_FLOAT,8,fountainTexCoords0);
  glColorPointer(4,GL_UNSIGNED_BYTE,4,fountainColrs0);
  glDrawElements(GL_TRIANGLES, fountainNumIndices0, GL_UNSIGNED_INT, fountainIndices0);

  glVertexPointer(3,GL_FLOAT,12,fountainVerts1);
  glTexCoordPointer(2,GL_FLOAT,8,fountainTexCoords1);
  glColorPointer(4,GL_UNSIGNED_BYTE,4,fountainColrs1);
  glDrawElements(GL_TRIANGLES, fountainNumIndices1, GL_UNSIGNED_INT, fountainIndices1);

  glVertexPointer(3,GL_FLOAT,12,fountainVerts2);
  glTexCoordPointer(2,GL_FLOAT,8,fountainTexCoords2);
  glColorPointer(4,GL_UNSIGNED_BYTE,4,fountainColrs2);
  glDrawElements(GL_TRIANGLES, fountainNumIndices2, GL_UNSIGNED_INT, fountainIndices2);

  glVertexPointer(3,GL_FLOAT,12,fountainVerts3);
  glTexCoordPointer(2,GL_FLOAT,8,fountainTexCoords3);
  glColorPointer(4,GL_UNSIGNED_BYTE,4,fountainColrs3);
  glDrawElements(GL_TRIANGLES, fountainNumIndices3, GL_UNSIGNED_INT, fountainIndices3);

  glVertexPointer(3,GL_FLOAT,12,fountainVerts4);
  glTexCoordPointer(2,GL_FLOAT,8,fountainTexCoords4);
  glColorPointer(4,GL_UNSIGNED_BYTE,4,fountainColrs4);
  glDrawElements(GL_TRIANGLES, fountainNumIndices4, GL_UNSIGNED_INT, fountainIndices4);

  glVertexPointer(3,GL_FLOAT,12,fountainVerts7);
  glTexCoordPointer(2,GL_FLOAT,8,fountainTexCoords7);
  glColorPointer(4,GL_UNSIGNED_BYTE,4,fountainColrs7);
  glDrawElements(GL_TRIANGLES, fountainNumIndices7, GL_UNSIGNED_INT, fountainIndices7);

  glVertexPointer(3,GL_FLOAT,12,fountainVerts8);
  glTexCoordPointer(2,GL_FLOAT,8,fountainTexCoords8);
  glColorPointer(4,GL_UNSIGNED_BYTE,4,fountainColrs8);
  glDrawElements(GL_TRIANGLES, fountainNumIndices8, GL_UNSIGNED_INT, fountainIndices8);

  glVertexPointer(3,GL_FLOAT,12,fountainVerts9);
  glTexCoordPointer(2,GL_FLOAT,8,fountainTexCoords9);
  glColorPointer(4,GL_UNSIGNED_BYTE,4,fountainColrs9);
  glDrawElements(GL_TRIANGLES, fountainNumIndices9, GL_UNSIGNED_INT, fountainIndices9);

  glVertexPointer(3,GL_FLOAT,12,fountainVerts10);
  glTexCoordPointer(2,GL_FLOAT,8,fountainTexCoords10);
  glColorPointer(4,GL_UNSIGNED_BYTE,4,fountainColrs10);
  glDrawElements(GL_TRIANGLES, fountainNumIndices10, GL_UNSIGNED_INT, fountainIndices10);

  glVertexPointer(3,GL_FLOAT,12,fountainVerts11);
  glTexCoordPointer(2,GL_FLOAT,8,fountainTexCoords11);
  glColorPointer(4,GL_UNSIGNED_BYTE,4,fountainColrs11);
  glDrawElements(GL_TRIANGLES, fountainNumIndices11, GL_UNSIGNED_INT, fountainIndices11);

  glVertexPointer(3,GL_FLOAT,12,fountainVerts12);
  glTexCoordPointer(2,GL_FLOAT,8,fountainTexCoords12);
  glColorPointer(4,GL_UNSIGNED_BYTE,4,fountainColrs12);
  glDrawElements(GL_TRIANGLES, fountainNumIndices12, GL_UNSIGNED_INT, fountainIndices12);

  glPopAttrib();
}
