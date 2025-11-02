#include <NmraDcc.h>
#include <ShiftPWM.h>

#include "Common.h"
#include "SignalShift.h"
#include "Terminal.h"
#include "DecoderEnv.h"
#include "messages.h"

boolean handleSignals(ModuleCmd cmd);
ModuleChain signals("signals", 0, &handleSignals);

extern void commandReset();
extern byte mastTypeNameCount; 
extern const MastTypeNameId mastTypeNames[];

int definedMast = -1;

void commandClear() {
  setFactoryDefault();
  commandSave();
  commandReset();
}

void commandPrintMastDef() {
  int nMast = nextNumber();
  if (nMast < 1 || nMast > NUM_SIGNAL_MAST) {
    Console.println(msg_InvalidMastID);
    return;
  }
  printMastDef(nMast - 1);
}

void commandDump() {
  Console.print("ADR:"); Console.println(Dcc.getAddr());
  dumpAllMasts();
}

void dumpAllMasts() {
  for (int m = 0; m < NUM_SIGNAL_MAST; m++) {
    if (!isMastActive(m)) {
      continue;
    }
    printMastDef(m);
  }
}

boolean isMastActive(int nMast) {
  int cvBase = sizeof(MastSettings) * nMast + START_CV_OUTPUT;
  for (int i = 0; i < maxOutputsPerMast; i++) {
    int o = Dcc.getCV(cvBase + i); 
    if ((o != ONA) && (o < NUM_OUTPUTS + 1)) {
      return true;
    }
  }
  return false;
}

boolean shouldPrintMastOutputs(int nMast) {
  int cvBase = sizeof(MastSettings) * nMast + START_CV_OUTPUT;
  int control = Dcc.getCV(cvBase + maxOutputsPerMast);
  if (!(control & usesCodes)) {
    return true;
  }
  int minLight = findMinLightIndex(nMast);
  
  const struct MastTypeDefinition& def = copySignalMastTypeDefinition(toTemplateIndex(control));
  for (int i = 0; i < maxOutputsPerMast; i++) {
    int out = Dcc.getCV(cvBase + i);
    if (out > 0) {
      out = out - minLight + 1;
    }
    if (out != def.outputs[i]) {
      return true;
    }
  }
  return false;
}

boolean shouldPrintAspects(int nMast) {
  int cvBase = sizeof(MastSettings) * nMast + START_CV_OUTPUT;
  int control = Dcc.getCV(cvBase + maxOutputsPerMast);
  if (!(control & usesCodes)) {
    return true;
  }
  
  int addresses = Dcc.getCV(cvBase + maxOutputsPerMast + 2);
  int limit = findNumberOfSignals(addresses, control);

  int aspectBase = START_CV_ASPECT_TABLE + nMast * maxAspects;

  const struct MastTypeDefinition& def = copySignalMastTypeDefinition(toTemplateIndex(control));
  for (int i = 0; i < limit; i++) {
    int out = Dcc.getCV(aspectBase + i);
    if (out != def.code2Aspect[i]) {
      return true;
    }
  }
  return false;
}


