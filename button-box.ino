#include <Arduino.h>
#include <Joystick.h>

// Button matrix
const byte ROW_PINS[] = {13, 12, 11, 10, 9};
const byte ROW_COUNT = sizeof(ROW_PINS) / sizeof(ROW_PINS[0]);
const byte COL_PINS[] = {8, 7, 6, 5};
const byte COL_COUNT = sizeof(COL_PINS) / sizeof(COL_PINS[0]);

byte previousButtonMatrixValues[ROW_COUNT][COL_COUNT];

// Physical joystick
const byte JOYSTICK_X_AXIS = A2;
const byte JOYSTICK_Y_AXIS = A1;
const byte JOYSTICK_BUTTON = A0;

int previousJoystickValues[3] = {512, 512, 0};

const byte ROTARY_TABLE[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

const byte ROTARY_ENCODER_AMOUNT = 4;
const byte ROTARY_ENCODER_PINS[ROTARY_ENCODER_AMOUNT][2] = {{4, 3}, {2, 1}, {0, A5}, {A4, A3}};
byte rotaryEncoderCodes[ROTARY_ENCODER_AMOUNT];

byte previousRotaryEncoderValues[ROTARY_ENCODER_AMOUNT];

Joystick_ Joystick(
    JOYSTICK_DEFAULT_REPORT_ID,
    JOYSTICK_TYPE_GAMEPAD,
    (ROW_COUNT * COL_COUNT) + (ROTARY_ENCODER_AMOUNT * 2) + 1, // Set correct amount of buttons, taking into account the 'buttons' for the rotary encodes and the button on the physical joystick
    0,                                                         // Amount of hat switches
    true,                                                      // X-axis
    true,                                                      // Y-axis
    false,                                                     // Z-axis
    false,                                                     // Rudder
    false,                                                     // Throttle
    false,                                                     // Accelerator
    false,                                                     // Brake,
    false                                                      // Steering
);

void setup() {
    buttonMatrixSetup();
    joystickSetup();
    rotaryEncoderSetup();

    Joystick.begin();
}

void loop() {
    checkMatrix();
    checkJoystick();
    checkRotaryEncoders();
}

// Checks if a button is pressed and sets the corresponding digital joystick button to on
void checkMatrix() {
    for (byte i = 0; i < COL_COUNT; i++) {
        byte currentColumn = COL_PINS[i];

        pinMode(currentColumn, OUTPUT);

        digitalWrite(currentColumn, LOW);

        for (byte j = 0; j < ROW_COUNT; j++) {
            byte currentRow = ROW_PINS[j];

            pinMode(currentRow, INPUT_PULLUP);

            byte button = (i * (COL_COUNT + 1)) + j; // The + 1 prevents every last button of a column being used by the first button of every column on the next row
            byte newValue = !digitalRead(currentRow);

            if (newValue != previousButtonMatrixValues[j][i]) {
                Joystick.setButton(button, newValue);

                previousButtonMatrixValues[j][i] = newValue;
            }

            pinMode(currentRow, INPUT);
        }

        pinMode(currentColumn, INPUT);
    }
}

void checkRotaryEncoders() {
    for (byte i = 0; i < ROTARY_ENCODER_AMOUNT; i++) {
        byte outputA = digitalRead(ROTARY_ENCODER_PINS[i][0]);

        Joystick.setButton((ROW_COUNT * COL_COUNT) + (i * 2), LOW);
        Joystick.setButton((ROW_COUNT * COL_COUNT) + (i * 2) + 1, LOW);

        for (byte i = 0; i < ROTARY_ENCODER_AMOUNT; i++) {
            rotaryEncoderCodes[i] <<= 2;

            if (digitalRead(ROTARY_ENCODER_PINS[i][0]))
                rotaryEncoderCodes[i] |= 0x02;
            if (digitalRead(ROTARY_ENCODER_PINS[i][1]))
                rotaryEncoderCodes[i] |= 0x01;

            rotaryEncoderCodes[i] &= 0x0f;

            if (ROTARY_TABLE[(rotaryEncoderCodes[i] & 0x0f)]) {
                if ((rotaryEncoderCodes[i] & 0x0f) == 0x0b) {
                    Joystick.setButton((ROW_COUNT * COL_COUNT) + (i * 2), HIGH);

                    delay(35);
                }

                if ((rotaryEncoderCodes[i] & 0x0f) == 0x07) {
                    Joystick.setButton((ROW_COUNT * COL_COUNT) + (i * 2) + 1, HIGH);

                    delay(35);
                }
            }
        }
    }
}

void checkJoystick() {
    int newValues[3] = {analogRead(JOYSTICK_X_AXIS), analogRead(JOYSTICK_Y_AXIS), !digitalRead(JOYSTICK_BUTTON)};

    if (newValues[0] != previousJoystickValues[0]) {
        Joystick.setXAxis(newValues[0]);

        previousJoystickValues[0] = newValues[0];
    }

    if (newValues[1] != previousJoystickValues[1]) {
        Joystick.setYAxis(newValues[1]);

        previousJoystickValues[1] = newValues[1];
    }

    if (newValues[2] != previousJoystickValues[2]) {
        Joystick.setButton((ROW_COUNT * COL_COUNT) + (ROTARY_ENCODER_AMOUNT * 2), newValues[2]);

        previousJoystickValues[2] = newValues[2];
    }
}

// Handles setup for the button matrix
void buttonMatrixSetup() {
    for (byte i = 0; i < ROW_COUNT; i++) {
        pinMode(ROW_PINS[i], INPUT);
    }

    for (byte i = 0; i < COL_COUNT; i++) {
        pinMode(COL_PINS[i], INPUT_PULLUP);
    }

    // Set all previous button values to 'Not pressed'
    for (byte i = 0; i < ROW_COUNT; i++) {
        for (byte j = 0; j < COL_COUNT; j++) {
            byte button = (i * COL_COUNT) + j;

            Joystick.setButton(button, LOW);

            previousButtonMatrixValues[i][j] = 0;
        }
    }
}

void rotaryEncoderSetup() {
    for (byte i = 0; i < ROTARY_ENCODER_AMOUNT; i++) {
        pinMode(ROTARY_ENCODER_PINS[i][0], INPUT_PULLUP);
        pinMode(ROTARY_ENCODER_PINS[i][1], INPUT_PULLUP);

        rotaryEncoderCodes[i] = 0;
    }
}

void joystickSetup() {
    pinMode(JOYSTICK_X_AXIS, INPUT);
    pinMode(JOYSTICK_Y_AXIS, INPUT);
    pinMode(JOYSTICK_BUTTON, INPUT_PULLUP);
}
