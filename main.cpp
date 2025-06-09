#define pin_magneticButton 2
#define pin_doorSensor 3
#define pin_buzzer 4
#define pin_relayOnGasPump 5
#include <EEPROM.h>

bool systemBlocked = false;
int magneticButtonPressedTime = 0;
int secondsHoldingMagneticButtonToUnblockSystem = 5;
int secondsWithoutPressingMagneticButtonToBlockSystem = 60;
bool temporizerToPressMagneticButton = true;
int secondsWithoutPressingMagneticButton = 0;
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

void unblockSystemIfMagneticButtonIsPressedLongEnough(bool magneticButtonPressed) {
    if (magneticButtonPressed) {
        notificationMagneticButtonPressed();
        magneticButtonPressedTime += 1;

        if (magneticButtonPressedTime > secondsHoldingMagneticButtonToUnblockSystem) {
            magneticButtonPressedTime = 0;
            systemBlocked = false;
            temporizerToPressMagneticButton = false;
            stopTemporizerUntilNextDoorOpen = true;
            secondsWithoutPressingMagneticButton = 0;
            EEPROM.update(0 , (byte)(0) ); // save unblocked state in EEPROM
        }
    } else {
        magneticButtonPressedTime = 0;
    }
}

void notificationMagneticButtonPressed() {
        buzzerActive(false);
        delay(50);
        buzzerActive(true);
        delay(50);
        buzzerActive(false);
}

void setup() {

    delay(500);
    Serial.begin(9600);
    
    pinMode(pin_magneticButton, INPUT_PULLUP);
    pinMode(pin_doorSensor, INPUT_PULLUP);
    pinMode(pin_buzzer, OUTPUT);
    pinMode(pin_relayOnGasPump, OUTPUT);

    systemBlocked = (bool)(EEPROM.read(0));
}

void loop() {

    delay(1000);
    bool magneticButtonPressed = !digitalRead(pin_magneticButton);
    bool doorOpen = digitalRead(pin_doorSensor);

    // if system is blocked:
    // - turn off gasPump
    // - turn on buzzer
    // - system is unblocked by holding magneticButton for 5 seconds
    if (systemBlocked) {
        gasPumpOff(true);
        buzzerActive(true);
        unblockSystemIfMagneticButtonIsPressedLongEnough(magneticButtonPressed);
    }

    // if system is unBlocked:
    // - turn on gasPump
    // - turn off buzzer
    // - temporizerToPressMagneticButton is triggered when door is open and stopTemporizerUntilNextDoorOpen is false... if not pressed within 60 seconds system is blocked
    else if (!systemBlocked) {
        gasPumpOff(false);
        buzzerActive(false);

        if (doorOpen && !stopTemporizerUntilNextDoorOpen) {
            temporizerToPressMagneticButton = true;
        }
        
        if (!doorOpen) {
            stopTemporizerUntilNextDoorOpen = false;
        }

        if (temporizerToPressMagneticButton) {
            secondsWithoutPressingMagneticButton += 1;

            unblockSystemIfMagneticButtonIsPressedLongEnough(magneticButtonPressed);

            if (secondsWithoutPressingMagneticButton > secondsWithoutPressingMagneticButtonToBlockSystem) {
                systemBlocked = true;
                EEPROM.update(0 , (byte)(1) ); // save blocked state in EEPROM
            }
        }
    }
}