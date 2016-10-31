// basicFluid.cpp
//
// This is the driver source for all the fluid demos.  It sets up OpenGL defaults, manages user input for
// keystrokes and the like, gets the menu bars set up, and manages the top physics & draw loop.
//
// -- BHS

#pragma warning (disable: 4786)

#include <harness/main.h>
#include <harness/CameraMover.h>
#include <harness/FreeCamera.h>
#include <harness/OriginOrbitCamera.h>
#include <harness/PlatformSpecific.h>
#include <harness/GlobalCamera.h>
#include <harness/OpenGL.h>
#include <harness/PlatformSpecific.h>
#include <harness/TextureManager.h>

#include <math/Vector.h>

#include <iostream>
#include <gl/glut.h>

#include <fluid/Render/ImplicitMesh.h>
#include <fluid/Tess/ContinuousTessellator.h>
#include <fluid/Tess/MarchingCube.h>
#include <fluid/Tess/CubePolygonizer.h>
#include <fluid/SurfRep/PotentialPoints.h>
#include <fluid/SurfRep/SurfaceSampler.h>
#include <fluid/AutoOrbitCam.h>

// Our demo objects.
#include <fluid/Demos/DummyDemo.h>
#include <fluid/Demos/StreamDemo.h>
#include <fluid/Demos/CongealDemo.h>
#include <fluid/Demos/FountainDemo.h>
#include <fluid/Demos/FillDemo.h>
#include <fluid/Demos/HeightFieldDemo.h>
#include <fluid/Demos/FountainProtoDemo.h>

#include <fluid/Utility/IntHash.h>

// Trying out some pui menu-bar action to prevent the incremental creep of lots of magic keystrokes
// needed to operate the demo.
#include <plib/pu.h>
#include <fluid/PuiPersona.h>

#pragma warning (disable: 4786)

//////////////////////////
//// Implicit Objects ////
//////////////////////////
ImplicitMesh mesh;
ContinuousTessellator tess;
PotentialPoints implicitPoints;
SurfaceSampler sampler;
CubePolygonizer polygonizer;
MarchingCube cube;

/////////////////////////////
//// Graphics State Data ////
/////////////////////////////

// Our cubelet size.
float cubeletSize = 0.60f;
bool drawNormals = false;
bool renderMesh = true;
GLenum polyMode = GL_FILL;

// Our sphere-map texture.
int texHandle = 0;

// If they specified a texture on the command line we use it, explicitly overriding demo defaults.
bool cmdLineTexture = false;

// The isovalue, used to determine where the isosurface is.
float surfaceThreshold = 3.0f;

////////////////////////////
//// Physics State Data ////
////////////////////////////

// Set STABLE_PHYSICS to true to put the ImplicitAnimator on a fixed timestep (i.e it calls it
// multiple times per frame if the frame is longer than the timestep, or only once every few frames
// if the frames are shorter, that kind of thing.)  If STABLE_PHYSICS is false the animator gets
// called once per frame with a variable timestep.
bool stablePhysics = true;
float physicsTick = 0.045;

// This can be used to scale real-time -- to make the demo run faster than real time, or slower.
float timeScalar = 1.0f;

bool paused = false;

///////////////////////////////////
//// Performance Analysis Data ////
///////////////////////////////////
int trisDrawn = 0;
int framesDrawn = 0;
float timePassed = 0;
bool drawStatistics = false;

////////////////////////////////////////
//// Menu-related data & prototypes ////
////////////////////////////////////////

void setupMenuBar();

std::vector<CameraPersona*> demoPersonas;
int preferredMover;

PuiPersona* menuMover;

bool menuMode = false;
bool menuHasBeenOpened = false;
int lastMenuCursorX, lastMenuCursorY;

// We keep our little "Hit '~' to toggle menu" banner texture here.
int menuInfoTexture = 0;

///////////////////////////////////////
//// Demo-object data & prototypes ////
///////////////////////////////////////

// Used to get all the defaults out of a demo when we switch, to "restart" things.
void readStateFromDemo();

