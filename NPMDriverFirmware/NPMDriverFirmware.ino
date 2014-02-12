// - - - - -
// NPM2000 Driver Firmware
// Implements a complete RDM responder

#include <EEPROM.h>
#include <DMXSerial2.h>

#define DmxModePin 2

#define NUM_OUTPUTS 8

uint8_t outputs[NUM_OUTPUTS] = { 3, 4, 5, 6, 7, 8, 9, 10 };

const char privateChannelsMessage[] = "Private Channel Map";

#define PID_PRIVATE_CHANNELS 0x8000

// see DMXSerial2.h for the definition of the fields of this structure
const uint16_t my_pids[] = { PID_PRIVATE_CHANNELS };

struct RDMINIT rdmInit = { "raumzeitlabor.de", // Manufacturer Label
		1, // Device Model ID
		"NPM2000", // Device Model Label
		NUM_OUTPUTS, // footprint
		(sizeof(my_pids) / sizeof(uint16_t)), my_pids, { 0x09, 0x7F, 0x23, 0x42,
				0x00, 0x00 }, E120_PRODUCT_CATEGORY_POWER, // Product category
		0x01000000 // Software Version
		};

void setup() {
	int i;

	// initialize the Serial interface to be used as an RDM Device Node.
	// There are several constants that have to be passed to the library so it can reposonse to the
	// corresponding commands for itself.
	pinMode(DmxModePin, OUTPUT);
	DMXSerial2.init(&rdmInit, processCommand, DmxModePin, HIGH, LOW);

	uint16_t start = DMXSerial2.getStartAddress();

	for (i = 0; i < sizeof(outputs); i++) {
		pinMode(outputs[i], OUTPUT);
	}
} // setup()

void loop() {
	// Calculate how long no data backet was received
	int i;
	unsigned long lastPacket = DMXSerial2.noDataSince();

	if (lastPacket < 30000) {
		for (i = 0; i < NUM_OUTPUTS; i++) {
			if (DMXSerial2.read(i + DMXSerial2.getStartAddress()) > 127) {
//              digitalWrite(outputs[i], HIGH);
			} else {
//              digitalWrite(outputs[i], LOW);
			}
		}

	}
	// check for unhandled RDM commands
	DMXSerial2.tick();
} // loop()

// This function was registered to the DMXSerial2 library in the initRDM call.
// Here device specific RDM Commands are implemented.
boolean processCommand(struct RDMDATA *rdm, uint16_t *nackReason) {
	byte CmdClass = rdm->CmdClass;     // command class
	uint16_t Parameter = rdm->Parameter;	   // parameter ID
	uint16_t i;
	boolean handled = false;

	if (CmdClass == E120_GET_COMMAND) {
		switch (Parameter) {
		case SWAPINT(E120_PARAMETER_DESCRIPTION):
			if (rdm->DataLength != 2) {
				*nackReason = E120_NR_FORMAT_ERROR;
			} else if (rdm->SubDev != 0) {
				*nackReason = E120_NR_SUB_DEVICE_OUT_OF_RANGE;
			} else if (READINT(rdm->Data) != PID_PRIVATE_CHANNELS) {
				*nackReason = E120_NR_DATA_OUT_OF_RANGE;
			} else {
				i = READINT(rdm->Data);

				if (i == PID_PRIVATE_CHANNELS) {
					PARAMETER_DESCRIPTION_RESPONSE *parameterDescriptionResponse =
							(PARAMETER_DESCRIPTION_RESPONSE *) (rdm->Data);

					parameterDescriptionResponse->pid = SWAPINT(i);
					parameterDescriptionResponse->pdlSize =
							sizeof(privateChannelsMessage);
					parameterDescriptionResponse->dataType =
							E120_DS_UNSIGNED_BYTE;
					parameterDescriptionResponse->commandClass = E120_CC_GET;
					parameterDescriptionResponse->type = 0;
					parameterDescriptionResponse->unit = E120_UNITS_NONE;
					parameterDescriptionResponse->prefix = E120_PREFIX_NONE;
					parameterDescriptionResponse->minValidValue = 0;
					parameterDescriptionResponse->maxValidValue = SWAPINT32(
							1UL);
					parameterDescriptionResponse->defaultValue = 0;

					memcpy(parameterDescriptionResponse->description,
							privateChannelsMessage,
							sizeof(privateChannelsMessage));
					rdm->DataLength = sizeof(PARAMETER_DESCRIPTION_RESPONSE);

					handled = true;
				}
			}
			break;
		case SWAPINT(PID_PRIVATE_CHANNELS):
			rdm->DataLength = NUM_OUTPUTS;
			for (i = 0; i < NUM_OUTPUTS; i++) {
				rdm->Data[i] = 1;
			}

			handled = true;
			break;
		}
	}
	return handled;
} // processCommand

// End.

