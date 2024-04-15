/*
      FabiWare - AsTeRICS Foundation
     For more info please visit: https://www.asterics-foundation.org

     Module: buttons.cpp - implementation of the button handling

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; See the GNU General Public License:
   http://www.gnu.org/licenses/gpl-3.0.en.html

*/

#include "FlipWare.h"  //  FABI command definitions
#include "infrared.h"
#include "keys.h"


struct slotButtonSettings buttons[NUMBER_OF_BUTTONS];            // array for all buttons - type definition see FlipWare.h
char* buttonKeystrings[NUMBER_OF_BUTTONS];                       // pointers to keystring parameters
char keystringBuffer[MAX_KEYSTRINGBUFFER_LEN] = { 0 };           // storage for keystring parameters for all buttons
struct buttonDebouncerType buttonDebouncers[NUMBER_OF_BUTTONS];  // array for all buttonsDebouncers - type definition see fabi.h
uint32_t buttonStates = 0;                                       // current button states for reporting raw values (AT SR)
bool isLongPress;

void initButtonKeystrings() {
  slotSettings.keystringBufferLen = 0;

  for (int i = 0; i < NUMBER_OF_BUTTONS; i++) {
    buttonKeystrings[i] = keystringBuffer + slotSettings.keystringBufferLen;
    while (keystringBuffer[slotSettings.keystringBufferLen++])
      ;
  }

#ifdef DEBUG_OUTPUT_FULL
  Serial.print("Init ButtonKeystrings, bufferlen =");
  Serial.println(slotSettings.keystringBufferLen);
#endif
}

char* getButtonKeystring(int num) {
  char* str = keystringBuffer;
  for (int i = 0; i < num; i++) {
    if (*str)
      while (*str++)
        ;
    else str++;
  }
  return (str);
}


void printKeystrings() {
  char* x = keystringBuffer;
  for (int i = 0; i < NUMBER_OF_BUTTONS; i++) {
    if (*x) {
      Serial.print("Keystring ");
      Serial.print(i);
      Serial.print(" = ");
      Serial.println(x);
      while (*x++)
        ;
    } else x++;
  }
}
uint16_t setButtonKeystring(uint8_t buttonIndex, char const* newKeystring) {
  char* keystringAddress = getButtonKeystring(buttonIndex);

  int oldKeyStringLen = strlen(keystringAddress);
  char* sourceAddress = keystringAddress + oldKeyStringLen + 1;

  if (slotSettings.keystringBufferLen - oldKeyStringLen + strlen(newKeystring) >= MAX_KEYSTRINGBUFFER_LEN - 1)
    return (0);  // new keystring does not fit into buffer !

  uint16_t bytesToMove = keystringBuffer + slotSettings.keystringBufferLen - sourceAddress;
  int delta = strlen(newKeystring) - oldKeyStringLen;  // if positive: expand keystringBuffer!
  char* targetAddress = sourceAddress + delta;
  if (delta) {
    memmove(targetAddress, sourceAddress, bytesToMove);
  }

  strcpy(keystringAddress, newKeystring);  // store the new keystring!

  //update ALL keystring pointers, because we might have moved some of them
  char* x = keystringBuffer;
  for (int i = 0; i < NUMBER_OF_BUTTONS; i++) {
    if (*x) {
      buttonKeystrings[i] = x;
      while (*x++)
        ;
    } else x++;
  }

  slotSettings.keystringBufferLen += delta;  // update buffer length

#ifdef DEBUG_OUTPUT_FULL
  printKeystrings();
  Serial.print("bytes left:");
  Serial.println(MAX_KEYSTRINGBUFFER_LEN - slotSettings.keystringBufferLen);
#endif
  return (MAX_KEYSTRINGBUFFER_LEN - slotSettings.keystringBufferLen);
}