// Physical manipulation.  We store a list of all possible demos so that we can cycle through them
// on the fly.
std::vector<FluidDemo*> demoObjectList;

// Our current demo index.
int demoObject;       

// If we're in auto-cam mode, we also periodically switch demos.
bool autoSwitchMode = false;
float demoDuration;
const float maxDemoDuration = 45.0f;
const float demoFadeTime = 2.0f;

/////////////////////////////////////////////
//////////////// App code ///////////////////
/////////////////////////////////////////////

const char* appWindowName()
{
  return "Basic Fluid Driver";
}

void appCleanup()
{
  for (int x=0; x<demoObjectList.size(); x++)
  {
    delete demoObjectList[x];
  }

  std::cout << "Tris Drawn: " << trisDrawn << std::endl;
  std::cout << "Frames Drawn: " << framesDrawn << std::endl;
  std::cout << "Tris / frame: " << ((float)trisDrawn / (float)framesDrawn) << std::endl;
  std::cout << "Tris / sec: " << ((float)trisDrawn / timePassed) << std::endl;
}

void appInit(int argc, char** argv)
{
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  
  float diffuse [] = {0.7f, 0.6f, 1.0f, 1.0f};

  glShadeModel(GL_SMOOTH);

  glDisable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glDisable(GL_COLOR_MATERIAL);

  // Setup our mover so we can have mouse-driven camera movement.
  CameraMover::initCallbacks();
  
  // We switch between these depending on what mode we're in -- camera movement, or using
  // the menu system.
  demoPersonas.push_back(new FreeCamera());
  demoPersonas.push_back(new OriginOrbitCamera());
  demoPersonas.push_back(new AutoOrbitCam());

  // We initially use the origin-orbit cam over the freecam.
  preferredMover = 1;

  menuMover = new PuiPersona();
  
  CameraMover::registerPersona(demoPersonas[preferredMover]);

  puHideCursor();

  GlobalCamera::Instance()->setFovY(40);
  GlobalCamera::Instance()->getPosition().x = 5;
  GlobalCamera::Instance()->getPosition().y = 15;
  GlobalCamera::Instance()->getPosition().z = 20;
  GlobalCamera::Instance()->getPosition().setOrientation(Vector(-5,-15,-20));
  GlobalCamera::Instance()->getPosition().setUpVector(Vector(0,0,1));

  // Also, make the mouse invisible since we do mlook-style movement now!
  glutSetCursor(GLUT_CURSOR_NONE);

  // Rev up the extensions.
  OpenGL::initExtensions();

  // Load up the menu info-bar texture.
  menuInfoTexture = TextureManager::instance()->addTexture(std::string("data/menuinfo.raw"), 256, 32, false, false);

  // Make sure our objects know about each other.
  sampler.setPotentialPoints(&implicitPoints);
  polygonizer.setSurfaceSampler(&sampler);
  polygonizer.setImplicitMesh(&mesh);
  tess.setMarchingCube(&cube);
  tess.setCubePolygonizer(&polygonizer);

  mesh.setUseColorArrays(false);
  mesh.setUseTexCoordArrays(true);
  mesh.setUseNormalArrays(false);

  // Initialize our functors for whatever this demo is.
  demoObject = 0;
//  demoObjectList.push_back(new DummyDemo());
  demoObjectList.push_back(new CongealDemo());
  demoObjectList.push_back(new FountainDemo());
  demoObjectList.push_back(new StreamDemo());
  demoObjectList.push_back(new HeightFieldDemo());
  demoObjectList.push_back(new FountainProtoDemo());
//  demoObjectList.push_back(new FillDemo());
  readStateFromDemo();

  // Argument parsing.
  for (int arg=1; arg<argc; arg++)
  {
    // Set up a texture if one was specified on the command line.
    if ((strcmp(argv[arg], "-texture") == 0 || strcmp(argv[arg], "/texture") == 0) && arg < argc-1)
    {
      std::string texName = argv[arg+1];
      cmdLineTexture = true;
      texHandle = TextureManager::instance()->addTexture(texName, 256, 256, false);
      glBindTexture(GL_TEXTURE_2D, texHandle);
    }

    // If they said to run as a demo, do so.
    if (strcmp(argv[arg], "-demo") == 0 || strcmp(argv[arg], "/demo") == 0)
    {
      preferredMover = 2;
      autoSwitchMode = true;
      demoDuration = 0;
      CameraMover::registerPersona(demoPersonas[preferredMover]);

      // Don't draw that menu texture.
      menuHasBeenOpened = true;
    }
  }

  glEnable(GL_TEXTURE_2D);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

  setupMenuBar();
}

