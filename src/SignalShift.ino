/****************************************************************************
 *  DCC Signal decoder for Arduino Nano / Arduino Pro Mini boards.

 *  use NmraDcc library from http://mrrwa.org/
 *
 *  Original code author: Petr Šídlo
 *  date 2018-09-04
 *  revision 1.2
 *  Original license GPLv2
 *  homepage https://sites.google.com/site/sidloweb/
 *
 *  Author modifications: svatopluk.dedic@gmail.com
 *  RELICENSED to Apache Public License 2.0 with the permission of the Original Author / Copyright holder.
 */

#include "Arduino.h"
#include <NmraDcc.h>
#include <EEPROM.h>

#include "Common.h"
#include "SignalShift.h"
#include "Terminal.h"
#include "DecoderEnv.h"
#include "messages.h"

#ifdef AVR_DEBUG

#include "avr8-stub.h"

#endif

#define Shift

// ------- Diagnostic setup -----------
const boolean debugFadeOnOff = false;
const boolean debugLightFlip = false;
const boolean debugLights = false;
const boolean debugAspects = false;

const uint8_t SW_VERSION = 12;

// --------- HARDWARE PIN WIRING / SETUP --------------------
const byte ACK_BUSY_PIN = A0;

// ShiftPWM HW setup setup
const byte ShiftPWM_dataPin = 11;
const byte ShiftPWM_clockPin = 13;
const byte ShiftPWM_latchPin = 10;

// min/max brightness for PWM. PWM is in range 0-255.
unsigned char maxBrightness = 255;
unsigned char minBrightness = 0;

// For 80 outputs 100Hz PWM frequency is too high, must set lower.
unsigned char pwmFrequency = 60;
const bool ShiftPWM_balanceLoad = false;
const bool ShiftPWM_invertOutputs = true;

// original:
//#define SHIFTPWM_USE_TIMER2
#define SHIFTPWM_USE_TIMER2

// must be included after #define for timer
#ifdef Shift
#include <ShiftPWM.h>
#endif

uint16_t thisDecoderAddress = 100;         // ACCESSORY DECODER ADDRESS
uint16_t maxDecoderAddress = 0;

const unsigned long BLINKING_TIME_54 = 556;
const unsigned long BLINKING_TIME_108 = 278;
const unsigned long BLINKING_TIME_45 = 667;
const unsigned long BLINKING_TIME_22 = 1333;

// PENDING: The brightness of LED is perceived non-lineraly by human eye. Revise the scale
// here according to https://diarmuid.ie/blog/pwm-exponential-led-fading-on-arduino-or-other-platforms
const byte FADE_TIME_LIGHT[11] = { 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66 };
const byte FADE_COUNTER_LIGHT_1[11] = { /* -1 */ 0, 1, 1, 1, 1, 1, 1, 2, 3, 6, 9 };
const byte FADE_COUNTER_LIGHT_2[11] = { /* -1 */ 1, 10, 7, 4, 3, 2, 2, 3, 4, 7, 10 };

// Will be initialized at startup, and whenever fadeRate global changes. As fadeRate is max 7, the value nicely fits in 9 bits unsigned.
unsigned int fadeTimeLight[11] = {};

unsigned long currentTime;
/**
 * Time threshold before code is considered not confirmed as an aspect. If mast's code changes, this time must elapse before the code
 * is converted into an aspect.
 */
unsigned int aspectLag;

SignalMastData signalMastData[NUM_SIGNAL_MAST];

LightFunction bublState2[NUM_OUTPUTS + 8] = {};

unsigned int lightStartTimeBubl[NUM_OUTPUTS + 8] = {};

byte overrides[(NUM_OUTPUTS + 7) / 8] = { false };

boolean reinitializeLocalVariables = true;
boolean aspectsInitialized = false;

int counterNrOutput = 0;
int counterNrSignalMast = 0;


NmraDcc Dcc;
DCC_MSG Packet;

uint8_t lsb;
uint8_t msb;
uint8_t decoderKey;
uint8_t decoderLock;
uint8_t rocoAddress;
uint8_t numSignalNumber;
uint8_t fadeRate;

#ifdef DEBUG_UART0
HardwareSerial& Console = Serial1;
#else
HardwareSerial& Console = Serial;
#endif

void maybeInitLocalVariables();
void initLocalVariables();
void doChangeMastType(int mast, int signalSetOrMastType, boolean stable);
void changeMastType(int mastIndex, int newType);
int reassignMastOutputs(int mastIndex, int from, boolean propagate);

/**********************************************************************************
 * Setup Arduino.
 */
void setup() {
#ifdef AVR_DEBUG
  debug_init();
#endif
  initSharedMessages();

  Console.begin(115200);
  Console.print(F("UNI16ARD-NAV-Shift version "));
  Console.println(SW_VERSION);
  Console.println(msg_InitialMessage);
  Console.println(F("Booting..."));

  // initialize the digital pins as an outputs
  pinMode(ACK_BUSY_PIN, OUTPUT);
  digitalWrite(ACK_BUSY_PIN, LOW);

  pinMode(2, INPUT);
  
  setupShiftPWM();
  // Setup which External Interrupt, the Pin it's associated with that we're using and enable the Pull-Up
  Dcc.pin(digitalPinToInterrupt(2), 2, 1);

  // Call the main DCC Init function to enable the DCC Receiver
  Dcc.initAccessoryDecoder(MAN_ID_DIY, SW_VERSION, FLAGS_OUTPUT_ADDRESS_MODE, 0);

  uint8_t cvVersion = Dcc.getCV(CV_AUXILIARY_ACTIVATION);
  if (cvVersion != VALUE_AUXILIARY_ACTIVATION) {
    setFactoryDefault();
  }

  initLocalVariables();
  ModuleChain::invokeAll(initialize);

  Console.print(F("Driving max "));
  Console.print(NUM_OUTPUTS);
  Console.print(F(" outputs from max "));
  Console.print(NUM_SIGNAL_MAST);
  Console.println(F(" masts"));

  initTerminal();
  setupTerminal();
  maybeInitLocalVariables();
}

