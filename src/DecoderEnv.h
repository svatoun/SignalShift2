#ifndef __decoder_env_h__
#define __decoder_env_h__

#include <Arduino.h>
#include "SignalShift.h"

/* ------------ Supported CV numbers ------------------ */
const uint16_t CV_AUXILIARY_ACTIVATION = 2;
const uint16_t CV_DECODER_KEY = 15;
const uint16_t CV_DECODER_LOCK = 16;
const uint16_t CV_RESET_TYPE = 33;
const uint16_t CV_ROCO_ADDRESS = 34;
const uint16_t CV_FADE_RATE = 39;
const uint16_t CV_NUM_SIGNAL_NUMBER = 40;
const uint16_t CV_ASPECT_LAG = 41;

const uint16_t CV_PROD_ID_1 = 47;
const uint16_t CV_PROD_ID_2 = 48;
const uint16_t CV_PROD_ID_3 = 49;
const uint16_t CV_PROD_ID_4 = 50;

const uint16_t START_CV_OUTPUT = 128;
const uint16_t END_CV_OUTPUT = START_CV_OUTPUT + (SEGMENT_SIZE * NUM_SIGNAL_MAST - 1);

const uint16_t START_CV_OUTPUT_BASE = 352;
const uint16_t END_CV_OUTPUT_BASE = START_CV_OUTPUT_BASE + NUM_SIGNAL_MAST - 1;

const uint16_t START_CV_ASPECT_TABLE = 512;
static_assert(END_CV_OUTPUT < START_CV_ASPECT_TABLE, "Too many outputs per masts, CV overlap");
static_assert(START_CV_ASPECT_TABLE + NUM_SIGNAL_MAST * maxAspects <= 1024, "Too many mast aspects, out of memory");

/* ------------- Default values for supported CVs ---------- */

const uint8_t VALUE_AUXILIARY_ACTIVATION = 4;  // change this value to restore CV defaults after upload sketch
const uint8_t VALUE_DECODER_KEY = 0;           // unlocked decoder
const uint8_t VALUE_DECODER_LOCK = 0;          // unlocked decoder

const uint8_t VALUE_ROCO_ADDRESS = 0;       // 1 - ROCO address, 0 - LENZ address
const uint8_t VALUE_FADE_RATE = 5;          // 0 - 7
const uint8_t VALUE_NUM_SIGNAL_NUMBER = 8;  // 1 - NUM_SIGNAL_MAST
const uint8_t VALUE_ASPECT_LAG = 1;         // 0 - 255   LAG × 0,128 s

const uint8_t VALUE_PROD_ID_1 = 2;  // productID #1
const uint8_t VALUE_PROD_ID_2 = 1;  // productID #2
const uint8_t VALUE_PROD_ID_3 = 1;  // productID #3
const uint8_t VALUE_PROD_ID_4 = 1;  // productID #4

#endif