void appKey(unsigned char keyHit, int x, int y)
{
  // Note that we're case-insensitive here (- and _ do the same thing).
  switch ( keyHit )
  {
  case '-':
  case '_':
    cubeletSize += 0.05f;
    break;

  case '=':
  case '+':
    cubeletSize = Math::maxOf(0.1, cubeletSize-0.05);
    break;

  case 'w':
  case 'W':
    polyMode = (polyMode == GL_FILL) ? GL_LINE : GL_FILL;
    break;

  case 'p':
  case 'P':
    paused = !paused;
    break;

  case 'l':
  case 'L':
    if (glIsEnabled(GL_LIGHTING))
    {
      glDisable(GL_LIGHTING);
      mesh.setUseColorArrays(true);
    }
    else
    {
      if (!mesh.getUseColorArrays())
      {
        glEnable(GL_LIGHTING);
      }
      else
      {
        mesh.setUseColorArrays(false);
      }
    }
    break;

  case 't':
  case 'T':
    if (glIsEnabled(GL_TEXTURE_GEN_S))
    {
      glDisable(GL_TEXTURE_GEN_S);
      glDisable(GL_TEXTURE_GEN_T);
      mesh.setUseTexCoordArrays(true);
    }
    else
    {
      if (!mesh.getUseTexCoordArrays())
      {
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        glEnable(GL_TEXTURE_2D);
      }
      else
      {
        glDisable(GL_TEXTURE_2D);
        mesh.setUseTexCoordArrays(false);
      }
    }
    break;

  case 's':
  case 'S':
    stablePhysics = !stablePhysics;
    break;

  case 'n':
  case 'N':
    drawNormals = !drawNormals;
    break;

  case 'd':
    tess.incrementDebugLevel(1);
    break;

  case 'D':
    tess.incrementDebugLevel(-1);
    break;

  case 'm':
  case 'M':
    renderMesh = !renderMesh;
    break;

  // Toggle between the freecam and the origin-orbit cam.
  case 'c':
  case 'C':
    preferredMover = (preferredMover + 1) % demoPersonas.size();
    
    // Make sure we're looking at the origin for the origin-orbit cam.
    if (preferredMover != 0)
    {
      GlobalCamera::Instance()->getPosition().setOrientation(-GlobalCamera::Instance()->getPosition());
      GlobalCamera::Instance()->getPosition().setUpVector(Vector(0,0,1));
    }
    if (preferredMover == 2)
    {
      autoSwitchMode = true;
      demoDuration = 0;
    }
    else
    {
      autoSwitchMode = false;
    }
    if (!menuMode)
    {
      CameraMover::registerPersona(demoPersonas[preferredMover]);
    }
    break;

  // Allow explicit setting of the test number.
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    {
      int demoNum = keyHit - '1';
      if (demoObjectList.size() > demoNum)
      {
        demoDuration = 0;
        implicitPoints.clear();
        demoObject = demoNum;
        readStateFromDemo();
      }
    }
    break;

  case 'z':
    timeScalar *= 1.05f;
    break;

  case 'Z':
    timeScalar /= 1.05f;
    break;

  case '`':
  case '~':
    if (menuMode)
    {
      // Reset this to avoid whipping the view around when we switch back to camera mode.
      int cenX = glutGet(GLUT_WINDOW_X) + glutGet(GLUT_WINDOW_WIDTH)/2;
      int cenY = glutGet(GLUT_WINDOW_Y) + glutGet(GLUT_WINDOW_HEIGHT)/2;
      glutWarpPointer(cenX, cenY);
      CameraMover::registerPersona(demoPersonas[preferredMover]);
      puHideCursor();

      lastMenuCursorX = x;
      lastMenuCursorY = y;
    }
    else
    {
      if (menuHasBeenOpened)
      {
        glutWarpPointer(lastMenuCursorX, lastMenuCursorY);
      }
      CameraMover::registerPersona(menuMover);
      puShowCursor();

      // If they ever turn the menu on, stop drawing the tip telling them how to.
      menuHasBeenOpened = true;
    }
    menuMode = !menuMode;
    break;

  case 'a':
  case 'A':
    drawStatistics = !drawStatistics;
    break;
  }

  // Check for normal usage.
  if (glIsEnabled(GL_LIGHTING) || glIsEnabled(GL_TEXTURE_GEN_S))
  {
    mesh.setUseNormalArrays(true);
  }
  else
  {
    mesh.setUseNormalArrays(false);
  }
}

