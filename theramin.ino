// testing git
#include <MIDI.h>

// Create and bind the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();

const int PING_PIN = 7; // Trigger Pin of Ultrasonic Sensor
const int ECHO_PIN = 6; // Echo Pin of Ultrasonic Sensor

// the minimum distance to be sensed by the Ultrasonic Sensor (min 200, max 500)
const int US_MIN = 200;
// the maximum distance to be sensed by the Ultrasonic Sensor
const int US_MAX = 2700;

const int CC_CHANNEL = 1;
const int CC_CONTROL_NUMBER = 3;
// delay between each loop in ms
const int LOOP_DELAY = 0;

// cc values for buttons
const int CC_OFF = 0;
const int CC_ON = 65;
const int CC_SUPER = 127;

// a variable to maintain threshold (min 10). The greater the threshold, the sensitive it is at the cost of Ceiling and flooring near the endpoint.
const int THRESHOLD = 35;

// a variable to control smoothing
const int SMOOTH_FACTOR = 0;

// the button that controls the input (pulled down with a 1K resistor)
// (5V -> Button terminal one; Button two terminal -> 1K resistor -> Gnd; Pin 10 -> between button & resistor)
const int BUTTON_PIN = 10;

// the test led at 13
const int TEST_LED = 11;

int CC_MAPPED_VALUE = 0;
int CC_CONTROL_VALUE = 0;
int OLD_CC_VAL = 0;

// sense the hand to provide a seamless output even when there is a difference bigger than the threshold
int HAND_SENSED = 0;

void setup()
{
    // MIDI rate
    // Serial.begin(9600);
    MIDI.begin(MIDI_CHANNEL_OMNI); // Listen to all incoming messages
    MIDI.sendControlChange(CC_CONTROL_NUMBER, CC_OFF, CC_CHANNEL);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(TEST_LED, OUTPUT);
}

void loop()
{
    int buttonState = digitalRead(BUTTON_PIN);
    digitalWrite(TEST_LED, buttonState);

    readSensor();

    if (buttonState == HIGH)
    {
        OLD_CC_VAL = 0;
        CC_CONTROL_VALUE = 1;
    }

    if (HAND_SENSED == 1 || abs(CC_CONTROL_VALUE - OLD_CC_VAL) < THRESHOLD)
    {
        // smoothen the tracking
        if (OLD_CC_VAL < CC_CONTROL_VALUE)
        {
            for (int i = OLD_CC_VAL; i <= CC_CONTROL_VALUE; i++)
            {
                MIDI.sendControlChange(CC_CONTROL_NUMBER, i, CC_CHANNEL);
                delay(SMOOTH_FACTOR);
            }
        }
        else
        {
            for (int i = OLD_CC_VAL; i >= CC_CONTROL_VALUE; i--)
            {
                MIDI.sendControlChange(CC_CONTROL_NUMBER, i, CC_CHANNEL);
                delay(SMOOTH_FACTOR);
            }
        }
        OLD_CC_VAL = CC_CONTROL_VALUE;
    }
    delay(LOOP_DELAY);
}

int limitCC()
{
    // limit the range only if hand sensed
    if (CC_CONTROL_VALUE < CC_OFF)
    {
        CC_CONTROL_VALUE = CC_OFF;
    }
    else if (CC_CONTROL_VALUE > CC_SUPER)
    {
        CC_CONTROL_VALUE = CC_SUPER;
    }
}

void readSensor()
{
    long secs = pingSensor();
    CC_CONTROL_VALUE = ceil(map(secs, US_MIN, US_MAX, CC_OFF, CC_SUPER));

    if (CC_CONTROL_VALUE >= CC_OFF && CC_CONTROL_VALUE <= CC_SUPER)
    {
        HAND_SENSED = 1;
    } else {
        HAND_SENSED = 0;
    }
    limitCC();
}

long pingSensor()
{
    long duration, cm;
    pinMode(PING_PIN, OUTPUT);
    digitalWrite(PING_PIN, LOW);
    digitalWrite(PING_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(PING_PIN, LOW);
    pinMode(ECHO_PIN, INPUT);
    duration = pulseIn(ECHO_PIN, HIGH);
    return duration;
}
