/*
 * SerialCommandHandler.cpp
 *
 * Serial command interface for DEBUG_MODE testing
 * Allows control of Spark amp via serial commands without physical buttons
 */

#ifdef DEBUG_MODE

#include "SerialCommandHandler.h"
#include "SparkStatus.h"

// Global instance
SerialCommandHandler serialCmdHandler;

void SerialCommandHandler::init(SparkDataControl *dataControl, SparkPresetControl &presetControl) {
    sparkDC_ = dataControl;
    presetCtrl_ = &presetControl;

    Serial.println("\n========================================");
    Serial.println("DEBUG_MODE: Serial Command Interface Enabled");
    Serial.println("Type 'help' for available commands");
    Serial.println("========================================\n");
}

void SerialCommandHandler::processCommands() {
    if (Serial.available() > 0) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        cmd.toLowerCase();

        if (cmd.length() == 0) {
            return; // Ignore empty lines
        }

        Serial.print("> ");
        Serial.println(cmd);

        // Parse command
        if (cmd.startsWith("p")) {
            // Preset command: p1, p2, p3, p4
            int presetNum = cmd.substring(1).toInt();
            handlePresetCommand(presetNum);
        }
        else if (cmd == "b+" || cmd == "b") {
            // Bank up
            handleBankCommand(true);
        }
        else if (cmd == "b-") {
            // Bank down
            handleBankCommand(false);
        }
        else if (cmd.startsWith("bank")) {
            // Direct bank selection: bank0, bank1, bank2, etc.
            int bankNum = cmd.substring(4).toInt();
            handleBankSelect(bankNum);
        }
        else if (cmd.startsWith("fx")) {
            // Toggle effect: fx1-fx7
            int effectNum = cmd.substring(2).toInt();
            handleEffectCommand(effectNum);
        }
        else if (cmd == "mode") {
            // Switch between preset and FX mode
            handleModeToggle();
        }
        else if (cmd == "status" || cmd == "s") {
            // Print current status
            printStatus();
        }
        else if (cmd == "help" || cmd == "h" || cmd == "?") {
            // Print help
            printHelp();
        }
        else {
            Serial.println("Unknown command. Type 'help' for available commands.");
        }
    }
}

void SerialCommandHandler::handlePresetCommand(int presetNum) {
    if (!SparkDataControl::isAmpConnected()) {
        Serial.println("ERROR: Not connected to Spark amp");
        return;
    }

    if (presetNum < 1 || presetNum > 4) {
        Serial.printf("ERROR: Invalid preset number %d (must be 1-4)\n", presetNum);
        return;
    }

    Serial.printf("Switching to preset %d...\n", presetNum);
    if (presetCtrl_->processPresetSelect(presetNum)) {
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
    }
}

void SerialCommandHandler::handleBankCommand(bool increase) {
    if (!SparkDataControl::isAmpConnected()) {
        Serial.println("ERROR: Not connected to Spark amp");
        return;
    }

    int currentBank = presetCtrl_->activeBank();

    if (increase) {
        Serial.printf("Bank up (from %d)...\n", currentBank);
        presetCtrl_->increaseBank();
    } else {
        Serial.printf("Bank down (from %d)...\n", currentBank);
        presetCtrl_->decreaseBank();
    }

    int newBank = presetCtrl_->pendingBank();
    Serial.printf("New bank: %d\n", newBank);
}

void SerialCommandHandler::handleBankSelect(int bankNum) {
    if (!SparkDataControl::isAmpConnected()) {
        Serial.println("ERROR: Not connected to Spark amp");
        return;
    }

    int currentBank = presetCtrl_->activeBank();
    int numBanks = presetCtrl_->numberOfBanks();

    if (bankNum < 0 || bankNum >= numBanks) {
        Serial.printf("ERROR: Invalid bank number %d (valid range: 0-%d)\n", bankNum, numBanks - 1);
        return;
    }

    Serial.printf("Selecting bank %d (from %d)...\n", bankNum, currentBank);
    presetCtrl_->setBank(bankNum);

    int newBank = presetCtrl_->pendingBank();
    Serial.printf("Bank set to: %d\n", newBank);
}