void printMastDef(int nMast) {
  int first = 0xff;
  int cnt = 0;
  
  int cvBase = sizeof(MastSettings) * nMast + START_CV_OUTPUT;
  for (int i = 0; i < maxOutputsPerMast; i++) {
    int o = Dcc.getCV(cvBase + i); 
    if ((o != ONA) && (o < NUM_OUTPUTS + 1)) {
      // Console.print("output "); Console.print(i); Console.print(" = "); Console.println(o);
      if (o < first) {
        first = o;
      }
      cnt++;
    }
  }
  if (first == ONA || first > NUM_OUTPUTS) {
    Console.print(F("DEL:")); Console.println(nMast + 1);
    return;
  }
  Console.print(F("DEF:")); Console.print(nMast + 1); Console.print(':'); 
  Console.print(first); Console.print(':');   

  int mode = Dcc.getCV(cvBase + 10);
  switch (mode & signalSetControlType) {
    case bitwiseControl:
      Console.print('b');
      break;
    case turnoutNoDirection:
      Console.print('t');
      break;
    case extendedPacket:
      Console.print('e');
      break;
    case turnoutControl:
      Console.print('c');
      break;
    default:
      Console.print("ERR/"); Console.print(mode, HEX); Console.print("/"); Console.print(mode & signalSetControlType, HEX);
      break;  
  }
  Console.print(':');
  boolean codes = false;
  int set = 1;
  int defcode = 0;
  if (usesCodes & mode) {
    int type = mode & signalSetType;
    codes = true;
    Console.print('+'); 
    
    char buffer[10];      
    boolean printed = false;
    for (int i = 0; i < mastTypeNameCount; i++) {
      int idx = (int)&(mastTypeNames[i]);
      int n = pgm_read_byte_near(idx); 
      int charIdx = pgm_read_word_near(idx + 1);
      if (n == 0) {
        if (charIdx == 0) {
          break;  
        } else {
          continue;
        }
      }
      n--;
      if (n == type) {
        strcpy_P(buffer, (char*)charIdx); 
        if (*buffer != 0) {
          Console.println(buffer);
          printed = true;
        }
        break;
      }
    }
    if (!printed) {
       Console.println(type);
    }
    const struct MastTypeDefinition& def = copySignalMastTypeDefinition(toTemplateIndex(type));
    set = def.signalSet;
    defcode = def.defaultCode;
    
  }
  if (!codes) {
    Console.print(cnt); Console.print(':'); 
    Console.println(signalMastData[nMast].signalCount);
  }
  byte mastSet = signalMastData[nMast].set;
  byte mastDefault = signalMastData[nMast].defaultAspect;
  if (mastSet != set || mastDefault != defcode) {
    Console.print(F("  MSS:")); Console.print(mastSet); Console.print(':'); Console.println(mastDefault);
  }
  if (shouldPrintMastOutputs(nMast)) {
    printMastOutputs(nMast, false);
  }
  if (shouldPrintAspects(nMast)) {
    printAspectMap(nMast);
  }
  Console.println(F("END"));
}

void printMastOutputs(int nMast, boolean suppressFull) {
  byte lights[maxOutputsPerMast];
  int cnt = 0;

  int cvBase = sizeof(MastSettings) * nMast + START_CV_OUTPUT;
  for (int i = 0; i < maxOutputsPerMast; i++) {
    lights[i] = Dcc.getCV(cvBase + i); 
    if (lights[i] != ONA && lights[i] <= NUM_OUTPUTS) {
      cnt++;
    }
  }
  int skipCnt = 0;
  int cnt2 =  0;
  boolean outPrinted = false;
  for (unsigned int x = 0; x < maxOutputsPerMast; x++) {
    int a = lights[x];
    if (a == ONA || a > NUM_OUTPUTS) {
      if (skipCnt++ == 0) {
        if (!outPrinted) {
          outPrinted = true;
          Console.print(F("OUT:"));
        }
        Console.print('-');
      }
      continue;
    }
    if (skipCnt > 0) {
      if (skipCnt == 2) {
        Console.print('-');
      } else if (skipCnt > 2) {
        Console.print(skipCnt - 1);
      }
      Console.print(':');
    }
    skipCnt = 0;
    unsigned int seq = x + 1;
    while ((seq < sizeof(lights)) && (lights[seq] == (1 + lights[seq - 1]))) {
      seq++;
    }
    int len = seq - x;
    if (x == 0 && len == cnt && suppressFull) {
      return;
    }
    if (!outPrinted) {
      outPrinted = true;
      Console.print(F("OUT:"));
    }
    if (len > 2) {
      Console.print(a); Console.print('-'); Console.print(a + len - 1);
      x = seq - 1; // will increment to seq in loop reinit
    } else if (len == 2) {
      Console.print(a); Console.print(':'); Console.print(a + 1);
      x++;
    } else {
      Console.print(a); 
    }
    cnt2 += len;
    if (cnt2 == cnt) {
      break;
    }
    Console.print(':');
  }
  Console.println();
}

