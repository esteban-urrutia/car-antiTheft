
#include <EEPROM.h>

const int pin_magneticButton = 2;
const int pin_doorSensor = 3;
const int pin_buzzer = 4;
const int pin_relayOnGasPump = 5;

const int secondsWithoutPressingMagneticButtonToBlockSystem = 60;
int secondsWithoutPressingMagneticButton = 0;

const int secondsHoldingMagneticButtonToUnblockSystem = 2;
int secondsHoldingMagneticButton = 0;

bool systemBlocked = false;
bool temporizerToPressMagneticButton = true;
bool stopTemporizerUntilNextDoorOpen = false;

void gasPumpOff(bool status) {
    if (status) {
        digitalWrite(pin_relayOnGasPump, HIGH); // turn off gasPump
    } else {
        digitalWrite(pin_relayOnGasPump, LOW); // turn on gasPump
    }
}

void buzzerActive(bool status) {
    if (status) {
        digitalWrite(pin_buzzer, HIGH); // turn on buzzer
    } else {
        digitalWrite(pin_buzzer, LOW); // turn off buzzer
    }
}

// beep x1
void notificationMagneticButtonPressed() {
        buzzerActive(false);
        delay(50);
        buzzerActive(true); // beep
        delay(50);
        buzzerActive(false);
}

// beep x2
void notificationSystemUnblockedByPressing() {
        buzzerActive(false);
        delay(50);
        buzzerActive(true); // beep
        delay(50);
        buzzerActive(false);
        delay(50);
        buzzerActive(true); // beep
        delay(50);
        buzzerActive(false);
}

// beep x5
void notificationSystemUnblockedByHolding() {
        buzzerActive(false);
        delay(50);
        buzzerActive(true); // beep
        delay(50);
        buzzerActive(false);
        delay(50);
        buzzerActive(true); // beep
        delay(50);
        buzzerActive(false);
        delay(50);
        buzzerActive(true); // beep
        delay(50);
        buzzerActive(false);
        delay(50);
        buzzerActive(true); // beep
        delay(50);
        buzzerActive(false);
        delay(50);
        buzzerActive(true); // beep
        delay(50);
        buzzerActive(false);
}

void unblockSystemIfMagneticButtonIsHoldedLongEnough(bool magneticButtonPressed) {
    if (magneticButtonPressed) {
        notificationMagneticButtonPressed();
        secondsHoldingMagneticButton += 1;

        if (secondsHoldingMagneticButton >= secondsHoldingMagneticButtonToUnblockSystem) {
            secondsHoldingMagneticButton = 0;
            systemBlocked = false;
            temporizerToPressMagneticButton = false;
            stopTemporizerUntilNextDoorOpen = true;
            secondsWithoutPressingMagneticButton = 0;
            EEPROM.update(0 , (byte)(0) ); // save unblocked state in EEPROM
            notificationSystemUnblockedByHolding();
        }
    } else {
        secondsHoldingMagneticButton = 0;
    }
}

void unblockSystemIfMagneticButtonIsPressed(bool magneticButtonPressed) {
    if (magneticButtonPressed) {
        notificationSystemUnblockedByPressing();
        systemBlocked = false;
        temporizerToPressMagneticButton = false;
        stopTemporizerUntilNextDoorOpen = true;
        secondsWithoutPressingMagneticButton = 0;
        EEPROM.update(0 , (byte)(0) ); // save unblocked state in EEPROM
    }
}

void setup() {
    delay(1000);
    pinMode(pin_magneticButton, INPUT_PULLUP);
    pinMode(pin_doorSensor, INPUT_PULLUP);
    pinMode(pin_buzzer, OUTPUT);
    pinMode(pin_relayOnGasPump, OUTPUT);

    systemBlocked = (bool)(EEPROM.read(0));
}

void loop() {
    bool magneticButtonPressed = !digitalRead(pin_magneticButton);
    bool doorOpen = digitalRead(pin_doorSensor);

    // if system is blocked:
    // - turn off gasPump
    // - turn on buzzer
    // - system is unblocked by holding magneticButton for 5 seconds
    if (systemBlocked) {
        gasPumpOff(true);
        buzzerActive(true);
        unblockSystemIfMagneticButtonIsHoldedLongEnough(magneticButtonPressed);
    }

    // if system is unBlocked:
    // - turn on gasPump
    // - turn off buzzer
    // - temporizerToPressMagneticButton is triggered when door is open and stopTemporizerUntilNextDoorOpen is false... if magnetic button is not pressed within 60 seconds the system is blocked
    //   (temporizer is triggered after closing the door and then opening it again)
    else if (!systemBlocked) {
        gasPumpOff(false);
        buzzerActive(false);

        if (!doorOpen) {
            stopTemporizerUntilNextDoorOpen = false;
        }

        if (doorOpen && !stopTemporizerUntilNextDoorOpen) {
            temporizerToPressMagneticButton = true;
        }
        
        if (temporizerToPressMagneticButton) {
            secondsWithoutPressingMagneticButton += 1;

            unblockSystemIfMagneticButtonIsPressed(magneticButtonPressed);

            if (secondsWithoutPressingMagneticButton > secondsWithoutPressingMagneticButtonToBlockSystem) {
                systemBlocked = true;
                EEPROM.update(0 , (byte)(1) ); // save blocked state in EEPROM
            }
        }
    }

    delay(1000);
}