void setupShiftPWM() {
  // put your setup code here, to run once:
  //pinMode(13, OUTPUT);
  //pinMode(11, OUTPUT);
  //pinMode(8, OUTPUT);

#ifdef Shift
  ShiftPWM.SetAmountOfRegisters(NUM_8BIT_SHIFT_REGISTERS);
  ShiftPWM.SetPinGrouping(1);  //This is the default, but I added here to demonstrate how to use the funtion
  ShiftPWM.Start(pwmFrequency, maxBrightness);
  ShiftPWM.SetAll(maxBrightness);
#endif
}

/**************************************************************************
 * Main loop.
 */
void loop() {
  maybeInitLocalVariables();
  currentTime = millis();
  Dcc.process();

  processTerminal();

  processAspectCode(counterNrSignalMast);
  counterNrSignalMast++;
  if (counterNrSignalMast >= numSignalNumber) {
    // Console.print("current mast ="); Console.print(counterNrSignalMast); Console.print(" numSignalNumber = "); Console.println(numSignalNumber);
    counterNrSignalMast = 0;
    aspectsInitialized = true;
  }

  processOutputLight(counterNrOutput);
  counterNrOutput++;
  if (counterNrOutput >= NUM_OUTPUTS) {
    counterNrOutput = 0;
  }
}

SignalMastData& signalData(byte no) {
  return signalMastData[no];
}

byte getMastOutput(byte nMast, byte light) {
  if (nMast >= numSignalNumber) {
    return ONA;
  }
  if (light >= maxOutputsPerMast) {
    return ONA;
  }
  int cvBase = sizeof(MastSettings) * nMast + START_CV_OUTPUT;
  byte o = Dcc.getCV(cvBase + light);
  if (o > NUM_OUTPUTS) {
    return ONA;
  } else {
    return o;
  }
}

/**************************************************************************
 * 
 */
int findSignalIndex(int address, int& position) {
  for (int i = 0; i < NUM_SIGNAL_MAST; i++) {
    int num = signalData(i).addressCount;
    // Console.print("Mast "); Console.print(i); Console.print(" Addresses: "); Console.println(num);
    if (address < num) {
      // Console.print("Found index "); Console.print(i); Console.print(" pos "); Console.println(address);
      position = address;
      return i;
    }
    address -= num;
  }
  return -1;
}

void notifyDccAccTurnoutOutput(uint16_t Addr, uint8_t Direction, uint8_t OutputPower) {
  // Console.println("DCC turnout");
  if (rocoAddress) {
    Addr = Addr + 4;
  }
  // Console.print("Addr = "); Console.println(Addr);
  // Console.print("Min = "); Console.println(thisDecoderAddress);
  // Console.print("Max = "); Console.println(maxDecoderAddress);
  if ((Addr < thisDecoderAddress)
      || (Addr >= maxDecoderAddress)) {
    return;
  }

  int pos;
  int signalIndex = findSignalIndex(Addr - thisDecoderAddress, pos);
  // Console.print(F("Turnout: ")); Console.print(Addr); Console.print(F(" = mast: ")); Console.print(signalIndex); Console.print(F(" pos: ")); Console.print(pos); Console.print(F(" dir: ")); Console.println(Direction);

  if (signalIndex == -1) {
    return;
  }

  signalMastChangePos(signalIndex, pos, Direction ? 0 : 1);
}

/**************************************************************************
 *
 */
struct CVPair {
  const uint16_t CV;
  const uint8_t Value;
};

const CVPair FactoryDefaultCVs[] = {
  { CV_ACCESSORY_DECODER_ADDRESS_LSB, INIT_DECODER_ADDRESS },
  { CV_ACCESSORY_DECODER_ADDRESS_MSB, 0 },
  { CV_AUXILIARY_ACTIVATION, VALUE_AUXILIARY_ACTIVATION },
  { CV_DECODER_KEY, VALUE_DECODER_KEY },
  { CV_DECODER_LOCK, VALUE_DECODER_LOCK },

  { CV_ROCO_ADDRESS, VALUE_ROCO_ADDRESS },
  { CV_FADE_RATE, VALUE_FADE_RATE },
  { CV_NUM_SIGNAL_NUMBER, VALUE_NUM_SIGNAL_NUMBER },
  { CV_ASPECT_LAG, VALUE_ASPECT_LAG },

  { CV_PROD_ID_1, VALUE_PROD_ID_1 },
  { CV_PROD_ID_2, VALUE_PROD_ID_2 },
  { CV_PROD_ID_3, VALUE_PROD_ID_3 },
  { CV_PROD_ID_4, VALUE_PROD_ID_4 },
};

const MastSettings factorySignalMastOutputs[] PROGMEM = {
  { { 1, 2, 3, 4, 5 }, 1 + (usesCodes | turnoutControl), 0, 0 },    // signal mast 0
//  { { 0, 6, 7, 8, 9 },     (usesCodes | turnoutControl), 0, 0 },    // signal mast 1
//  { { 0, 10, 11, 12, 13 }, (usesCodes | turnoutControl), 0, 0 },    // signal mast 2
//  { { 0, 14, 15, 16, 17 }, (usesCodes | turnoutControl), 0, 0 },    // signal mast 3
//  { { 0, 18, 19, 20, 21 }, (usesCodes | turnoutControl), 0, 0 },    // signal mast 4
//  { { 0, 22, 23, 24, 25 }, (usesCodes | turnoutControl), 0, 0 },    // signal mast 5
};

const uint8_t noSignalMastOutputs[] PROGMEM = {
  ONA, ONA, ONA, ONA, ONA, ONA, ONA, ONA, ONA, ONA, 0, 0, 5  // signal mast 7
};