int findSameAspect(int nMast) {
  byte aspectMap[maxAspects];

  boolean identity = true;
  int cv = START_CV_ASPECT_TABLE + (nMast * maxAspects);
  for (int x = 0; x < maxAspects; x++, cv++) {
    aspectMap[x] = Dcc.getCV(cv);
    if (aspectMap[x] != x) {
      identity = false;
    }
  }
  if (identity) {
    return NUM_SIGNAL_MAST + 1;
  }
  for (int i = 0; i < nMast; i++) {
    if (signalMastData[i].addressCount != signalMastData[nMast].addressCount) {
      continue;
    }
    cv = START_CV_ASPECT_TABLE + (i * maxAspects);
    boolean found = true;
    for (int x = 0; x < maxAspects; x++, cv++) {
      if (Dcc.getCV(cv) != aspectMap[x]) {
        found = false;
        break;
      }
    }
    if (found) {
      return i;
    }
  }
  return -1;
}

void printAspectMap(int nMast) {
  int cvBase = sizeof(MastSettings) * nMast + START_CV_OUTPUT;
  byte mastTypeOrSignalSet = Dcc.getCV(cvBase + maxOutputsPerMast);
  int addresses = Dcc.getCV(cvBase + maxOutputsPerMast + 2);
  int limit = findNumberOfSignals(addresses, mastTypeOrSignalSet);
  // Console.print("type = "); Console.println(mastTypeOrSignalSet);
  // Console.print("Limit = "); Console.println(limit);
  int copyOf = findSameAspect(nMast);
  if (copyOf >= 0) {
    if (copyOf >= NUM_SIGNAL_MAST) {
      return;
    }
    Console.print(F("  MCP:")); Console.println(copyOf + 1);
  } else {
    byte aspectMap[maxAspects];
    int cnt = 0;
    int cv = START_CV_ASPECT_TABLE + (nMast * maxAspects);
    for (int x = 0; x < limit; x++, cv++) {
      aspectMap[x] = Dcc.getCV(cv);
      if (aspectMap[x] <= maxAspects && aspectMap[x] > 0) {
        cnt++;
      }
    }
    int skipCnt = 0;
    int l = 0;
    Console.print(F("  MAP:"));
    for (int x = 0; x < limit; x++) {
      int a = aspectMap[x];
      int seq = x + 1;
      l = x;
      if (a == 0 || a > maxAspects) {
        skipCnt++;
        continue;
      }
      if (skipCnt > 0) {
        if (x > 0) {
          Console.print(':');
        }
        Console.print('-');
        if (skipCnt == 2) {
          Console.print('-');
        } else if (skipCnt > 2) {
          Console.print(skipCnt);
        }
      }
      if (x > 0) {
        Console.print(':');
      }
      while ((seq < limit) && (aspectMap[seq] == (1 + aspectMap[seq - 1]))) {
        seq++;
      }
      int len = seq - x;
      if (len > 2) {
        Console.print(a); Console.print('-'); Console.print(a + len - 1);
        x = seq - 1; // will increment to seq in loop reinit
      } else if (len == 2) {
        Console.print(a); Console.print(':'); Console.print(a + 1);
        x = seq - 1; // will increment to seq in loop reinit
      } else {
        Console.print(a);
      }
    }
    if (skipCnt > 0) {
      if (l > 0) {
        Console.print(':');
      }
      Console.print('-');
      if (skipCnt == 2) {
        Console.print('-');
      } else if (skipCnt > 2) {
        Console.print(skipCnt);
      }
    }
  }
  Console.println();
}

void commandEnd() {
  if (definedMast < 0) {
    Console.println(msg_MastNotNotOpened);
    return;
  } else {
    Console.print(F("Mast #")); Console.print(definedMast); Console.println(F(" closed."));
    definedMast = -1;
  }
}

