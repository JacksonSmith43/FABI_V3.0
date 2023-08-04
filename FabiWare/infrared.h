/*
     FabiWare - AsTeRICS Foundation
     For more info please visit: https://www.asterics-foundation.org

     Module: infrared.h - implementation of the infrared record/replay functions 

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; See the GNU General Public License:
   http://www.gnu.org/licenses/gpl-3.0.en.html

*/

#ifndef _INFRARED_H_
#define _INFRARED_H_


/**
   physical pin definitions
*/
//RP2040:
#define IR_LED_PIN     14    //  IR-Led output pin      // NOTE: changed for RP2040 ? 28 and 26 are ADC pins ...
#define IR_SENSOR_PIN  16    //  input pin of the TSOP IR receiver

// Time until the record command will be canceled (in milliseconds)
#define IR_USER_TIMEOUT_MS 10000

// Longest valid IR edge-to-edgetime (in microseconds)
#define IR_EDGE_TIMEOUT_US  15000UL

// ir code repeat gap (in microseconds)
#define IR_REPEAT_GAP 10000

// Maximum count of uint16_t edges for one IR-command
#define IR_EDGE_REC_MAX 250

// minimum count of signal edges which are necessary to accept a command
#define IR_EDGE_REC_MIN 5

// name of the idle code command (played after other ir commands if it exists)
#define IDLESEQUENCE_NAME "idle"
#define IDLESEQUENCE_REPEAT 0

// maximum time interval which can be stored in high precision (microseconds) format
#define MAX_HIGHPRECISION_DURATION 64000


/**
   @name initIR
   @brief initializes IR gpio pins and carrier frequency
   @return none
*/
void initIR();

/**
   @name record_IR_command
   @brief record an IR command with given name and store edge timing to EEPROM
          prints error messages if timeout or data structure full
   @param name name of the IR command to be recorded
   @return none
*/
void record_IR_command(char * name);

/**
   @name play_IR_command
   @brief play an IR command with given name from EEPROM 
          code will be played once
   @param name name of the IR command to be played
   @return none
*/
void play_IR_command(char * name);

/**
   @name hold_IR_command
   @brief play an IR command with given name from EEPROM 
          code will be played repeatedly (until repeatCounter==0)
   @param name name of the IR command to be played
   @return none
*/
void hold_IR_command(char * name);

/**
   @name stop_IR_command
   @brief stop currently playing IR command by setting repeatCounter=0
   @return none
*/
void stop_IR_command();

/**
   @name list_IR_command
   @brief prints all IR command names stored in EEPROM to serial interface
   @return none
*/
void list_IR_commands();

/**
   @name delete_IR_command
   @brief deletes IR command of given names from EEPROM
          If you want to clear all IR commands, use '\0' as name
   @param name name of the IR command to be deleted
   @return 1 if successful, 0 otherwise
*/
uint8_t delete_IR_command(char * name);

/**
   @name set_IR_timeout
   @brief sets the inter-edge timeout for IR recording 
   @param us timeout value in microseconds
   @return none
*/
void set_IR_timeout(uint32_t us);

/**
   @name get_IR_timeout
   @brief gets the inter-edge timeout for IR recording 
   @return timeout value in microseconds
*/
uint32_t get_IR_timeout();

/**
   @name wipe_IR_commands
   @brief deletes all IR commands from EEPROM
   @return none
*/
void wipe_IR_commands();

#endif
