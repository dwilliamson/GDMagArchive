/*
  The purpose of this is to be a quarantined random number
  generator: one with its own state, as opposed to C's rand()
  function which has shared global state.  That shared global
  state causes a lot of software engineering problems.

  However, is is not trying to be an excessively higher-quality
  random number generator than what you would get with random()
  and srandom().  At some point we may augment this file with
  other generators like the Mersenne Twister.
 */

#ifndef __RANDOM_GENERATOR_H
#define __RANDOM_GENERATOR_H

struct Random_Generator {
    Random_Generator();

    void seed(unsigned long);
    unsigned long get();

    float get_nonnegative_real(float max);
    float get_angle(float width);
    float get_within_range(float min, float max);

  private:
    unsigned long state;
};

#endif // __RANDOM_GENERATOR_H