/*
  Syntax: DEF:mast:first-out:mode:lights:codes
          DEF:mast:first-out:mode:+name
*/
void commandDefineMast() {
  int nMast = nextNumber();
  if (nMast < 1 || nMast > NUM_SIGNAL_MAST) {
    Console.println(msg_InvalidMastID);
    return;
  }

  definedMast = nMast;
  int firstOut = nextNumber();
  if (firstOut == -2) {
    Console.print(F("Mast #")); Console.print(nMast); Console.println(F(" definition open."));
    return;
  }
  if (firstOut < 1 || firstOut > NUM_OUTPUTS) {
    Console.println(msg_InvalidOutput);
    return;
  }

  int mastType = -1;

  int mode = -1;
  switch (*inputPos) {
    case 'b': case 'B':
      // bitwise
      mode = bitwiseControl;
      break;
    case 't': case 'T':
      // turnout
      mode = turnoutNoDirection;
      break;
    case 'e': case 'E':
      // extended
      mode = extendedPacket;
      break;
    case 'c': case 'C':
      // turnout control
      mode = turnoutControl;
      break;
  }
  if (mode != -1) {
    inputPos++;
    if (*inputPos == ':') {
      inputPos++;
    }
  } else {
    mode = bitwiseControl;
  }

  int numSignals = 0;
  int numLights = 0;
  int signalSetOrMastType = mode | SIGNAL_SET_CSD_BASIC;
  int defSignal = 0;
  int bits = 0;
  int mastIdx = nMast - 1;
  int cvBase = START_CV_OUTPUT + (mastIdx) * SEGMENT_SIZE;
  
  if (*inputPos == '+') {
    const char *s = ++inputPos;
    char *e = strchr(inputPos, ':');
    char *next;
    if (e != NULL) {
      *e = 0;
      next = e + 1;
    } else {
      e = inputPos + strlen(inputPos);
      next = e;
    }
    // mast type from template
    // Console.print("Trying to match: "); Console.println(s);
    char buffer[10];
    for (int i = 0; i < 32; i++) {
      int idx = (int)&(mastTypeNames[i]);
      int id = pgm_read_byte_near(idx);
      int charIdx = pgm_read_word_near(idx + 1);
      if (id == 0) {
        if (charIdx == 0) {
          break;
        } else {
          continue;
        }
      }
      id--;
      
      strcpy_P(buffer, (char*)charIdx); 

      if (strcmp(buffer, s) == 0) {
        mastType = usesCodes | mode | id;
        // Console.print("Found id: "); Console.print(id); Console.print(" mode "); Console.println(mode);
        break;
      }
    }
    if (mastType < 0) {
      mastType = nextNumber();
    }
    if (mastType < 0) {
      Console.println("Bad mast type");
      return;
    }
    inputPos = next;
  }

  if (mastType < 0) {
    numLights = nextNumber();
    if (numLights < 1 || (firstOut + numLights > NUM_OUTPUTS) || numLights > maxOutputsPerMast) {
      Console.println(F("Bad # of lights"));
      return;
    }

    numSignals = nextNumber();
    if (numSignals == -2) {
      numSignals = 1 << numLights;
    }
    
    if (numSignals < 1 || numSignals > 32) {
      Console.println(F("Bad # of signals"));
      return;
    }

    for (int i = 0; i < maxOutputsPerMast; i++) {
      if (i >= numLights) {
        Dcc.setCV(cvBase + i, ONA);
      } else {
        Dcc.setCV(cvBase + i, firstOut + i - 1);
      }
    }
    // the signal set number
    Dcc.setCV(cvBase + 10, signalSetOrMastType);
    // the default signal
    Dcc.setCV(cvBase + 11, 0);
    signalMastData[mastIdx].set = SIGNAL_SET_CSD_BASIC;
  } else {
    const struct MastTypeDefinition& def = copySignalMastTypeDefinition(toTemplateIndex(mastType));
    numSignals = def.codeCount;
    // numLights = def.lightCount;
    defSignal = def.defaultCode;
    // Console.print("Copying from template ");
    // Console.println(toTemplateIndex(mastType));
    // Console.print("signals: "); Console.print(numSignals); Console.print(" lights: "); Console.print(numLights); Console.print(" default: "); Console.println(defSignal);
    saveTemplateOutputsToCVs(def, mastIdx, firstOut);
    saveTemplateAspectsToCVs(mastIdx, mastType);
      // the signal set number
    Dcc.setCV(cvBase + 10, mastType & 0xff);
    signalMastData[mastIdx].set = def.signalSet;
  }
  
  bits = findRequiredAddrCount(numSignals, mode);
  // number of addresses = bits
  // Console.print("Wrote to CV #"); Console.println(cvBase + 12, HEX);
  Dcc.setCV(cvBase + 12, bits);

  signalMastData[mastIdx].signalCount = numSignals;
  signalMastData[mastIdx].defaultAspect = defSignal;
  signalMastData[mastIdx].addressCount = bits;
  // Console.print(F("Mast #")); Console.print(nMast); Console.print(F(" uses outputs ")); Console.print(firstOut); Console.print(F(" - ")); Console.print(firstOut + numLights);
  // Console.print(F(" and ")); Console.print(bits); Console.println(F(" addresses."));

  Console.println(F("Mast open, END to finish."));
}

