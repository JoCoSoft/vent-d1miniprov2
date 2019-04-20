const int MOTOR_CONTROL_PINS[] = {2, 0, 4, 5};

class Motor
{
public:
  static void rotateMotor(bool clockwise, float degrees);
};