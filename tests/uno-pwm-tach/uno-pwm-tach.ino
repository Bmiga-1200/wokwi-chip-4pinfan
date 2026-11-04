const uint8_t TACH_PIN = 2;  // Tachometer input
const uint8_t PWM_PIN = 10;  // Only works with Pin 10
const uint8_t DUTY_CYCLE = 75;
const unsigned long SERIAL_UPDATE_INTERVAL = 1000;
const float FREQUENCY = 25000;

unsigned long serialLastUpdateMillis = 0;
unsigned long calcRPMLastMillis = 0;
volatile unsigned long tachPulseCount = 0;

#include "AVR_PWM.h"

AVR_PWM* PWM_Instance;

/**
 * @brief Increment the tach pulse count
 *
 * This function is an interrupt service routine (ISR) that increments
 * the tachPulseCount variable each time a pulse is detected on the
 * tachometer pin. The pulse count is used to calculate the RPM of
 * the fan.
 */
void CountTachPulse() {
  // Count each pulse from the tach pin of the fan
  tachPulseCount++;
}


/**
 * @brief Setup function for the Uno PWM Tach program
 *
 * This function initializes the serial monitor at 115200 baud and waits
 * for the serial port to open. It then prints a message to the serial
 * monitor and initializes the PWM and tachometer pins.
 *
 * The PWM is set to run at a frequency of 25KHz and is configured to
 * run at 100% speed for 2 seconds on startup. The tachometer is
 * configured to count pulses on the falling edge and to interrupt the
 * processor when a pulse is detected.
 *
 * @note This function should be called once at the beginning of the program.
 */
void setup() {
  // Setup tach
  Serial.begin(115200);
  while (!Serial)
    ;
  delay(100);
  Serial.print(F("\nStarting fan ..."));

  PWM_Instance = new AVR_PWM(PWM_PIN, FREQUENCY, DUTY_CYCLE);

  if (PWM_Instance) {
    PWM_Instance->setPWM();
  }

  pinMode(TACH_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TACH_PIN), CountTachPulse, FALLING);

  // Setup PWM control
  pinMode(PWM_PIN, OUTPUT);
  // Fast PWM Mode, Prescaler = /8
  // PWM on Pin 3, Pin 11 disabled
  // 16Mhz / 8 / (79 + 1) = 25Khz
  TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS21);

  // Set TOP
  OCR2A = 79;  // TOP - DO NOT CHANGE, SETS PWM PULSE RATE

  // Run fan at 100% speed for 2 seconds to ensure it starts.
  // Intel spec says run at 30% speed max on startup but this
  // is easier. You can change the initial value of 0CR2B if
  // you want it to be "correct".
  // Note: duty cycle for Pin 3 (0-79) generates 1 500nS
  // pulse even when 0.
  OCR2B = 79;  // duty cycle for Pin 1(PB1)
  delay(2000);
}


/**
 * @brief Calculate the RPM of the fan based on time passed and pulses counted.
 *
 * This function calculates the RPM of the fan by using the time passed since
 * the last call to CalculateRPM and the pulses counted by the tachometer
 * in that time. The RPM is calculated by dividing the pulse count by the time
 * passed and multiplying by 60. The pulse count is then reset to zero for the
 * next round of calculations.
 *
 * @return The calculated RPM of the fan as an unsigned long.
 */
unsigned long CalculateRPM() {
  // Calculate RPM based on time passed and pulses counted.

  noInterrupts();

  // Calculate RPM
  float elapsedMS = (millis() - calcRPMLastMillis);
  unsigned long rpm = (tachPulseCount / (elapsedMS / 500.0)) * 60;

  // Reset for next round
  calcRPMLastMillis = millis();
  tachPulseCount = 0;

  interrupts();

  return rpm;
}


/**
 * @brief Main loop of the Arduino program
 *
 * This function is the main loop of the Arduino program. It updates the
 * serial monitor with the current RPM of the fan every second.
 */
void loop() {

  // Update serial monitor with RPM every second
  if (millis() - serialLastUpdateMillis > SERIAL_UPDATE_INTERVAL) {
    Serial.print("Fan RPM = ");
    Serial.println(CalculateRPM());
    serialLastUpdateMillis = millis();
  }
}