void appDraw()
{
  glClear(GL_DEPTH_BUFFER_BIT);

  static float lastTime = 0;
  static float accumTime = 0;

  float curTime = timeGetTime() / 1000.0f;
  float lastUnscaledFrameTime = lastTime == 0 ? 0 : (curTime-lastTime);
  float lastFrameTime = lastTime == 0 ? 0 : (demoObjectList[demoObject]->getTimeScalar()*timeScalar*(curTime-lastTime));

  if (lastTime != 0)
  {
    if (!paused)
    {
      // Safety check, force stable physics on if framerate is under half the stable physics
      // rate.  Otherwise unstable physics will cause things to blow up.
      if (stablePhysics || (lastFrameTime > 2*physicsTick))
      {
        accumTime += lastFrameTime;

        // We use this safety check to count the number of physics ticks we're running.  If it's ever
        // above a few ticks per frame, the framerate's too low and we're going to start churning to death,
        // getting slower every frame, so scale the time scalar back.
        int safetyCheck = 0;

        while (accumTime > physicsTick)
        {
          if (demoObjectList[demoObject]->getGenerator()  != 0) demoObjectList[demoObject]->getGenerator()->update(implicitPoints, physicsTick);
          if (demoObjectList[demoObject]->getForce()      != 0) demoObjectList[demoObject]->getForce()->update(implicitPoints, physicsTick);
          if (demoObjectList[demoObject]->getIntegrator() != 0) demoObjectList[demoObject]->getIntegrator()->update(implicitPoints, physicsTick);
          if (demoObjectList[demoObject]->getCollision()  != 0) demoObjectList[demoObject]->getCollision()->update(implicitPoints, physicsTick);
          accumTime -= physicsTick;
          safetyCheck++;

          // Only do it when it's equal to the threshold or else it'll crank it down for every tick above
          // the threshold that it needs to do on a given frame (leaving it running abysmally slowly.)
          if (safetyCheck == 20)
          {
            timeScalar *= 0.5f;
          }
        }
      }
      else
      {
        if (demoObjectList[demoObject]->getGenerator()  != 0) demoObjectList[demoObject]->getGenerator()->update(implicitPoints, lastFrameTime);
        if (demoObjectList[demoObject]->getForce()      != 0) demoObjectList[demoObject]->getForce()->update(implicitPoints, lastFrameTime);
        if (demoObjectList[demoObject]->getIntegrator() != 0) demoObjectList[demoObject]->getIntegrator()->update(implicitPoints, lastFrameTime);
        if (demoObjectList[demoObject]->getCollision()  != 0) demoObjectList[demoObject]->getCollision()->update(implicitPoints, lastFrameTime);
      }
    }

    // Use real (non-scaled) time here, it's used in benchmarking tris/sec.
    timePassed += curTime-lastTime;
  }

  // Update any mouse movement.
  CameraMover::update(lastUnscaledFrameTime);

  // Re-init the camera.
  GlobalCamera::Instance()->makeCurrent();

  // Position the light.
  float lightPos[4];
  lightPos[0] = 1; 
  lightPos[1] = 1; 
  lightPos[2] = 10; 
  lightPos[3] = 0.0f; 
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

  implicitPoints.finalizePointChanges();

  // Draw our surface.
  glPolygonMode(GL_FRONT_AND_BACK, polyMode);

  mesh.clear();
  tess.tessellate(implicitPoints, cubeletSize, surfaceThreshold);
  mesh.generateNormals();
  mesh.doLight();
  mesh.doSphereMap();

  // Let this guy draw any decoration he wants; objects in the "scene" getting collided against, 
  // for instance.  Also gives him a chance to make any per-frame changes he wants.  Note that we
  // pause him when the physics are paused, as well.
  demoObjectList[demoObject]->update(paused ? 0 : lastFrameTime);

  if (renderMesh)
  {
    glBindTexture(GL_TEXTURE_2D, texHandle);
    demoObjectList[demoObject]->setGLCurrents();
    mesh.render();
  }

  if (drawNormals)
  {
    mesh.renderNormals();
  }           

  trisDrawn += mesh.getNumTris();
  framesDrawn++;
  lastTime = curTime;

  if (menuMode)
  {
    glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_POLYGON_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    puDisplay();
    glPopAttrib();
  }

  int w = puGetWindowWidth();
  int h = puGetWindowHeight();
  
  glPushAttrib   (GL_ENABLE_BIT | GL_TRANSFORM_BIT);
  
  glDisable      (GL_LIGHTING);
  glDisable      (GL_DEPTH_TEST);
  glDisable      (GL_TEXTURE_GEN_S);
  glDisable      (GL_TEXTURE_GEN_T);
  
  glMatrixMode   (GL_PROJECTION);
  glPushMatrix   ();
  glLoadIdentity ();
  gluOrtho2D     (0, w, 0, h);
  glMatrixMode   (GL_MODELVIEW);
  glPushMatrix   ();
  glLoadIdentity ();

  if (!menuHasBeenOpened)
  {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, menuInfoTexture);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glBegin(GL_QUADS);
    glColor3f(1,1,1);
    glTexCoord2f(0,0);
    glVertex2f(0,0);
    glTexCoord2f(1,0);
    glVertex2f(256,0);
    glTexCoord2f(1,1);
    glVertex2f(256,32);
    glTexCoord2f(0,1);
    glVertex2f(0,32);
    glEnd();
  }

  if (drawStatistics)
  {
    tess.drawDebugStatistics();
  }
  
  glMatrixMode   (GL_PROJECTION);
  glPopMatrix    () ;
  glMatrixMode   (GL_MODELVIEW);
  glPopMatrix    () ;
  glPopAttrib    () ;

  // If we're running in autoswitch mode, switch if it's time to do so.
  if (autoSwitchMode)
  {
    demoDuration += lastFrameTime;
    if (demoDuration > maxDemoDuration)
    {
      // It's a bit of a hack to patch this through my key callback system... but oh well.
      key('1' + ((demoObject + 1) % demoObjectList.size()), 0, 0);
    }

    // Fade in or out.
    float fadeScalar = 0;
    bool fade = false;
    if (demoDuration < demoFadeTime*1.1f)
    {
      fade = true;
      fadeScalar = 1.0f - (demoDuration-demoFadeTime*0.1f)/demoFadeTime;
    }
    else if (maxDemoDuration - demoDuration < demoFadeTime*1.1f)
    {
      fade = true;
      fadeScalar = 1.0f - (maxDemoDuration-demoDuration-demoFadeTime*0.1f)/demoFadeTime;
    }
    
    if (fade)
    {
      glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_LIGHTING);
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);
  
      glMatrixMode   (GL_PROJECTION);
      glPushMatrix   ();
      glLoadIdentity ();
      gluOrtho2D     (0, w, 0, h);
      glMatrixMode   (GL_MODELVIEW);
      glPushMatrix   ();
      glLoadIdentity ();

      glBegin(GL_QUADS);
      glColor4f(0,0,0,fadeScalar);
      glVertex2f(0,0);
      glVertex2f(w,0);
      glVertex2f(w,h);
      glVertex2f(0,h);
      glEnd();

      glMatrixMode   (GL_PROJECTION);
      glPopMatrix    ();
      glMatrixMode   (GL_MODELVIEW);
      glPopMatrix    ();

      glPopAttrib();
    }
  }
}

