#ifndef WEATHER_H
#define WEATHER_H

#include <list>
#include <math/Vector.h>

typedef Vector WeatherParticle;

class Weather
{
public:
  // I guess we could have more of these later or something.
  enum WEATHER_TYPE { Snow };

  // This gets the weather system.
  static Weather* instance();

  // Well, since there's only snow right now, this is kinda moot.
  void setWeatherType(WEATHER_TYPE);

  // This determines how many particle per... some square somethings
  // there are.  Tweak to taste.
  void setThickness(float);

  // This determines what area the weather is created over.  It's a
  // radius-value from 0, in Manhattan distance (a square).
  void setArea(float);
  
  // This defines the range of the life of the weather particles.
  void setStartZ(float);
  void setEndZ(float);

  // Currently, I only support a constant velocity on weather systems.
  // Maybe I can support acceleration later.
  void setVelocity(const Vector&);

  // This is a diameter-size of the quad that is the particle.
  void setSize(float);

  // This is the texture used in drawing the weather particles.
  void setTexture(int);
  
  // Update should be called each frame to move stuff.
  void update();

  // Draw draws the weather system.
  void draw();

protected:
  explicit Weather();
  static Weather* singleton;

  WEATHER_TYPE weatherType;
  float thickness;
  float area;
  float highZ, lowZ;
  int textureName;
  float diam;

  Vector velocity;

  // Okay, so the name is snow-specific.  So sue me.
  std::list<WeatherParticle> flakes;

  long lastTime;
};

#endif // WEATHER_H