void setFactoryDefault() {
  Console.println(F("Resetting to factory defaults"));
  uint8_t FactoryDefaultCVSize = sizeof(FactoryDefaultCVs) / sizeof(CVPair);

  // Console.println(F("Setting default CV values"));
  for (uint16_t i = 0; i < FactoryDefaultCVSize; i++) {
    Dcc.setCV(FactoryDefaultCVs[i].CV, FactoryDefaultCVs[i].Value);
  }

  int cvNumber = START_CV_OUTPUT;
  int pgmIndex = (int)(void*)factorySignalMastOutputs;
  // Console.println(F("Copying factorySignalMastOutputs"));
  for (unsigned int i = 0; i < sizeof(factorySignalMastOutputs); i++, cvNumber++) {
    // Console.print(F("CV ")); Console.print(cvNumber); Console.print(F(":=")); Console.println(pgm_read_byte_near(pgmIndex + i));
    Dcc.setCV(cvNumber, pgm_read_byte_near(pgmIndex + i));
  }

  // Console.println(F("Clearing rest of signal CVs"));

  for (unsigned int i = sizeof(factorySignalMastOutputs) / SEGMENT_SIZE; i < NUM_SIGNAL_MAST; i++) {
    for (unsigned int j = 0; j < sizeof(noSignalMastOutputs); j++, cvNumber++) {
      Dcc.setCV(cvNumber, pgm_read_byte_near(noSignalMastOutputs + j));
    }
  }

  // Console.println(F("Generating aspect table"));

  uint16_t OutputCV = START_CV_ASPECT_TABLE;
  for (uint16_t i = 0; i < NUM_SIGNAL_MAST; i++) {
    for (uint16_t j = 1; j <= maxAspects; j++) {
      Dcc.setCV(OutputCV, j);
      OutputCV++;
    }
  }

  // define CVs based on the predefined templates:
  // Console.print(F("pgmIndex base ")); Console.println((int)factorySignalMastOutputs, HEX);
  // Console.print(F(" count ")); Console.println(sizeof(factorySignalMastOutputs) / sizeof(factorySignalMastOutputs[0]));
  for (unsigned int mast = 0; mast < sizeof(factorySignalMastOutputs) / sizeof(factorySignalMastOutputs[0]); mast++) {
    const MastSettings& settingsDef = factorySignalMastOutputs[mast];
    
    // Console.print(F("pgmIndex for mast ")); Console.print(mast); Console.print(F(" = ")); Console.println((int)(&settingsDef), HEX);
    // Console.print(F("cvBase for mast ")); Console.print(mast); Console.print(F(" = ")); Console.println(cvBase);
    int pgmIndex2 = (int)(&settingsDef.signalSetOrMastType);
    
    byte signalSetOrMastType = pgm_read_byte_near(pgmIndex2);
    if ((signalSetOrMastType & usesCodes) > 0) {
      doChangeMastType(mast, signalSetOrMastType, false);
    }
  }
  initLocalVariables();
}

const struct MastTypeDefinition& copySignalMastTypeDefinition(byte typeId);

/* __attribute__((noinline)) */ void doChangeMastType(int mast, int signalSetOrMastType, boolean stable) {
  // Console.print(F("Using template for mast ")); Console.println(mast);

  // NOT used as an actual address, but only as a number for computing 'cv'
  const MastSettings& settingsDef = ((MastSettings*)factorySignalMastOutputs)[0];
  int cvBase = (sizeof(MastSettings) * mast) + START_CV_OUTPUT;
  int cv = cvBase + ((int)(&settingsDef.defaultCodeOrAspect)) - ((int)(&settingsDef));

  const struct MastTypeDefinition& def = copySignalMastTypeDefinition(toTemplateIndex(signalSetOrMastType));
  int codeCount = def.codeCount;
  //int lightCount = def.lightCount;

  // printByteBuffer((byte*)&def, sizeof(def)); Console.println();
  saveTemplateOutputsToCVs(def, mast, stable);

  byte val = def.defaultCode;
  // Console.print(F("Setting cv default code ")); Console.print(cv); Console.print(F(" to ")); Console.println(val);
  Dcc.setCV(cv, val);
  cv++;
  val = findRequiredAddrCount(codeCount, signalSetOrMastType);
  // Console.print(F("Setting cv addresses ")); Console.print(cv); Console.print(F(" to ")); Console.println(val);
  Dcc.setCV(cv, val);

  cv = START_CV_ASPECT_TABLE + mast * maxAspects;
  for (unsigned int i = 0; i < sizeof(def.code2Aspect); i++) {
    Dcc.setCV(cv, def.code2Aspect[i]);
    cv++;
  }
}

__attribute__((noinline)) void changeMastType(int mastIndex, int newType) {
  if (newType == 0) {
    int cv = START_CV_OUTPUT + (mastIndex * SEGMENT_SIZE);
    for (unsigned int i = 0; i < maxOutputsPerMast; i++) {
      Dcc.setCV(cv + i, 0);
    }
    // number of addresses.
    Dcc.setCV(cv + SEGMENT_SIZE - 1, 0);
    return;
  }
  if ((newType & usesCodes) == 0) {
    return;
  }
  doChangeMastType(mastIndex, newType, true);
}

byte getMastTypeOrSignalSet(int mastIndex) {
    int cv = START_CV_OUTPUT + (mastIndex * SEGMENT_SIZE);
    return notifyCVRead(cv + maxOutputsPerMast);
}

void printByteBuffer(const byte* buf, int size) {
  for (int i = 0; i < size; i++) {
    if (i > 0) {
      Console.print('-');
    }
    int n = *buf;
    buf++;
    if (n < 0x10) {
      Console.print('0');
    }
    Console.print(n, HEX);
  }
}

int findNumberOfSignals(int addresses, int mastType) {
  if (mastType & usesCodes) {
    const struct MastTypeDefinition& def = copySignalMastTypeDefinition(toTemplateIndex(mastType));
    return def.codeCount;
  }
  switch (mastType & signalSetControlType) {
    case turnoutNoDirection:
      return addresses;
    case extendedPacket:
      return 32;
    case turnoutControl:
      return addresses * 2;
    case bitwiseControl:
      return addresses * 8;
    default:
      return 0;
  }
}

int findRequiredAddrCount(int codes, int mastType) {
  switch (mastType & signalSetControlType) {
    case turnoutNoDirection:
      return codes;
    case extendedPacket:
      return 1;
    case turnoutControl:
      return (codes + 1) / 2;
    case bitwiseControl:
      for (int i = 0; i < 8; i++) {
        int max = (1 << i);
        if (max > codes) {
          return i + 1;
        }
      }
      return -1;
    default:
      return 1;
  }
}