void commandSetSignal() {
  int nMast = nextNumber();
  if (nMast < 1 || nMast > NUM_SIGNAL_MAST) {
    Console.println(msg_InvalidMastID);
    return;
  }
  int mastID = nMast - 1;
  int aspect = nextNumber();
  int maxAspect = signalMastData[mastID].maxSignalCount();
  byte newAspect;

  if (aspect < 1 || aspect > maxAspect) {
    Console.println(msg_InvalidAspect);
    newAspect = 255;
  } else {
    newAspect = aspectJmri(mastID, aspect - 1) ;
  }
  signalMastChangeAspect(mastID, newAspect);
  // Console.print(F("Mast #")); Console.print(nMast); Console.print(F(" set to aspect index #")); Console.print(aspect); Console.print('='); Console.println(newAspect);
}

void commandGetCV() {
  int cv = nextNumber();
  if (cv < 1 || cv > 1023) {
    Console.println(F("Invalid CV #"));
    return;
  }
  int lastCV = nextNumber();
  if (lastCV < 0) {
    int val = Dcc.getCV(cv);
    Console.print(F(" CV #")); Console.print(cv); Console.print(F(" = ")); Console.println(val);
    return;
  }
  while (cv <= lastCV) {
    if (cv < 0x10) {
      Console.print("00");
    } else if (cv < 0x100) {
      Console.print("0");
    }
    Console.print(cv, HEX);
    Console.print(": ");
    for (int a = 0; a < 16; a++) {
      if (cv > lastCV) {
        break;
      }
      int val = Dcc.getCV(cv);
      cv++;
      if (val < 0x0f) {
        Console.print("0");
      }
      Console.print(val, HEX);
      Console.print(" ");
    }
    Console.println();
  }
}

void commandSetCV() {
  int cv = nextNumber();
  if (cv < 1 || cv > 512) {
    Console.println(F("Bad CV #"));
    return;
  }
  int val = nextNumber();
  if (val < 0 || val > 255) {
    Console.println(F("Bad CV value"));
    return;
  }
  int oldV = Dcc.getCV(cv);
  int x = nextNumber();
  
  Dcc.setCV(cv, val);
  if (x > 0) {
    // Console.println(F("Notify CV change"));
    notifyDccCVChange(cv, val);
  }
  // Console.print(F("Changed CV #")); Console.print(cv); Console.print(F(": ")); Console.print(oldV); Console.print(F(" => ")); Console.println(val);
}

void commandMastSet() {
  if (definedMast < 0) {
    Console.println(msg_MastNotNotOpened);
    return;
  }
  int nMast = definedMast;
  int signalSet = nextNumber();
  if (signalSet < 0 || signalSet >= _signal_set_last) {
    Console.println(F("Bad signal set"));
    return;
  }
  nMast--;
  int defaultAspect = nextNumber();
  int cv = START_CV_OUTPUT + nMast * SEGMENT_SIZE;
  int bits = Dcc.getCV(cv + 12);
  if (defaultAspect < 0 || defaultAspect > maxAspects || defaultAspect > (1 << bits)) {
    Console.println(msg_InvalidAspect);
    return;
  }
  defaultAspect--;
  Dcc.setCV(cv + 10, signalSet);
  Dcc.setCV(cv + 11, defaultAspect);
  signalMastData[nMast].set = (SignalSet)signalSet;
  signalMastData[nMast].defaultAspect = defaultAspect;
}

