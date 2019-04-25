#include "Motor.h"
#include "Arduino.h"

// private
const int HALFSTEP_SEQUENCE[8][4] = {
    {HIGH, LOW, LOW, LOW},
    {HIGH, HIGH, LOW, LOW},
    {LOW, HIGH, LOW, LOW},
    {LOW, HIGH, HIGH, LOW},
    {LOW, LOW, HIGH, LOW},
    {LOW, LOW, HIGH, HIGH},
    {LOW, LOW, LOW, HIGH},
    {HIGH, LOW, LOW, HIGH}};
const float FULL_ROTATION_STEPS = 509.50;
const float FULL_ROTATION_DEGREES = 360.00;

// public
void Motor::rotateMotor(bool clockwise, float degrees)
{
  float halfstepExecutions = FULL_ROTATION_STEPS *
                             (degrees / FULL_ROTATION_DEGREES);

  for (int i = 0; i < halfstepExecutions; i++)
  {
    if (clockwise)
    {
      for (int j = 0; j < 8; j++)
      {
        for (int k = 0; k < 4; k++)
        {
          digitalWrite(MOTOR_CONTROL_PINS[k], HALFSTEP_SEQUENCE[j][k]);
        }
        delay(1);
      }
    }
    else
    {
      for (int j = 7; j >= 0; j--)
      {
        for (int k = 3; k >= 0; k--)
        {
          digitalWrite(MOTOR_CONTROL_PINS[k], HALFSTEP_SEQUENCE[j][k]);
        }
        delay(1);
      }
    }
  }

  // Reset and turn off all pins after rotation
  int control_pins_len = (sizeof(MOTOR_CONTROL_PINS) / sizeof(MOTOR_CONTROL_PINS[0]));
  for (int x = 0; x < control_pins_len; x++)
  {
    digitalWrite(MOTOR_CONTROL_PINS[x], LOW);
  }
}