int findMinLightIndex(int mast) {
  int cvStart = START_CV_OUTPUT + mast * SEGMENT_SIZE;
  int min = 255;
  for (int i = cvStart; i < cvStart + maxOutputsPerMast; i++) {
    int x = Dcc.getCV(i);
    if (x == ONA || x > NUM_OUTPUTS) {
      continue;
    }
    if (min > x) {
      min = x;
    }
  }
  return min == 255 ? -1 : min;
}

int findLightCount(int mast) {
  int cnt = 0; 
  int cvStart = START_CV_OUTPUT + mast * SEGMENT_SIZE;
  for (int i = cvStart; i < cvStart + maxOutputsPerMast; i++) {
    int x = Dcc.getCV(i);
    if (x == ONA || x > NUM_OUTPUTS) {
      continue;
    }
    cnt++;
  }
  return cnt;
}

int findNextLight(int mast) {
  if (mast == 0) {
    return 1;
  }

  int cvStart;
  int prevMast = mast - 1;
  byte type;
  do {
    cvStart = START_CV_OUTPUT + prevMast * SEGMENT_SIZE;
    //Console.print(F("cvStart ")); Console.println(cvStart);
    type = Dcc.getCV(cvStart + maxOutputsPerMast);    
    if (type > 0) {
      break;
    }
    prevMast--;
  } while (prevMast >= 0);
  if (prevMast < 0) {
    return 1;
  }
  int max = -1;
  //Console.print(F("Searching from ")); Console.println(cvStart);
  for (int i = cvStart; i < cvStart + maxOutputsPerMast; i++) {
    int x = Dcc.getCV(i);
    if (x == ONA || x > NUM_OUTPUTS) {
      continue;
    }
    if (max < x) {
      max = x;
    }
  }
  return max < 0 ? -1 : max + 1;
}

void saveTemplateAspectsToCVs(int mast, int signalSetOrMastType) {
  // Console.print(F("Using template aspects for mast ")); Console.println(mast);

  const struct MastTypeDefinition& def = copySignalMastTypeDefinition(toTemplateIndex(signalSetOrMastType));

  printByteBuffer((byte*)&def.code2Aspect, sizeof(def.code2Aspect)); Console.println();

  int cv = START_CV_ASPECT_TABLE + mast * maxAspects;
  for (unsigned int i = 0; i < sizeof(def.code2Aspect); i++) {
    Dcc.setCV(cv, def.code2Aspect[i]);
    cv++;
  }
}  

void saveTemplateOutputsToCVs(const struct MastTypeDefinition& def, int mastIndex, boolean stable) {
  // Console.print(F("Setting outputs to mast #")); Console.print(mastIndex); Console.print(F(" from def ")); Console.println((int)(int*)(&def), HEX);
  int minLightNumber = findMinLightIndex(mastIndex);
  //int oldLightCount = findLightCount(mastIndex);
  int from = findNextLight(mastIndex);

  if (stable && (minLightNumber >= 0)) {
    from = minLightNumber;
  }
  saveTemplateOutputsToCVs(def, mastIndex, from);
}

void saveTemplateOutputsToCVs(const struct MastTypeDefinition& def, int mastIndex, int from) {
  int lc = 0;
  int cvIndex = (sizeof(MastSettings) * mastIndex) + START_CV_OUTPUT;
  for (unsigned int i = 0; i < sizeof(def.outputs); i++) {
    byte n = def.outputs[i];
    if (n != ONA && n != 0 && n <= NUM_OUTPUTS) {
      n += from;
      n--;
      lc++;
    } else {
      n = ONA;
    }
    // Console.print(F("Set CV ")); Console.print(cvIndex); Console.print(F(" := ")); Console.println(n);
    Dcc.setCV(cvIndex, n);
    cvIndex++;
  }
  // skip the aspect table/template.
  cvIndex++;
  Dcc.setCV(cvIndex, def.defaultCode);
}

__attribute__((noinline)) int reassignMastOutputs(int mastIndex, int from, boolean propagate) {
  int nextInput = -1;

  while (mastIndex < NUM_SIGNAL_MAST) {
    nextInput = reassignMastOutputs2(mastIndex, from);
    if (!propagate) {
      return nextInput;
    }
    int nextMast;
    for (nextMast = mastIndex + 1; nextMast < NUM_SIGNAL_MAST; nextMast++) {
      int cvStart = START_CV_OUTPUT + nextMast * SEGMENT_SIZE;
      byte type = Dcc.getCV(cvStart + maxOutputsPerMast);
      //Console.print("Next mast: "); Console.print(nextMast); Console.print(" type "); Console.println(type, HEX);
      if (type == 0) {
        continue;
      }
      int min = findMinLightIndex(nextMast);
      //Console.print("minLight: "); Console.print(min);  Console.print(" nextInput "); Console.println(nextInput);
      if (min == nextInput) {
        return -1;
      }
      break;
    }
    mastIndex = nextMast;
    from = nextInput;
  }
  return nextInput;
}

int reassignMastOutputs2(int mastIndex, int from) {
  int cvIndex = (sizeof(MastSettings) * mastIndex) + START_CV_OUTPUT;
  int minLightNumber = findMinLightIndex(mastIndex);
  
  int lastOutput = -1;

  // Console.print(F("Reassigning outputs for ")); Console.print(mastIndex); Console.print(F(" start from ")); Console.println(from);

  for (unsigned int i = 0; i < sizeof(MastSettings::outputs); i++) {
    byte n = getMastOutput(mastIndex, i);
    if (n != ONA) {
      n = (n - minLightNumber) + from;
      Dcc.setCV(cvIndex, n);
      lastOutput = n;
    }
    cvIndex++;
  }
  lastOutput++;
  return lastOutput;
}

/**********************************************************************************
 * Init local variables.
 */
/* __attribute__((noinline)) */void initLocalVariables() {
  reinitializeLocalVariables = true;
}