extern int inputDelim;

void commandMapOutput() {
  if (definedMast < 0) {
    Console.println(msg_MastNotNotOpened);
    return;
  }
  int nMast = definedMast - 1;
  int outCV = START_CV_OUTPUT + (nMast) * SEGMENT_SIZE;
  int cnt = 0;
  while (*inputPos) {
    while (*inputPos == '-') {
      inputPos++;
      if (*inputPos == ':' || *inputPos == '-') {
        Dcc.setCV(outCV, ONA);
        outCV++;
        cnt++;
        continue;
      } else if (!*inputPos) {
        Dcc.setCV(outCV, ONA);
        outCV++;
        cnt++;
        break;
      } else /* if (*inputPos != '-') */ {
        int n = nextNumber();
        if (n < 0 || n + cnt > maxOutputsPerMast) {
          Console.println(F("Too much skipped"));
          return;
        }
        for (int x = 0; x < n; x++) {
          Dcc.setCV(x + outCV, ONA);
        }
        cnt += n;
        outCV+= n;
      }
    }
    if (!*inputPos) {
      break;
    }
    int out = nextNumber(true);
    if (out < 1 || out > NUM_OUTPUTS) {
      Console.println(msg_InvalidOutput);
      return;
    }
    if (cnt >= maxOutputsPerMast) {
      Console.println(F("Many outputs"));
      return;
    }
    
    if (inputDelim == '-') {
      int to = nextNumber(false);

      if (to <= out || to > NUM_OUTPUTS) {
        Console.println(msg_InvalidOutput);
        return;
      }
      if (to - out > maxOutputsPerMast) {
        Console.println(F("Many outputs"));
        return;
      }
      // Console.print("out = "); Console.print(out); Console.print(" to "); Console.println(to);
      for (int i = out; i <= to; i++) {
        // Console.print("Add output "); Console.println(out);
        Dcc.setCV(outCV, out);
        cnt++;
        out++;
        outCV++;
      }
    } else {
      // Console.print("Add output "); Console.println(out);
      Dcc.setCV(outCV, out);
      cnt++;
      outCV++;
    }
  }
  while (cnt < maxOutputsPerMast) {
      Dcc.setCV(outCV++, ONA);
      cnt++;
  }
}

void commandMapAspects() {
  if (definedMast < 0) {
    Console.println(msg_MastNotNotOpened);
    return;
  }
  int nMast = definedMast - 1;
  int cur = 0;
  int limit = signalMastData[nMast].maxSignalCount();
  int cvBase = START_CV_ASPECT_TABLE + nMast * maxAspects;
  while (*inputPos) {
    if (*inputPos == ':') {
      inputPos++;
      continue;
    }
    if (cur >= limit) {
      Console.println(F("Too many aspects"));
      return;
    }
    if (*inputPos == '-') {
      inputPos++;
      if (*inputPos == ':' || *inputPos == '-') {
        Dcc.setCV(cvBase + cur, 255);
        cur++;
        continue;
      } else if (!*inputPos) {
        Dcc.setCV(cvBase + cur, 255);
        cur++;
        break;
      } else /* if (*inputPos != '-') */ {
        int n = nextNumber();
        if (n < 1 || cur + n > limit) {
          Console.print(F("Bad len"));
          return;
        }
        for (int x = 0; x < n; x++) {
          Dcc.setCV(x + cur, ONA);
        }
        cur += n;
      }

      if (*inputPos == '-' || *inputPos == ':' || *inputPos == 0) {
        do {
          Dcc.setCV(cvBase + cur, 255);
        } while (*(inputPos++) == '-');
        continue;
      }
    }
    if (!*inputPos) {
      break;
    }
    if (*inputPos == '=') {
      inputPos++;
      cur = nextNumber();
      if (cur < 0) {
        Console.print(F("Bad index"));
        return;
      }
      continue;
    }

    int aspect = nextNumber(true);
    if (aspect < 0) {
      break;
    }
    if (aspect >= maxAspects) {
      Console.print(msg_InvalidAspect);
    }
    if (inputDelim == '-') {
      int aspectTo = nextNumber();
      if (aspectTo < aspect || aspectTo >= maxAspects) {
        Console.println(F("Bad range"));
        return;
      }
      for (int x = aspect; x <= aspectTo; x++) {
        Dcc.setCV(cvBase + cur, x);
        cur++;
      }
    } else {
      Dcc.setCV(cvBase + cur, aspect);
      cur++;
    }
  }
}

