// - - - - -
// DmxSerial2 - A hardware supported interface to DMX and RDM.
// RDMSerialRecv.ino: Sample RDM application.
//
// Copyright (c) 2011-2013 by Matthias Hertel, http://www.mathertel.de
// This work is licensed under a BSD style license. See http://www.mathertel.de/License.aspx
// 
// This Arduino project is a sample application for the DMXSerial2 library that shows
// how a 3 channel receiving RDM client can be implemented.
// The 3 channels are used for PWM Output:
// address (startAddress) + 0 (red) -> PWM Port 9
// address (startAddress) + 1 (green) -> PWM Port 6
// address (startAddress) + 2 (blue) -> PWM Port 5
//
// Ths sample shows how Device specific RDM Commands are handled in the processCommand function.
// The following RDM commands are implemented here:
// E120_LAMP_HOURS
// E120_DEVICE_HOURS
//
// More documentation and samples are available at http://www.mathertel.de/Arduino
// 06.12.2012 created from DMXSerialRecv sample.
// 09.12.2012 added first RDM response.
// 22.01.2013 first published version to support RDM
// 03.03.2013 Using DMXSerial2 as a library
// 15.05.2013 Arduino Leonard and Arduino MEGA compatibility
// 15.12.2013 ADD: output information on a LEONARDO board by using the #define SERIAL_DEBUG definition
//            If you have to save pgm space you can delete the inner lines of this "#if" blocks
// 24.01.2014 Peter Newman/Sean Sill: Get device specific PIDs returning properly in supportedParameters
// 24.01.2014 Peter Newman: Make the device specific PIDs compliant with the OLA RDM Tests. Add device model ID option
// - - - - -

#include <EEPROM.h>
#include <DMXSerial2.h>

#define DmxModePin 2

uint8_t outputs[8] = {3,4,5,6,7,8,9,10};

#define NUM_OUTPUTS 8

// see DMXSerial2.h for the definition of the fields of this structure
const uint16_t my_pids[] = {E120_DEVICE_HOURS, E120_LAMP_HOURS};
struct RDMINIT rdmInit = {
  "raumzeitlabor.de", // Manufacturer Label
  1, // Device Model ID
  "NPM2000", // Device Model Label
  NUM_OUTPUTS, // footprint
  (sizeof(my_pids)/sizeof(uint16_t)), my_pids
};


void setup () {
  int i;
  
  // initialize the Serial interface to be used as an RDM Device Node.
  // There are several constants that have to be passed to the library so it can reposonse to the
  // corresponding commands for itself.
  pinMode(DmxModePin, OUTPUT);
  DMXSerial2.init(&rdmInit, processCommand, DmxModePin, HIGH, LOW);

  uint16_t start = DMXSerial2.getStartAddress();

  for (i=0;i<sizeof(outputs);i++) {
     pinMode(outputs[i], OUTPUT); 
  }
} // setup()


void loop() {
  // Calculate how long no data backet was received
  int i;
  unsigned long lastPacket = DMXSerial2.noDataSince();

  if (lastPacket < 30000) {
    for (i=0;i<NUM_OUTPUTS;i++) {
        if (DMXSerial2.read(i+DMXSerial2.getStartAddress()) > 127) {
              digitalWrite(outputs[i], HIGH);
			} else {
              digitalWrite(outputs[i], LOW);
			}
    }

  }  
  // check for unhandled RDM commands
  DMXSerial2.tick();
} // loop()


// This function was registered to the DMXSerial2 library in the initRDM call.
// Here device specific RDM Commands are implemented.
boolean processCommand(struct RDMDATA *rdm, uint16_t *nackReason)
{
  byte CmdClass       = rdm->CmdClass;     // command class
  uint16_t Parameter  = rdm->Parameter;	   // parameter ID
  boolean handled = false;

// This is a sample of how to return some device specific data
  if (Parameter == SWAPINT(E120_DEVICE_HOURS)) { // 0x0400
    if (CmdClass == E120_GET_COMMAND) {
      if (rdm->DataLength > 0) {
        // Unexpected data
        *nackReason = E120_NR_FORMAT_ERROR;
      } else if (rdm->SubDev != 0) {
        // No sub-devices supported
        *nackReason = E120_NR_SUB_DEVICE_OUT_OF_RANGE;
      } else {
        rdm->DataLength = 4;
        rdm->Data[0] = 0;
        rdm->Data[1] = 0;
        rdm->Data[2] = 2;
        rdm->Data[3] = 0;
        handled = true;
      }
    } else if (CmdClass == E120_SET_COMMAND) {
      // This device doesn't support set
      *nackReason = E120_NR_UNSUPPORTED_COMMAND_CLASS;
    }

  } else if (Parameter == SWAPINT(E120_LAMP_HOURS)) { // 0x0401
    if (CmdClass == E120_GET_COMMAND) {
      if (rdm->DataLength > 0) {
        // Unexpected data
        *nackReason = E120_NR_FORMAT_ERROR;
      } else if (rdm->SubDev != 0) {
        // No sub-devices supported
        *nackReason = E120_NR_SUB_DEVICE_OUT_OF_RANGE;
      } else {
        rdm->DataLength = 4;
        rdm->Data[0] = 0;
        rdm->Data[1] = 0;
        rdm->Data[2] = 0;
        rdm->Data[3] = 1;
        handled = true;
      }
    } else if (CmdClass == E120_SET_COMMAND) {
      // This device doesn't support set
      *nackReason = E120_NR_UNSUPPORTED_COMMAND_CLASS;
    }
  } // if
  
  return handled;
} // processCommand


// End.