__attribute__((noinline)) void maybeInitLocalVariables() {
  if (!reinitializeLocalVariables) {
    return;
  }
  Console.println("Reinitializing...");
  reinitializeLocalVariables = false;
  lsb = Dcc.getCV(CV_ACCESSORY_DECODER_ADDRESS_LSB);
  msb = Dcc.getCV(CV_ACCESSORY_DECODER_ADDRESS_MSB);
  thisDecoderAddress = (msb << 8) | lsb;

  decoderKey = Dcc.getCV(CV_DECODER_KEY);
  decoderLock = Dcc.getCV(CV_DECODER_LOCK);

  rocoAddress = Dcc.getCV(CV_ROCO_ADDRESS);

  fadeRate = Dcc.getCV(CV_FADE_RATE);
  initializeFadeTime();

  numSignalNumber = Dcc.getCV(CV_NUM_SIGNAL_NUMBER);
  aspectLag = Dcc.getCV(CV_ASPECT_LAG);
  aspectLag = aspectLag << 7;

  initLocalVariablesSignalMast();
}

void initializeFadeTime() {
  fadeRate = fadeRate & 0x07;
  for (int i = 0; i < 11; i++) {
    fadeTimeLight[i] = FADE_TIME_LIGHT[i] * fadeRate;
  }
}

uint8_t recordOutput(byte* bitmap, uint8_t output) {
  if (output < 1 || output == ONA || output >= NUM_OUTPUTS) {
    return 0;
  }
  uint8_t o = output - 1;
  bitSet(bitmap[o / 8], o % 8);
  return output;
}

/**********************************************************************************
 * Init local variables for signal mast.
 */
void initLocalVariablesSignalMast() {

  int counter = START_CV_OUTPUT;
  static byte usedOutputs[(NUM_OUTPUTS + 7) / 8] = { 0 };

  for (int i = 0; i < NUM_SIGNAL_MAST; i++) {
    // record for each output
    for (int l = 0; l < maxOutputsPerMast; l++) {
      recordOutput(usedOutputs, Dcc.getCV(counter));  // yellow upper
      counter++;
    }

    byte mastTypeOrSignalSet = Dcc.getCV(counter);
    counter++;
    byte signalSet = 0;

    if ((mastTypeOrSignalSet & usesCodes) > 0) {
      signalSet = findMastTypeSignalSet(mastTypeOrSignalSet);
    } else {
      signalSet = mastTypeOrSignalSet;
    }
    signalSet = signalSet & (~signalSetFlags);
    if (signalSet >= maxSignalSets) {
      signalSet = 0;
    }

    SignalMastData& data = signalData(i);

    data.set = (SignalSet)signalSet;

    byte defaultAspectIdx = Dcc.getCV(counter);
    counter++;

    data.defaultAspect = defaultAspectIdx;
    data.addressCount = Dcc.getCV(counter);
    // Console.print(F("Addresses: ")); Console.println(signalMastNumberAddress[i]);
    counter++;

    data.signalCount = findNumberOfSignals(data.addressCount, mastTypeOrSignalSet);

  }

  // clear out all unused outputs: set them to HIGH, so the voltage diff against common + will be 0.
  for (int j = 0; j < NUM_OUTPUTS; j++) {
    boolean used = bitRead(usedOutputs[j / 8], j % 8);
    if (!used) {
#ifdef Shift
      ShiftPWM.SetOne(numberToPhysOutput(j), 0);
#endif        
    }
  }

  maxDecoderAddress = thisDecoderAddress;
  for (int i = 0; i < numSignalNumber; i++) {
    const SignalMastData& data = signalData(i);
    maxDecoderAddress = maxDecoderAddress + data.addressCount;
    signalMastData[i].setCode(data.defaultAspect);
  }
  // Console.print(F("Max address: ")); Console.println(maxDecoderAddress);
}

unsigned int timeElapsedForBulb(byte nrOutput) {
  unsigned int start = (lightStartTimeBubl[nrOutput] & 0xffff);
  if (start == 0) {
    if (aspectsInitialized) {
      return UINT16_MAX;
    } else {
      // special case for bootstrap: pretend fade in/out time elapsed.
      return fadeTimeLight[(sizeof(fadeTimeLight) / sizeof(fadeTimeLight[0]))- 1] + 20;
    }
  }
  unsigned int cur = currentTime & 0xffff;
  int span;

  if (start > cur) {
    span = (unsigned int)((0x10000 - start) + cur);
  } else {
    span = cur - start;
  }
  if (span >= INT16_MAX) {
    return INT16_MAX;
  } else {
    return (int)span;
  }
}


/**********************************************************************************
 *
 */
void processOutputLight(byte nrOutput) {
  switch (bublState2[nrOutput].sign) {
    case fixed:
      processBulbBlinking(nrOutput, 0);
      break;
    case blinking54:
      processBulbBlinking(nrOutput, BLINKING_TIME_54);
      break;
    case blinking108:
      processBulbBlinking(nrOutput, BLINKING_TIME_108);
      break;
    case blinking45:
      processBulbBlinking(nrOutput, BLINKING_TIME_45);
      break;
    case blinking22:
      processBulbBlinking(nrOutput, BLINKING_TIME_22);
      break;
    case inactive:
    case _lightsign_last:
      break;
  }
}

void changeLightState2(byte lightOutput, struct LightFunction newState) {
  if (lightOutput == 0 || lightOutput == ONA || lightOutput > NUM_OUTPUTS) {
    return;
  }
  lightOutput--;
  LightFunction& bs = bublState2[lightOutput];
  if (debugLights) {
    Console.print(F("Change light #"));
    Console.print(lightOutput);
    Console.print(F(" curState: "));
    Console.print(bs.sign);
    Console.print(F(", off = "));
    Console.print(bs.off);
    Console.print(F(", newstate: "));
    Console.print(newState.sign);
    Console.print(F(", off = "));
    Console.println(newState.off);
  }

  boolean wasOff = bs.off;
  if (bs.sign == newState.sign) {
    // exception: fixed(on) != fixed(off)
    if (newState.sign != fixed || newState.off == wasOff) {
      return;
    }
  }

  if (aspectsInitialized) {
    resetStartTime(lightOutput);
  } else {
    lightStartTimeBubl[lightOutput] = 0;
  }
  bs = newState;
  if (newState.sign != fixed) {
    bs.off = !wasOff;
  }
  if (debugLights) {
    Console.print(F("*set light #"));
    Console.print(lightOutput);
    Console.print(F(" newState: "));
    Console.print(bublState2[lightOutput].sign);
    Console.print(F(", off = "));
    Console.println(bublState2[lightOutput].off);
  }
}

