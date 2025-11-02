#ifndef __signalshift_h__
#define __signalshift_h__

#include <Arduino.h>

/**
 * Maximum possible outputs (lights) for a single mast. The value here affects the CV layout: for each Mast,
 * a consecutive CV list specifies physical outputs of individual Mast lights. Changing the value here will
 * damage existing decoder configurations as the CVs will be shifted.
 */
const int maxOutputsPerMast = 10;

/**
 * Maximum aspects supported per one signal table.
 */
const int maxAspects = 32;

/**
 * Maximum number of outputs. This value does not affect the CV layout, but is used as a dimension of status arrays.
 */
const int NUM_OUTPUTS = 80;

/**
 * Maximum number of signal masts supported by a single decoder.
 */
const int NUM_SIGNAL_MAST = 16;

const int SEGMENT_SIZE = maxOutputsPerMast + 3;

extern unsigned long currentTime;
extern unsigned int aspectLag;

#define LON (fixed)
#define LOFF (fixed | 0x10)
#define L(n) (n)
// originally, blinking had a bit flag, but that's not necessary
#define B(n) (n)

// Signs on an individual light. Fixed (on, off) and blinking.
enum LightSign {
  inactive = 0x00,  // since initialized globals are zeroed after initializer till end of object
  fixed = 0x01,
  blinking54,
  blinking108,
  blinking45,
  blinking22,

  _lightsign_last  // must be last
};

/**
 * Describes the state of a light output. Defined as a structure for
 * easier access; the alternative would be a bit manipulation.
 */
struct LightFunction {
  /**
   * The sign signalled by the light. This field 
   */
  LightSign sign : 4;
  static_assert(_lightsign_last <= 16, "Too many signs, must fit in 4 bits (LightFunction::sign)");

  /**
   * True to change the light towards OFF, false to change towards ON.
   */
  boolean off : 1;

  /**
   * If true, the light reached the on/off terminal state
   */
  boolean end : 1;

  // Default initializer: output off(=true), final brightness (end=true), not alternating
  LightFunction()
    : sign(inactive), off(true), end(true) {}

  // Normal construction
  LightFunction(LightSign sign, boolean initOff)
    : sign(sign), off(initOff), end(false) {}

  LightFunction(byte data) {
    *((byte*)(void*)this) = data;
  }

  // Copy constructor for easy assignment; make fast assingment of the whole byte.
  LightFunction(const LightFunction& f) {
    *((byte*)(void*)this) = *((byte*)(void*)&f);
  }

  inline boolean isFinished() {
    return end;
  }

  inline boolean towardsOn() {
    return !off;
  }

  inline boolean towardsOff() {
    return off;
  }
};

// Signal sets defined in the decoder
enum SignalSet : byte {
  DisabledSignalSet = 0,
  SIGNAL_SET_CSD_BASIC = 1,         // ČSD basic signal set
  SIGNAL_SET_CSD_INTERMEDIATE = 2,  // ČSD intermediate signal set
  SIGNAL_SET_CSD_EMBEDDED = 3,      // ČSD embedded signal set
  SIGNAL_SET_SZDC_BASIC = 4,        // SŽDC basic signal set
  SIGNAL_SET_CSD_MECHANICAL = 5,    // ČSD mechanical signal set

  _signal_set_last  // must be last
};

const byte maxSignalSets = 16;
const byte maxMastTypes = 32;

enum SignalSetFlags : byte {
  // signal controlled by a binary number encoded into turnout states
  bitwiseControl = 0x00,
  // each address represents one signal
  turnoutNoDirection = 0x20,
  // signal controlled by an extended packet
  extendedPacket = 0x40,
  // signal controlled as a series of turnouts
  turnoutControl = 0x60,
  // if set, the signal is rather driven by a codetable (which maps to aspects),
  // rather than the aspect table itself.
  usesCodes  = 0x80,

  signalSetFlags = 0xe0,
  signalSetType = (byte)~signalSetFlags,
  signalSetControlType = (signalSetFlags - usesCodes)
};
static_assert(_signal_set_last <= ((~signalSetFlags) & 0xff), "Too many signal sets");

inline byte toSignalSetIndex(byte b) { 
  byte x = b & signalSetType;
  return x >= maxSignalSets ? 0 : x;
}

extern const byte mastTypeDefinitionCount;

/**
 * Defines a mast prototype. Each mast uses
 * - a signal set
 * - recognizes a defined number of codes
 * - translates a code into an aspect from the signal set
 */
struct MastTypeDefinition {
  // number of different codes. Implies number of addresses used.
  byte codeCount;
  // default light count
  byte lightCount;
  SignalSet signalSet;
  byte defaultCode;

  byte outputs[maxOutputsPerMast];
  byte code2Aspect[maxAspects];
};

