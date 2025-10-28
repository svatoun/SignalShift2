
#include <EEPROM.h>
#include "Common.h"

const boolean debugInfra = false;

extern void commandReset();

const int MAX_LINE = 60;
boolean interactive = true;
void (*charModeCallback)(char) = NULL;
const int maxLineCommands = 40;

const int maxInputLine = MAX_LINE;
char inputLine[maxInputLine + 1];
char* inputPos = inputLine;
char* inputEnd = inputLine;

LineCommand lineCommands[maxLineCommands];

void initTerminal() {
  registerLineCommand("RST", &commandReset);
  registerLineCommand("CLR", &commandClear);
}

void clearInputLine() {
  inputLine[0] = 0;
  inputEnd = inputPos = inputLine;
}

void setupTerminal() {
  resetTerminal();
}

boolean inResetTerminal = false;

void resetTerminal() {
  if (inResetTerminal) {
    return;
  }
  inResetTerminal = true;
  clearInputLine();
  if (charModeCallback != NULL) {
    (*charModeCallback)(0);
  }
  charModeCallback = NULL;
  printPrompt();
  inResetTerminal = false;
}


void registerLineCommand(const char* aCmd, void (*aHandler)()) {
  for (int i = 0; i < maxLineCommands; i++) {
    if (lineCommands[i].handler == NULL) {
      lineCommands[i].cmd = aCmd;
      lineCommands[i].handler = aHandler;
      return;
    }
  }
}

void processLineCommand() {
  inputEnd = inputPos;
  char* pos = strchr(inputLine, ':');
  if (pos == NULL) {
    pos = inputEnd;
    inputPos = pos;
  } else {
    *pos = 0;
    inputPos = pos + 1;
  }
  if (debugInfra) {
    Serial.print(F("Command: "));
    Serial.println(inputLine);
  }
  for (int i = 0; i < maxLineCommands; i++) {
    LineCommand& c = lineCommands[i];
    if (c.handler == NULL) {
      break;
    }
    const char* p = c.cmd;
    const char* x = inputLine;
    for (; (*p != 0) && (*x != 0); x++, p++) {
      char e = *x;
      if ((e >= 'a') && (e <= 'z')) {
        e -= ('a' - 'A');
      }
      if (e != (*p)) {
        goto end;
      }
    }
    if (*p != *x) {
      goto end;
    }
    if (debugInfra) {
      Serial.print(F("Trying handler for command "));
      Serial.println(c.cmd);
    }
    if (debugInfra) {
      Serial.print(F("Remainder of command "));
      Serial.println(inputPos);
    }
    c.handler();
    return;

end:;
  }
  Serial.println(F("\nBad command"));
}

void printPrompt() {
  if (!interactive) {
    return;
  }
  if (charModeCallback != NULL) {
    return;
  }
  Serial.print("@ > ");
}


void processTerminal() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (charModeCallback != NULL) {
      if (c == '`') {
        // reset from the character mode
        charModeCallback = NULL;
        resetTerminal();
        continue;
      }
      charModeCallback(c);
      continue;
    }
    if (c == 0x7f || c == '\b') {
      Serial.write(c);
      if (inputPos > inputLine) {
        inputPos--;
        *inputPos = 0;
      }
      continue;
    }
    if (c == '\n' || c == '\r') {
      Serial.write("\r\n");
      processLineCommand();
      clearInputLine();
      printPrompt();
      continue;
    }
    Serial.write(c);
    if ((inputPos - inputLine) >= MAX_LINE) {
      continue;
    }
    *inputPos = tolower(c);
    inputPos++;
    *inputPos = 0;
  }
}

int nextNumber() {
  return nextNumber(false);
}

int inputDelim = 0;

int nextNumber(boolean extDelims) {
  inputDelim = 0;
  if ((*inputPos) == 0) {
    return -2;
  }
  if ((*inputPos) == ':') {
    inputPos++;
    return -3;
  }
  char c = *inputPos;
  if (c < '0' || c > '9') {
    return -1;
  }
  char* p = strchr(inputPos, ':');
  char* p2 = extDelims ? strchr(inputPos, '-') : NULL;
  if (p == NULL) {
    if (p2 != NULL) {
      p = p2;
      inputDelim = *p;
      *p = 0;
    } else {
      p = inputEnd - 1;
    }
  } else {
    if (p2 != NULL && p2 < p) {
      p = p2;
      inputDelim = *p;
    }
    *p = 0;
  }
  char* ne;
  int val = atoi(inputPos);
  inputPos = p + 1;
  return val;
}

void commandInteractive() {
  char c = *inputPos;
  switch (c) {
    case 'y':
      interactive = true;
      break;
    default:
      interactive = false;
      break;
  }
  if (interactive) {
    //commandStatus();
  }
}

void commandReset() {
  void (*func)() = 0;
  func();
}