LightFunction getBulbState(byte n) {
  if (n == 0 || n > NUM_OUTPUTS) {
    return (LightFunction)0;
  } else {
    return bublState2[n - 1];
  }
}

/**
 * The 74HC595 on the PCB is placed so that its outputs are mirrored: MSB is at the edge, while LSB is in the middle of the
 * pin line. We need to reverse output position within each 8-bit sequence
 */
inline byte numberToPhysOutput(byte nrOutput) {
  return (nrOutput & (~0x07)) + (7 - (nrOutput & 0x07));
}

void setPWM(byte nrOutput, byte level) {
  boolean b = bitRead(overrides[nrOutput / 8], nrOutput & 0x07);
  if (b) {
    return;
  }
#ifdef Shift
  ShiftPWM.SetOne(numberToPhysOutput(nrOutput), level);
#endif
}

void processBulbBlinking(int nrOutput, unsigned int blinkDelay) {
  if (nrOutput >= NUM_OUTPUTS) {
    return;
  }
  boolean off = bublState2[nrOutput].off;
  unsigned int elapsed = timeElapsedForBulb(nrOutput);
  if (elapsed > 0xff00) {
    // just to be sure, elapsed time is too large, ignore the light.
    return;
  }
  if (elapsed > fadeTimeLight[10]) {
    if (!bublState2[nrOutput].end) {
      if (debugLightFlip) {
        Console.print(F("Light elapsed "));
        Console.print(nrOutput);
        Console.print(F(" off = "));
        Console.println(off);
      }
      bublState2[nrOutput].end = true;
      setPWM(nrOutput, off ? minBrightness : maxBrightness);
    }

    if (blinkDelay > 0) {
      if (elapsed > blinkDelay) {
        if (debugLightFlip) {
          Console.print(F("Light blinked "));
          Console.println(nrOutput);
        }
        bublState2[nrOutput].end = false;
        bublState2[nrOutput].off = !off;
        resetStartTime(nrOutput);
      }
    } else {
      // disable state changes.
      if (debugLightFlip) {
        Console.print(F("Light fixed "));
        Console.print(nrOutput);
        Console.print(F(" state "));
        Console.println(!off);
      }
      lightStartTimeBubl[nrOutput] = 0;
    }
    return;
  }
  if (off) {
    processFadeOff(nrOutput);
  } else {
    processFadeOn(nrOutput);
  }
}


/**********************************************************************************
 * 
 */
void signalMastChangePos(int nrSignalMast, uint16_t pos, uint8_t Direction) {
  if (nrSignalMast >= NUM_SIGNAL_MAST) {
    Console.print(F("Error: mastChangePos mast out of range: "));
    Console.println(nrSignalMast);
    return;
  }
  SignalMastData& data = signalData(nrSignalMast);
  if (pos >= data.addressCount) {
    Console.print(F("Error: mastChangePos pos out of range"));
    Console.println(pos);
    return;
  }
  
  byte oldAspectIdx = data.getLastCode();
  byte newAspectIdx = oldAspectIdx;
  // Console.print("existing aspect: "); Console.println(newAspectIdx);
  byte type = getMastTypeOrSignalSet(nrSignalMast);
  switch (type & signalSetControlType) {
    case turnoutNoDirection:
      newAspectIdx = pos;
      break;
    // signal controlled by an extended packet
    case extendedPacket:
      // not supported yet
      break;
    // signal controlled as a series of turnouts
    case turnoutControl:
      newAspectIdx = (2 * pos) | (Direction ? 1 : 0);
      break;
    // signal controlled by a binary number encoded into turnout states
    case bitwiseControl:
      // Console.println("Bitwise");
      bitWrite(newAspectIdx, pos, Direction);
      break;
  }
  newAspectIdx = newAspectIdx & (maxAspects - 1);
  // Console.print(F("mast: ")); Console.print(nrSignalMast); Console.print(F(" type:")); Console.print(type, HEX); Console.print(F(" pos: ")); Console.print(pos); Console.print(F(" dir:")); Console.println(Direction);
  // Console.print(F("new aspect: ")); Console.println(newAspectIdx);

  if (oldAspectIdx == newAspectIdx) {
    return;
  }

  if (newAspectIdx >= maxAspects) {
    Console.print(F("Error: newAspectIdx out of range"));
    Console.println(newAspectIdx);
    return;
  }
  data.setCode(newAspectIdx);
}

/**********************************************************************************
 *
 */
void processAspectCode(int nrSignalMast) {
  SignalMastData& data = signalData(nrSignalMast);
  if (aspectsInitialized && !data.isReadyForProcess()) {
    return;
  }

  byte newCode = data.processed();
  byte newAspect = aspectJmri(nrSignalMast, newCode);
  signalMastChangeAspect(nrSignalMast, newAspect);
}

void signalMastChangeAspectCsdBasic(SignalMastData& data, int nrSignalMast, byte newAspect);
void signalMastChangeAspectCsdMechanical(SignalMastData& data, int nrSignalMast, byte newAspect);
void signalMastChangeAspectCsdIntermediate(SignalMastData& data, int nrSignalMast, byte newAspect);
void signalMastChangeAspectCsdEmbedded(SignalMastData& data, int nrSignalMast, byte newAspect);
void signalMastChangeAspectSzdcBasic(SignalMastData& data, int nrSignalMast, byte newAspect);

/**********************************************************************************
 *
 */

void signalMastChangeAspect(int nrSignalMast, byte newAspect) {
  SignalMastData& data = signalData(nrSignalMast);
  signalMastChangeAspect(data, nrSignalMast, newAspect);
}