enum MastType {
  none = 0,
  incoming5,
  departure4,
  departure3,
  shutning2,
};

struct MastTypeNameId {
  byte id;
  const char* name;
};

/**
 * Structure that corresponds to a CV block and defines one mast.
 */
struct MastSettings {
  /**
   * Assignments of mast lights to output indexes. It is 1-based, 0 means that the light
   * has no output (i.e. green light on a mast that cannot signal full speed).
   */
  byte outputs[maxOutputsPerMast];
  /**
   * If the usesCodes bit is set, the lower 5 bits is the index to the mastTypeDefinitions table (see Definitions).
   * The table defines the signal set, number of codes -> number of addresses, initial light assignment.
   * 
   * If usesCodes is not set, the lower 5 bits is one of the SignalSet constants.
   * 
   * Bits 5-7 define how the mast is controlled, see SignalSetFlags
   */
  byte signalSetOrMastType;

  /**
   * The default code (usesCodes) or aspect (!usesCodes)
   */
  byte defaultCodeOrAspect;

  /**
   * The number of assigned addresses. 0 is used in configuration data to compute the actual number of addresses
   * from the mast type and control style.
   */
  byte addresses;
};
static_assert(SEGMENT_SIZE == sizeof(MastSettings), "MastSettings != SEGMEN_SIZE");

class SignalMastData {
public:
  boolean changed : 1;  // lastCode was set, but not converted to the aspect.
  SignalSet set : 3;
  byte addressCount : 3;
  byte signalCount : 4; // 0..maxOutputsPerMast
  byte currentAspect : 5; // 0..maxAspects
  byte lastCode : 5;  // last code set to the signal
  unsigned int lastTime;  // time the last code was set
  byte defaultAspect;
 
  SignalMastData() : changed(false), set(SIGNAL_SET_CSD_BASIC), addressCount(0), signalCount(0), currentAspect(0), lastCode(0), lastTime(0), defaultAspect(255) {}

  unsigned int maxSignalCount() {
    return 1 << addressCount;
  }

  boolean isReadyForProcess() {
    if (!changed) {
      return false;
    }
    long diff = (currentTime & 0xffff)- lastTime;
    if (diff < 0) {
      diff += 0x10000;
    }
    if (diff < 0) {
      // someting really strange happened, the timer rolled around
      // discard the change.
      processed();
      return false;
    }
    return (unsigned long)diff > aspectLag;
  }

  void setCode(byte c) {
    changed = true;
    lastCode = c;
    lastTime = currentTime & 0xffff;
  }

  byte processed() {
    byte save = lastCode;
    changed = false;
    lastTime = 0;
    lastCode = 0;
    return save;
  }

  byte getLastCode() {
    return lastCode;
  }
};
static_assert(maxOutputsPerMast <= 16,  "Too many signals");
static_assert(maxAspects <= 32, "Too many aspects");

void signalMastChangeAspect(int nrSignalMast, byte newAspect);
void signalMastChangeAspect(int progMemOffset, int tableSize, int nrSignalMast, SignalMastData& data, byte newAspect);

const int NUM_8BIT_SHIFT_REGISTERS = (NUM_OUTPUTS + 7) / 8;

const int maxAspectBits = 5;
static_assert(maxAspects <= (1 << maxAspectBits), "Too many aspects, do not fit in 5 bits");

const byte ONA = 0;  // OUTPUT_NOT_ASSIGNED

const uint8_t INIT_DECODER_ADDRESS = 100;  // ACCESSORY DECODER ADDRESS default

extern NmraDcc Dcc;

void setFactoryDefault();
int findMinLightIndex(int mast);
const struct MastTypeDefinition& copySignalMastTypeDefinition(byte typeId);

inline byte toTemplateIndex(byte b) { 
  byte x = b & (~signalSetFlags);
  return x >= mastTypeDefinitionCount ? 0 : x;
}

extern SignalMastData signalMastData[NUM_SIGNAL_MAST];
extern byte overrides[(NUM_OUTPUTS + 7) / 8];

int findNumberOfSignals(int addresses, int mastType);
void saveTemplateOutputsToCVs(const struct MastTypeDefinition& def, int mastIndex, int from);
void saveTemplateOutputsToCVs(const struct MastTypeDefinition& def, int mastIndex, boolean stable);
void saveTemplateAspectsToCVs(int mast, int signalSetOrMastType);
int findRequiredAddrCount(int codes, int mastType);
int findNumberOfSignals(int addresses, int mastType);
byte aspectJmri(int nrSignalMast, byte aspectMx);
inline byte numberToPhysOutput(byte nrOutput);
byte getMastOutput(byte nMast, byte light);
LightFunction getBulbState(byte n);
SignalMastData& signalData(byte mast);

#endif