// Used to get all the defaults out of a demo when we switch, to "restart" things.
void readStateFromDemo()
{
  demoObjectList[demoObject]->start();

  cubeletSize = max(0.05, demoObjectList[demoObject]->getTessellationDensity());
  stablePhysics = demoObjectList[demoObject]->useStablePhysics();
  physicsTick = demoObjectList[demoObject]->getPhysicsTick();
  surfaceThreshold = demoObjectList[demoObject]->getSurfaceThreshold();

  if (!cmdLineTexture)
  {
    std::string texName = demoObjectList[demoObject]->getEnvMapFilename();
    texHandle = TextureManager::instance()->addTexture(texName, 256, 256, false);
    glBindTexture(GL_TEXTURE_2D, texHandle);
  }
}

//////////////////////////////////////////////////////////////////////
/////////////////////// PUI Menu Setup Code //////////////////////////
//////////////////////////////////////////////////////////////////////

// Hypothetically I could use a template function for this instead of a whole class,
// but MSVC 5 is broken so a template function doesn't work in this case (it can't resolve
// the bracketed unsigned char template parameter for the function, but magically it can for
// a class... whatever.)
template <unsigned char whichKey>
class keyCallbackClass
{
public:
  static void puiKeyCallback(puObject*)
  {
    // We call appKey with the specified key; this allows us to templatize the PUI menu system
    // callbacks to just translate into keystrokes.
    key(whichKey, 0, 0);
  }
};