void signalMastChangeAspect(SignalMastData& data, int nrSignalMast, byte newAspect) {
  // diagnostics: light up everything.
  if (newAspect == 255) {
    signalMastChangeAspectCsdBasic(data, nrSignalMast, 26);
    return;
  }
  if (newAspect > maxAspects || newAspect == 0) {
    // aspect is invalid, or undefined all off
    signalMastChangeAspectCsdMechanical(data, nrSignalMast, 1);
    return;
  }
  newAspect--;
  if (aspectsInitialized && (data.currentAspect == newAspect)) {
    return;
  }
  const byte set = data.set;
  switch (set) {
    case SIGNAL_SET_CSD_BASIC:
      signalMastChangeAspectCsdBasic(data, nrSignalMast, newAspect);
      break;
    case SIGNAL_SET_CSD_INTERMEDIATE:
      signalMastChangeAspectCsdIntermediate(data, nrSignalMast, newAspect);
      break;
    case SIGNAL_SET_CSD_EMBEDDED:
      signalMastChangeAspectCsdEmbedded(data, nrSignalMast, newAspect);
      break;
    case SIGNAL_SET_SZDC_BASIC:
      signalMastChangeAspectSzdcBasic(data, nrSignalMast, newAspect);
      break;
    case SIGNAL_SET_CSD_MECHANICAL:
      signalMastChangeAspectCsdMechanical(data, nrSignalMast, newAspect);
      break;
    case DisabledSignalSet:
    case _signal_set_last:
      break;
  }
}

/**********************************************************************************
 *
 */
void signalMastChangeAspect(int progMemOffset, int tableSize, int nrSignalMast, SignalMastData& data, byte newAspect) {
  if (debugAspects) {
    Console.print(F("Change mast "));
    Console.print(&data - signalMastData);
    Console.print(F(" to aspect "));
    Console.println(newAspect);
  }
  if (newAspect >= tableSize) {
    Console.print(msg_InvalidAspect);
    Console.println(newAspect);
    return;
  }
  data.currentAspect = newAspect;

  // Configuration of light outputs
  LightFunction buffer[maxOutputsPerMast];

  // Must copy bytes from PROGMEM to bufferon stack:
  byte* out = (byte*)(void*)&(buffer[0]);
  for (unsigned int i = 0; i < sizeof(buffer); i++) {
    *out = pgm_read_byte_near(progMemOffset + i);
    out++;
  }

  // Process LightFunction instructions
  int cvBase = (sizeof(MastSettings) * nrSignalMast) + START_CV_OUTPUT;
  for (int i = 0; i < maxOutputsPerMast; i++) {
    int nOutput = Dcc.getCV(cvBase + i);
    if (debugAspects) {
      Console.print("Change output "); Console.print(nOutput); Console.print(" => "); Console.println(*(byte*)(void*)&buffer[i]);
    }
    changeLightState2(nOutput, buffer[i]);
  }
}

void resetStartTime(int lightOutput) {
  unsigned int v = currentTime & 0xffff;
  if (v == 0) {
    v++;
  }
  lightStartTimeBubl[lightOutput] = v;
}

/**********************************************************************************
 *
 */
void processFadeOnOrOff(byte nrOutput, boolean fadeOn) {
  int idx;
  unsigned int elapsed = timeElapsedForBulb(nrOutput);

  int limit = (sizeof(fadeTimeLight) / sizeof(fadeTimeLight[0]));

  if (debugFadeOnOff) {
    if (fadeOn) {
      Console.print(F("Fade ON, output="));
    } else {
      Console.print(F("Fade OFF, output="));
    }
    Console.print(nrOutput);
  }
  if (elapsed > fadeTimeLight[limit - 1]) {
    if (debugFadeOnOff) {
      Console.println(F("Reached limit"));
    }
    setPWM(nrOutput, fadeOn ? maxBrightness : minBrightness);
    return;
  }
  for (idx = 0; idx < limit && elapsed > fadeTimeLight[idx]; idx++)
    ;
  // idx will be one higher than last less elapsed time, so if elapsed > fadeTimeLight[idx], then becomes 2.
  if (idx >= limit) {
    // should never happen, but prevents out-of-range access
    setPWM(nrOutput, fadeOn ? maxBrightness : minBrightness);
    return;
  }

  int span = (maxBrightness - minBrightness);
  int pwm = ((span * FADE_COUNTER_LIGHT_1[idx]) / FADE_COUNTER_LIGHT_2[idx]);
  if (fadeOn) {
    pwm += minBrightness;
  } else {
    pwm = maxBrightness - pwm;
  }
  if (debugFadeOnOff) {
    Console.print(" idx=");
    Console.print(idx);
    Console.print(", counters: ");
    Console.print(FADE_COUNTER_LIGHT_1[idx]);
    Console.print("/");
    Console.print(FADE_COUNTER_LIGHT_2[idx]);
    Console.print(", pwm = ");
    Console.print(pwm);
    Console.print(", time=");
    Console.println(elapsed);
  }
  setPWM(nrOutput, pwm);
}

void processFadeOn(byte nrOutput) {
  processFadeOnOrOff(nrOutput, true);
  unsigned int idx = 0;
  unsigned int limit = (sizeof(fadeTimeLight) / sizeof(fadeTimeLight[0]));

  unsigned int elapsed = timeElapsedForBulb(nrOutput);
  for (idx = 0; idx < limit && elapsed > fadeTimeLight[idx]; idx++)
    ;
  int span = (maxBrightness - minBrightness);
  int pwm = idx >= limit ? maxBrightness : ((span * FADE_COUNTER_LIGHT_1[idx]) / FADE_COUNTER_LIGHT_2[idx]) + minBrightness;

  setPWM(nrOutput, pwm);

  if (debugFadeOnOff) {
    Console.print("Fade ON, output=");
    Console.print(nrOutput);
    Console.print("idx=");
    Console.print(idx);
    Console.print(", counters: ");
    Console.print(FADE_COUNTER_LIGHT_1[idx]);
    Console.print("/");
    Console.print(FADE_COUNTER_LIGHT_2[idx]);
    Console.print(", time=");
    Console.print(elapsed);
    Console.print(" pwm=");
    Console.println(pwm);
  }
}

/**********************************************************************************
 *
 */