void commandOverride() {
  int n = nextNumber(true);
  if (n < 1 || n > NUM_OUTPUTS) {
    Console.println(msg_InvalidOutput);
    return;
  }
  int to = n;
  if (inputDelim == '-') {
    to = nextNumber();
    if (to < n || to > NUM_OUTPUTS) {
      Console.println(F("Bad range"));
      return;
    }
  }
  int level = nextNumber();
  if (level > 255) {
      Console.println(F("Bad pwm"));
      return;
  }
  for (int i = n - 1; i < to; i++) {
    if (level <= 0) {
      Console.print("Disable output: "); Console.println(i);
      bitWrite(overrides[i / 8], i & 0x07, false);
      ShiftPWM.SetOne(numberToPhysOutput(i), 0);
    } else {
      Console.print("Set output: "); Console.print(i); Console.print(" to "); Console.println(level);
      bitWrite(overrides[i / 8], i & 0x07, true);
      ShiftPWM.SetOne(numberToPhysOutput(i), level);
    }
  }
}

void commandInf() {

}

void commandSave() {
  
}

void commandAddress() {
  int n = nextNumber();
  if (n < 1 || n > 2048) {
    Console.println("Bad DCC address");
    return;
  }
  if (n < 128) {
    Dcc.setCV(CV_ACCESSORY_DECODER_ADDRESS_MSB, 0);
    Dcc.setCV(CV_ACCESSORY_DECODER_ADDRESS_LSB, n);
    Dcc.setCV(1, n);
    Dcc.setCV(29, Dcc.getCV(29) & (~32));
  } else {
    Dcc.setCV(CV_ACCESSORY_DECODER_ADDRESS_MSB, n >> 8);
    Dcc.setCV(CV_ACCESSORY_DECODER_ADDRESS_LSB, n % 255);
    Dcc.setCV(1, n);
    Dcc.setCV(29, Dcc.getCV(29) | 32);
  }
}

void commandDccTurnout() {
  int tnt = nextNumber();
  if (tnt < 1) {
    Console.print("Bad turnout");
    return;
  }
  int d = 0;
  switch (*inputPos) {
    case '-': case '=': case '0': case 'r': case 'R':
      d = 1;
      break;
    case '+': case '/': case '1': case 'o': case 'O':
      d = 0;
      break;
    default:
      Console.println(F("Bad direction"));
      return;
  }
  Console.print(F("Set turnout ")); Console.print(tnt); Console.print(' '); Console.println(d ? F("Straight") : F("Diverging"));
  notifyDccAccTurnoutOutput(tnt, d, 0);
}

boolean handleSignals(ModuleCmd cmd) {
  switch (cmd) {
    case initialize:
      registerLineCommand("CLR", &commandClear);
      registerLineCommand("SGN", &commandSetSignal);
      registerLineCommand("SET", &commandSetCV);
      registerLineCommand("GET", &commandGetCV);
      registerLineCommand("DEF", &commandDefineMast);
      registerLineCommand("INF", &commandPrintMastDef);
      registerLineCommand("DMP", &commandDump);
      registerLineCommand("MSS", &commandMastSet);
      registerLineCommand("OUT", &commandMapOutput);
      registerLineCommand("END", &commandEnd);
      registerLineCommand("MAP", &commandMapAspects);
      registerLineCommand("OVR", &commandOverride);
      registerLineCommand("ADR", &commandAddress);
      registerLineCommand("TNT", &commandDccTurnout);
      break;
    default:
      break;
  }
  return true;
}