void initButtons() {  // The default values for the buttons. Can be seen in action configuration (WebGUI).

  initButtonKeystrings();

  // set default functions
  for (int i = 0; i < NUMBER_OF_BUTTONS; i++) {
    buttons[i].value = 0;
    buttons[i].mode = CMD_NC;  // no command
  }

  buttons[0].mode = CMD_KP;  // KP, Key press.
  setButtonKeystring(0, "KEY_SPACE ");
  buttons[1].mode = CMD_KP;
  setButtonKeystring(1, "KEY_ENTER ");
  buttons[2].mode = CMD_CL;  // CL, Left mouse button clicked.
  buttons[3].mode = CMD_KP;
  setButtonKeystring(3, "KEY_LEFT ");
  buttons[4].mode = CMD_KP;
  setButtonKeystring(4, "KEY_RIGHT ");
}


void handlePress(int buttonIndex)  // a button was pressed
{
  buttonStates |= (1 << buttonIndex);  //save for reporting
  performCommand(buttons[buttonIndex].mode, buttons[buttonIndex].value, buttonKeystrings[buttonIndex], 1);
}

void handleRelease(int buttonIndex)  // a button was released: deal with "sticky"-functions
{
  buttonStates &= ~(1 << buttonIndex);  //save for reporting
  switch (buttons[buttonIndex].mode) {
    case CMD_PL:
    case CMD_HL:
      mouseRelease(MOUSE_LEFT);
      break;
    case CMD_PR:
    case CMD_HR:
      mouseRelease(MOUSE_RIGHT);
      break;
    case CMD_PM:
    case CMD_HM:
      mouseRelease(MOUSE_MIDDLE);
      break;
    case CMD_JP: joystickButton(buttons[buttonIndex].value, 0); break;
    case CMD_KH: releaseKeys(buttonKeystrings[buttonIndex]); break;
    case CMD_IH:
      stop_IR_command();
      break;
  }
}


uint8_t handleButton(int i, uint8_t state)  // button debouncing and press detection
{
  if (buttonDebouncers[i].bounceState == state) {                     // Checks whether the current state of the button is the same as the previous state.
    if (buttonDebouncers[i].bounceCount < DEFAULT_DEBOUNCING_TIME) {  // Makes sure that the debouncer issue does not surpass the default time of 5ms.
      buttonDebouncers[i].bounceCount++;
      if (buttonDebouncers[i].bounceCount == DEFAULT_DEBOUNCING_TIME) {

        if (state != buttonDebouncers[i].stableState) {  // entering stable state // Checks whether the button state has changed.
          buttonDebouncers[i].stableState = state;

          if (state == 1) {  // new stable state: pressed ! // Checking whether a button has been pressed.
            //  if (inHoldMode(i))

            buttonStates |= (1 << i);  //save for reporting // Reporting, can be seen in visualisation of the WebGUI.
            handlePress(i);
            buttonDebouncers[i].timestamp = millis();  // start measuring time

          } else {  // new stable state: released !
            // if (!inHoldMode(i))
            // handlePress(i);
            buttonStates &= ~(1 << i);  //save for reporting

            if (inHoldMode(i))
              handleRelease(i);
            return (1);  // indicate that button action has been performed !
          }
        }
      }

    } else {  // in stable state
    }

  } else {
    buttonDebouncers[i].bounceState = state;
    buttonDebouncers[i].bounceCount = 0;
  }
  return (0);
}


uint8_t inHoldMode(int i) {  // Not necessarily on purpose.
  if ((buttons[i].mode == CMD_PL) || (buttons[i].mode == CMD_PR) || (buttons[i].mode == CMD_PM) || (buttons[i].mode == CMD_HL) || (buttons[i].mode == CMD_HR) || (buttons[i].mode == CMD_HM) || (buttons[i].mode == CMD_JP) || (buttons[i].mode == CMD_MX) || (buttons[i].mode == CMD_MY) || (buttons[i].mode == CMD_KH) || (buttons[i].mode == CMD_IH))
    return (1);
  else return (0);
}

void initDebouncers() {
  for (int i = 0; i < NUMBER_OF_BUTTONS; i++)  // initialize button array
  {
    buttonDebouncers[i].bounceState = 0;
    buttonDebouncers[i].stableState = 0;
    buttonDebouncers[i].bounceCount = 0;
    buttonDebouncers[i].longPressed = 0;
  }
}