void setupColorScheme(puObject* ui)
{
  // Now setup the color scheme (note that it doesn't auto-propagate.)
  ui->setColorScheme(0.3f, 0.0f, 0.0f);
  ui->setColor(PUCOL_LEGEND, 1.0f, 1.0f, 1.0f);
    
  if (ui->getType() & PUCLASS_GROUP)
  {
    puGroup* uiGroup = (puGroup*)(ui);
    
    puObject* subUI = uiGroup->getFirstChild();
    while (subUI != 0)
    {
      setupColorScheme(subUI);
      subUI = subUI->next;
    }
  }
}

void setupMenuBar()
{
  // Setup PUI menus.
  char*      file_submenu    [] = { "Exit (ESC)", "", "Take Screenshot (X)", 0};
  puCallback file_submenu_cb [] = { keyCallbackClass<(unsigned char)27>::puiKeyCallback, 0,
                                    keyCallbackClass<'x'>::puiKeyCallback, 0};

  char*      glsettings_submenu    [] = { "Toggle Camera Style (C)",
                                          "","Cycle lighting style (L)", "Cycle texgen style (T)",
                                          "Decrease Tessellation (-)","Increase Tessellation (+)", 0};
  puCallback glsettings_submenu_cb [] = { keyCallbackClass<'c'>::puiKeyCallback, 0,
                                          keyCallbackClass<'l'>::puiKeyCallback, 
                                          keyCallbackClass<'t'>::puiKeyCallback, 
                                          keyCallbackClass<'-'>::puiKeyCallback, 
                                          keyCallbackClass<'+'>::puiKeyCallback, 0};
  
  char*      debug_submenu    [] = { "Show/Hide Hashtable Distributions (A)", "Toggle Mesh Drawing (M)", 
                                     "Decrement Debug Draw Level (D)", 
                                     "Increment Debug Draw Level (d)", "Toggle Normals (N)",  
                                     "Toggle Wireframe (W)", 0};
  puCallback debug_submenu_cb [] = { keyCallbackClass<'a'>::puiKeyCallback,
                                     keyCallbackClass<'m'>::puiKeyCallback,
                                     keyCallbackClass<'D'>::puiKeyCallback,
                                     keyCallbackClass<'d'>::puiKeyCallback, 
                                     keyCallbackClass<'n'>::puiKeyCallback, 
                                     keyCallbackClass<'w'>::puiKeyCallback, 0};

  char*      physics_submenu    [] = { "Toggle Stable / Unstable Physics (S)", "Decrease speed by 5% (Z)", 
                                       "Increase Speed by 5% (z)", "Pause / Unpause Physics (P)", 0};
  puCallback physics_submenu_cb [] = { keyCallbackClass<'s'>::puiKeyCallback,
                                     keyCallbackClass<'Z'>::puiKeyCallback,
                                     keyCallbackClass<'z'>::puiKeyCallback, 
                                     keyCallbackClass<'p'>::puiKeyCallback, 0};
  
  char*      about_submenu    [] = { "http://www.maniacal.org/",
                                     "For more information:","",
                                     "Alexander Calder.",
                                     "on the original work by",
                                     "Steel Studios, based",
                                     "Ashley of Stainless",
                                     "and textured by Steve",
                                     "Mercury Fountain modeled","",
                                     "Magazine.",
                                     "issues of Game Developer",
                                     "in the July & Aug. '00",
                                     "Accompanies my articles","",
                                     "implicit surfaces.",
                                     "realtime-tessellated",
                                     "Fluid modeling using","",
                                     "brian@maniacal.org",
                                     "Brian Sharp",
                                     "Fluid", 0};
  puCallback about_submenu_cb [] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

  char* null_submenu[] = {0};
  puCallback null_submenu_cb[] = {0};

  // Set this one up dynamically so it automatically gets filled with the list of demos.
  char** demo_submenu = new char*[demoObjectList.size()+1];
  puCallback* demo_submenu_cb = new puCallback[demoObjectList.size()+1];
  
  // Cap the ends.
  demo_submenu[demoObjectList.size()] = 0;
  demo_submenu_cb[demoObjectList.size()] = 0;

  // Annoying but necessary: can't have template arguments come from variables, so just set these up
  // since we don't know how many we need but we know it'll be less than 10.
  puCallback numericCallbacks[] = {keyCallbackClass<'1'>::puiKeyCallback, keyCallbackClass<'2'>::puiKeyCallback,
                                   keyCallbackClass<'3'>::puiKeyCallback, keyCallbackClass<'4'>::puiKeyCallback,
                                   keyCallbackClass<'5'>::puiKeyCallback, keyCallbackClass<'6'>::puiKeyCallback,
                                   keyCallbackClass<'7'>::puiKeyCallback, keyCallbackClass<'8'>::puiKeyCallback,
                                   keyCallbackClass<'9'>::puiKeyCallback, keyCallbackClass<'0'>::puiKeyCallback};

  // Now fill them.  Remember, it's a stack, so the order is backwards...
  for (int x=0; x<demoObjectList.size(); x++)
  {
    std::string demoName = demoObjectList[x]->getName();
    demoName += " (";
    demoName += (unsigned char)('1' + x);
    demoName += ")";
    demo_submenu[demoObjectList.size()-x-1] = new char[demoName.size()+5];
    strcpy(demo_submenu[demoObjectList.size()-x-1], demoName.c_str());

    unsigned char keyName = '1';
    demo_submenu_cb[demoObjectList.size()-x-1] = numericCallbacks[x];
  }
  
  puMenuBar* menu = new puMenuBar();
  
  menu->add_submenu("File", file_submenu, file_submenu_cb);
  menu->add_submenu("|", null_submenu, null_submenu_cb);
  menu->add_submenu("Demo", demo_submenu, demo_submenu_cb);
  menu->add_submenu("|", null_submenu, null_submenu_cb);
  menu->add_submenu("Graphics", glsettings_submenu, glsettings_submenu_cb);
  menu->add_submenu("|", null_submenu, null_submenu_cb);
  menu->add_submenu("Debug", debug_submenu, debug_submenu_cb);
  menu->add_submenu("|", null_submenu, null_submenu_cb);
  menu->add_submenu("Physics", physics_submenu, physics_submenu_cb);
  menu->add_submenu("|", null_submenu, null_submenu_cb);
  menu->add_submenu("About", about_submenu, about_submenu_cb);

  setupColorScheme(menu);

  menu->close();
}
