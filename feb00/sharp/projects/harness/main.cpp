//
//  Project driver file.
//
//      Brian Sharp, 6/2/98
//      bsharp@acm.org
//
//  This file contains the basic driver setup
//  for all my 3D OpenGL-based personal projects.
//

// This includes windows.h for us, and gets us all the OpenGL functions.
#include <harness/PlatformSpecific.h>
#include <gl/glut.h>
#include <mmsystem.h>
#include <assert.h>

// We use this for outputting the fps rating.
#include <iostream>

// We include this for the filename-generating we do in screenShot().
#include <strstream>
#include <iomanip>

// This we need for strstr, which is used to see if the implementation of OpenGL
// is accelerated or not.
#include <string.h>

// This we use to position the camera.  This way, others can know about it and
// modify it.
#include <harness/GlobalCamera.h>

// Just because you can't register multiple people to receive the keyboard messages...
#include <harness/CameraMover.h>

// Used to make sure we're cleaning up after ourself.
#include <harness/TextureManager.h>

// This has our prototypes and the application-defined functions that we call from here.
#include "main.h"

unsigned long numFrames = 0;
long startTime, endTime;

int screenWidth = 1024;
int screenHeight = 768;

void init()
{
    // This is a really bare-bones init().  It just
    // turns on lighting, puts a light in, and turns
    // on color material.  It assumes a lot of OpenGL
    // defaults.

    float ambient [] = {0.0f, 0.0f, 0.03f, 1.0f};
    float diffuse [] = {0.8f, 0.9f, 1.0f, 1.0f};
    float specular[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float position[] = {0.0f,-15.0f,5.0f, 0.0f};

    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_COLOR_MATERIAL);

    GlobalCamera::Instance()->setFovY(50);
    GlobalCamera::Instance()->getPosition().x = 0;
    GlobalCamera::Instance()->getPosition().y = 20;
    GlobalCamera::Instance()->getPosition().z = 10;
    GlobalCamera::Instance()->getPosition().setOrientation(Vector(0,-20,-10));
    GlobalCamera::Instance()->getPosition().setUpVector(Vector(0,0,1));

    // Both the SGI and MS software OpenGL implementations have the word "Generic" in the
    // RENDERER string.  So we check, and if it's not there, we'll assume it's accelerated and
    // turn on blending and smoothing.
    if ( strstr( (const char*)glGetString( GL_RENDERER ), "Generic" ) == 0 )
    {
//        // Turn on point smoothing.
//        glEnable(GL_POINT_SMOOTH);
//        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
//
//        // Turn on line smoothing.
//        glEnable(GL_LINE_SMOOTH);
//        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
//
//        // Turn on polygon smoothing.
//        glEnable(GL_POLYGON_SMOOTH);
//        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
//
//        // We need this for the blending to work properly.
//        glEnable(GL_BLEND);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
}

void screenShot()
{
  char* imgData = new char[ screenWidth * screenHeight * 3 ];
  // Read the pixels back.
  ::glReadPixels( 0, 0, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, imgData );

  // This holds the screen shot number and its string representation.
  int x = 0;
  char strNum[ 3 ];

  // This holds the name of the screenshot and whether we've hit one that
  // is free (there isn't already a file named fname.c_str())
  std::string fname;
  bool foundName = false;
  FILE* outfile;

  // Test to find the next available screenshot filename.
  while (!foundName)
  {
      std::strstream writeNum( strNum, 2 );
      writeNum << std::setw(2) << std::setfill('0') << x;
      strNum[2] = '\0';

      fname = "screenshot";
      fname += strNum;
      fname += ".raw";

      outfile = fopen( fname.c_str(), "rb" );
      if ( outfile == 0 )
      {
          foundName = true;
      }
      else
      {
        fclose( outfile );
      }

      // Move to the next file number.
      x++;
  }

  outfile = fopen( fname.c_str(), "wb" );
  fwrite( imgData, 1, screenWidth * screenHeight * 3, outfile );
  fclose( outfile );
  
  delete[] imgData;
}

void key(unsigned char keyHit, int x, int y)
{
    // Let this guy have a shot at it.
    CameraMover::keyboard(keyHit, x, y);

    float fps;

    // Hitting escape exits, and hitting < or >
    // spins the triangle.
    switch (keyHit) {
    case 27:
        endTime = timeGetTime();

        fps = (float)numFrames / ( ( (float)endTime - (float)startTime ) / 1000.0 );
        std::cout << "FPS: " << fps << std::endl;

        appCleanup();
        
        TextureManager::instance()->deleteTextures();

        exit(0);
        break;

    case 'x':
    case 'X':
        screenShot();
        break;

    default:
		break;
    }

    // Check to see if the application wants the key.
    appKey(keyHit, x, y);
}

void draw() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Set up our projection matrix.  Note that because I'm
    // lazy, I'm putting the gluLookAt in the modelview matrix.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(GlobalCamera::Instance()->getFovY(), 1.33f, 1.0f, 220.0f);

    // Clear our matrix, reposition the viewer.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float eyeX = GlobalCamera::Instance()->getPosition().x;
    float eyeY = GlobalCamera::Instance()->getPosition().y;
    float eyeZ = GlobalCamera::Instance()->getPosition().z;
    float oriX = eyeX + GlobalCamera::Instance()->getPosition().getOrientation().x;
    float oriY = eyeY + GlobalCamera::Instance()->getPosition().getOrientation().y;
    float oriZ = eyeZ + GlobalCamera::Instance()->getPosition().getOrientation().z;
    float upvX = GlobalCamera::Instance()->getPosition().getUpVector().x;
    float upvY = GlobalCamera::Instance()->getPosition().getUpVector().y;
    float upvZ = GlobalCamera::Instance()->getPosition().getUpVector().z;

    gluLookAt(eyeX,eyeY,eyeZ, oriX,oriY,oriZ, upvX, upvY, upvZ);

    // Now draw.
    appDraw();

    // We're done.  Swap and leave.
    glutSwapBuffers();

    // Increment the frame counter.
    numFrames++;
}

int main(int argc, char** argv) {
    // Standard stuff.  Give us a window.
    glutInitWindowSize(screenWidth,screenHeight);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow( appWindowName() );
    
    // See if they want us to go fullscreen.
    if (!(argc == 2 && !strcmp(argv[1], "/nofullscreen")))
    {
      // Aargh -- glutFullScreen doesn't work right with 3Dfx boards (Voodoo 2, at least).
//      if ( strstr( (const char*)glGetString( GL_RENDERER ), "3Dfx" ) == 0 )
      {
//        glutFullScreen();
      }
    }

    // Initialize OpenGL the way we want it.
    init();

    // Do anything the application needs.
    appInit(argc, argv);

    // Setup our callbacks.
    glutKeyboardFunc(key);
    glutDisplayFunc(draw);
    glutIdleFunc(draw);

    // Handy information -- tell us about the implementation.
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Extensions: " << glGetString(GL_EXTENSIONS) << std::endl;
    
    // Go.
    startTime = timeGetTime();
    glutMainLoop();

    // ANSI C says to do this.
    return 0;
}