void processFadeOff(byte nrOutput) {
  processFadeOnOrOff(nrOutput, false);
  int idx;
  int limit = (sizeof(fadeTimeLight) / sizeof(fadeTimeLight[0]));
  unsigned int elapsed = timeElapsedForBulb(nrOutput);
  for (idx = 0; idx < limit && elapsed > fadeTimeLight[idx]; idx++)
    ;

  int span = (maxBrightness - minBrightness);
  int pwm = idx >= limit ? minBrightness : maxBrightness - ((span * FADE_COUNTER_LIGHT_1[idx]) / FADE_COUNTER_LIGHT_2[idx]);

  setPWM(nrOutput, pwm);

  if (debugFadeOnOff) {
    Console.print("Fade OFF, output=");
    Console.print(nrOutput);
    Console.print("idx=");
    Console.print(", counters: ");
    Console.print(FADE_COUNTER_LIGHT_1[idx]);
    Console.print("/");
    Console.print(FADE_COUNTER_LIGHT_2[idx]);
    Console.print(", time=");
    Console.print(elapsed);
    Console.print(" pwm=");
    Console.println(pwm);
  }
}

/**************************************************************************
 *
 */
void notifyCVAck() {

  digitalWrite(ACK_BUSY_PIN, HIGH);
  delay(6);
  digitalWrite(ACK_BUSY_PIN, LOW);
}

/**************************************************************************
 * CV was changed.
 */
void notifyCVChange(uint16_t CV, uint8_t Value) {
  if (CV >= START_CV_OUTPUT && CV <= END_CV_OUTPUT) {
    int diff = CV - START_CV_OUTPUT;
    if ((diff % SEGMENT_SIZE) == maxOutputsPerMast) {
      changeMastType(diff / SEGMENT_SIZE, Value);
    }
    initLocalVariables();
    return;
  }
  if (CV >= START_CV_OUTPUT_BASE && CV <= END_CV_OUTPUT_BASE) {
    int mast = CV - START_CV_OUTPUT_BASE;
    boolean shift;
    if (Value > 100) {
      Value -= 100;
      shift = true;
    } else {
      shift = false;
    }
    reassignMastOutputs(mast, Value, shift);
    initLocalVariables();
  }
}

void notifyDccCVChange(uint16_t CV, uint8_t Value) {
  /*
  Console.print(F("DCC CV changed: "));
  Console.print(CV);
  Console.print(F(" := "));
  Console.println(Value);
  */
  if (CV == CV_ACCESSORY_DECODER_ADDRESS_LSB) {
    lsb = Value;
    thisDecoderAddress = (msb << 8) | lsb;
    return;
  }

  if (CV == CV_ACCESSORY_DECODER_ADDRESS_MSB) {
    msb = Value;
    thisDecoderAddress = (msb << 8) | lsb;
    return;
  }

  if (CV == CV_MULTIFUNCTION_EXTENDED_ADDRESS_LSB) {
    Dcc.setCV(CV_ACCESSORY_DECODER_ADDRESS_LSB, Value);
    lsb = Value;
    thisDecoderAddress = (msb << 8) | lsb;
    return;
  }

  if (CV == CV_MULTIFUNCTION_EXTENDED_ADDRESS_MSB) {
    Value = Value & 0x07;
    Dcc.setCV(CV_ACCESSORY_DECODER_ADDRESS_MSB, Value);
    msb = Value;
    thisDecoderAddress = (msb << 8) | lsb;
    return;
  }

  if (CV == CV_ROCO_ADDRESS) {
    rocoAddress = Value;
    return;
  }

  if (CV == CV_NUM_SIGNAL_NUMBER) {
    numSignalNumber = Value;
    return;
  }

  if (CV == aspectLag) {
    aspectLag = Value;
    aspectLag = aspectLag << 7;
    return;
  }

  if (CV == CV_DECODER_KEY) {
    decoderKey = Value;
    return;
  }

  if (CV == CV_DECODER_LOCK) {
    decoderLock = Value;
    return;
  }

  if (CV == CV_FADE_RATE) {
    fadeRate = Value;
    initializeFadeTime();
    return;
  }
}

/******************************************************************************
 * Check CV valid.
 */
uint8_t notifyCVValid(uint16_t CV, uint8_t Writable) {

  uint8_t unlocked = isUnlocked(CV);

  if (unlocked && Writable && (CV == CV_MANUFACTURER_ID)) {
    setFactoryDefault();
    return false;
  }

  if (!unlocked) {
    return false;
  }

  if (CV > MAXCV) {
    return false;
  }

  if (Writable && (CV == CV_VERSION_ID)) {
    return false;
  }

  return true;
}

/********************************************************************
 * Test for unlocked key.
 */
uint8_t isUnlocked(uint16_t CV) {

  if (CV == CV_DECODER_KEY) {
    return true;
  }

  if (decoderKey == 255) {
    return true;
  }

  if (decoderLock == decoderKey) {
    return true;
  }

  return false;
}

/********************************************************************
 * 
 */
byte aspectJmri(int nrSignalMast, byte aspectMx) {

  byte aspectJmri;

  uint16_t cvAdr = START_CV_ASPECT_TABLE;

  cvAdr = cvAdr + maxAspects * nrSignalMast + aspectMx;

  aspectJmri = Dcc.getCV(cvAdr);

  return aspectJmri;
}

extern const MastTypeDefinition mastTypeDefinitions[];

const struct MastTypeDefinition& copySignalMastTypeDefinition(byte typeId) {

  typeId &= (~signalSetFlags);
  if (typeId >= mastTypeDefinitionCount) {
    typeId = 0;
  }
  // Console.print(F("Copy from id ")); Console.println(typeId);
  static MastTypeDefinition buffer;
  int progMemOffset = (int)(void*)(mastTypeDefinitions + typeId);
  byte* out = (byte*)(void*)&(buffer);
  for (unsigned int i = 0; i < sizeof(buffer); i++) {
    *out = pgm_read_byte_near(progMemOffset + i);
    out++;
  }
  return buffer;
}

byte findMastTypeSignalSet(byte typeId) {
  typeId &= (~signalSetFlags);
  if (typeId >= mastTypeDefinitionCount) {
    typeId = 0;
  }

  int progMemOffset = (int)(void*)&mastTypeDefinitions[typeId].signalSet;
  byte signalSet = pgm_read_byte_near(progMemOffset);
  if (signalSet >= _signal_set_last) {
    signalSet = 0;
  }
  return signalSet;
}

uint8_t notifyCVRead(uint16_t cv) {
  return EEPROM.read(cv);
}
