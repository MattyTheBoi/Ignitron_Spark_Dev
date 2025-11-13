#ifndef SERIAL_COMMAND_HANDLER_H
#define SERIAL_COMMAND_HANDLER_H

#ifdef DEBUG_MODE

#include "Arduino.h"
#include "SparkDataControl.h"
#include "SparkPresetControl.h"

/**
 * @brief Serial command handler for DEBUG_MODE testing
 *
 * Allows control of Spark amp via serial commands without physical buttons.
 * Useful for testing BLE connectivity and functionality before building hardware.
 *
 * Available commands:
 *   p1-p4    : Switch to preset 1-4
 *   b+       : Bank up
 *   b-       : Bank down
 *   fx1-fx7  : Toggle effect (1=NoiseGate, 2=Comp, 3=Drive, 4=Amp, 5=Mod, 6=Delay, 7=Reverb)
 *   mode     : Toggle between PRESET and FX mode
 *   status   : Show current connection and state
 *   help     : Show available commands
 */

class SerialCommandHandler {
public:
    /**
     * @brief Initialize the serial command handler
     * @param dataControl Pointer to SparkDataControl instance
     * @param presetControl Reference to SparkPresetControl instance
     */
    void init(SparkDataControl* dataControl, SparkPresetControl& presetControl);

    /**
     * @brief Process serial input (call from main loop)
     */
    void processCommands();

private:
    SparkDataControl* sparkDC_;
    SparkPresetControl* presetCtrl_;

    void printHelp();
    void printStatus();
    void handlePresetCommand(int presetNum);
    void handleBankCommand(bool increase);
    void handleBankSelect(int bankNum);
    void handleEffectCommand(int effectNum);
    void handleModeToggle();
};

// Global instance
extern SerialCommandHandler serialCmdHandler;

#endif // DEBUG_MODE

#endif // SERIAL_COMMAND_HANDLER_H