void SerialCommandHandler::handleEffectCommand(int effectNum) {
    if (!SparkDataControl::isAmpConnected()) {
        Serial.println("ERROR: Not connected to Spark amp");
        return;
    }

    if (effectNum < 1 || effectNum > 7) {
        Serial.printf("ERROR: Invalid effect number %d (must be 1-7)\n", effectNum);
        return;
    }

    const char *effectNames[] = {
        "Noise Gate", "Compressor", "Drive", "Amp", "Modulation", "Delay", "Reverb"
    };

    Serial.printf("Toggling %s...\n", effectNames[effectNum - 1]);
    if (SparkDataControl::toggleEffect(effectNum)) {
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
    }
}

void SerialCommandHandler::handleModeToggle() {
    if (!SparkDataControl::isAmpConnected()) {
        Serial.println("ERROR: Not connected to Spark amp");
        return;
    }

    Serial.println("Toggling sub-mode (PRESET <-> FX)...");
    if (sparkDC_->toggleSubMode()) {
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
    }
}

void SerialCommandHandler::printStatus() {
    Serial.println("\n========== STATUS ==========");

    // Connection status
    Serial.print("BLE Connection: ");
    if (SparkDataControl::isAmpConnected()) {
        Serial.println("CONNECTED");
    } else {
        Serial.println("DISCONNECTED");
    }

    // Operation mode
    Serial.print("Operation Mode: ");
    OperationMode opMode = sparkDC_->operationMode();
    switch (opMode) {
        case SPARK_MODE_APP:
            Serial.println("APP");
            break;
        case SPARK_MODE_AMP:
            Serial.println("AMP");
            break;
        case SPARK_MODE_KEYBOARD:
            Serial.println("KEYBOARD");
            break;
        default:
            Serial.println("UNKNOWN");
    }

    // Sub mode (if in APP mode)
    if (opMode == SPARK_MODE_APP) {
        Serial.print("Sub Mode: ");
        SubMode subMode = sparkDC_->subMode();
        switch (subMode) {
            case SUB_MODE_PRESET:
                Serial.println("PRESET");
                break;
            case SUB_MODE_FX:
                Serial.println("FX");
                break;
            case SUB_MODE_LOOPER:
                Serial.println("LOOPER");
                break;
            case SUB_MODE_SPK_LOOPER:
                Serial.println("SPARK_LOOPER");
                break;
            case SUB_MODE_TUNER:
                Serial.println("TUNER");
                break;
            default:
                Serial.println("UNKNOWN");
        }
    }

    // Preset info (if connected)
    if (SparkDataControl::isAmpConnected()) {
        Serial.printf("Active Bank: %d\n", presetCtrl_->activeBank());
        Serial.printf("Active Preset: %d\n", presetCtrl_->activePresetNum());
        Serial.printf("Preset Name: %s\n", presetCtrl_->activePreset().name.c_str());

        // Effect status
        Serial.println("\nEffects:");
        const Preset &preset = presetCtrl_->activePreset();
        if (preset.pedals.size() >= 7) {
            for (int i = 0; i < 7; i++) {
                const Pedal &pedal = preset.pedals[i];
                Serial.printf("  %d. %-15s : %s\n",
                    i + 1,
                    pedal.name.c_str(),
                    pedal.isOn ? "ON" : "OFF"
                );
            }
        }
    }

    Serial.println("============================\n");
}

void SerialCommandHandler::printHelp() {
    Serial.println("\n======== AVAILABLE COMMANDS ========");
    Serial.println("Preset Control:");
    Serial.println("  p1-p4    : Switch to preset 1-4");
    Serial.println("  b+ / b   : Bank up");
    Serial.println("  b-       : Bank down");
    Serial.println("  bank0-3  : Select bank directly (bank0=A, bank1=B, etc.)");
    Serial.println();
    Serial.println("Effect Control:");
    Serial.println("  fx1      : Toggle Noise Gate");
    Serial.println("  fx2      : Toggle Compressor");
    Serial.println("  fx3      : Toggle Drive");
    Serial.println("  fx4      : Toggle Amp");
    Serial.println("  fx5      : Toggle Modulation");
    Serial.println("  fx6      : Toggle Delay");
    Serial.println("  fx7      : Toggle Reverb");
    Serial.println("  mode     : Toggle PRESET/FX mode");
    Serial.println();
    Serial.println("Information:");
    Serial.println("  status/s : Show current status");
    Serial.println("  help/h/? : Show this help");
    Serial.println("=====================================\n");
}

#endif // DEBUG_MODE
