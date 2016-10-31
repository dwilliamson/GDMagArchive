#pragma once
#ifndef __CONFIG_H
#define __CONFIG_H

//
// Values used to configure the behavior of the system...
//

const int MAX_DATA_MODELS = 10;

// MESSAGE_ID_LIMIT controls, of course, the limit of message IDs.
// Each message we send requires a unique, serial message ID, so that
// we can group messages into blocks.  We need to store that ID in
// each packet, which takes up space.  We don't want it to take up
// too much space, so we make the limit small.  Right now, we don't
// have a way of recycling message IDs; so we just start at 0 and
// count upward, and if we ever hit MESSAGE_ID_LIMIT, the program
// will assert (or crash if in release build).  For a real system,
// you would want to cycle the message IDs back to 0.  This would
// not be very difficult to do, but I left that out, since I want
// this example to be simple.
const int MESSAGE_ID_LIMIT = 6000;

const int MESSAGES_PER_BLOCK = 10;
const int MESSAGE_LENGTH_MAX = 500;

// You can tweak WEIGHT_FOR_OLD_MODEL and WEIGHT_FOR_NEW_MODEL
// to control the rate at which the adaptive models converge
// to the recent input.
const int WEIGHT_FOR_OLD_MODEL = 2;
const int WEIGHT_FOR_NEW_MODEL = 1;

// Turn on DISABLE_ADAPTATION to see how the system performs
// using only the static model.
const int DISABLE_ADAPTATION = 0;

// PACKET_LOSS_PROBABILITY tells us how likely it is that a
// network message will be lost.
const float PACKET_LOSS_PROBABILITY = 0.006f;



// Computed based on the values above; don't change this!
const int BLOCK_INDEX_LIMIT = (MESSAGE_ID_LIMIT / MESSAGES_PER_BLOCK) + 1;


#endif
