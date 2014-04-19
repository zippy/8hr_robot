#include <Servo.h> // include the Servo library
#include <Bounce.h>

typedef struct {
  void (*fn)();
}State;

#define SERVO_VERY_FAST   60
#define SERVO_FAST        40
#define SERVO_SLOW        20
#define SERVO_VERY_SLOW   10
#define SERVO_STOP        0

#define SERVO_FORWARD  false
#define SERVO_BACKWARD true

#define FORWARD             0
#define BOTH_FORWARD_WAIT   1
#define BACK_TURN           2
#define BACKUP_180_PHASE_1  3
#define BACKUP_180_PHASE_2  4

#define AL_PIN 5
#define AR_PIN 4

#define R_MOTOR 7
#define L_MOTOR 6

#define LED_PIN 3

#define BACKUP_WAIT_TIME         150
#define BACK_TURN_TIME           1500
#define BACKUP_180_PHASE_1_TIME  1000
#define BACKUP_180_PHASE_2_TIME  1600


Bounce al = Bounce();
Bounce ar = Bounce();

boolean alUpdate;
boolean arUpdate;
int fadeAmount;

// create the servo objects
Servo leftMotor;
Servo rightMotor;

int val;
int ledVal;

State states[] = {
  {forward},
  {both_forward_wait},
  {back_turn},
  {backup_180_phase_1},
  {backup_180_phase_2},
};

State state;
boolean isLeft;
long time;
long ledTime;

void forward() {
  Serial.print("forward\n");
  if (alUpdate || arUpdate) {
    setState(BOTH_FORWARD_WAIT);
    isLeft = al.read() == LOW;
  }

  setSpeed(R_MOTOR, SERVO_VERY_FAST, SERVO_FORWARD);
  setSpeed(L_MOTOR, SERVO_VERY_FAST, SERVO_FORWARD);
}

void both_forward_wait() {
  Serial.print("both_forward_wait\n");
  if (millis() - time > BACKUP_WAIT_TIME)
  {
    setState(BACK_TURN);
  }
  else if ((isLeft && arUpdate) || (!isLeft && alUpdate))
  {
    setState(BACKUP_180_PHASE_1);
  }
  setSpeed(R_MOTOR, SERVO_SLOW, SERVO_FORWARD);
  setSpeed(L_MOTOR, SERVO_SLOW, SERVO_FORWARD);
}

void back_turn()
{
  Serial.print("back_turn\n");
  if ((millis() - time) > BACK_TURN_TIME)
  {
    setState(FORWARD);
  }
  else
  {
    if (isLeft)
    {
      setSpeed(R_MOTOR, SERVO_VERY_FAST, SERVO_BACKWARD);
      setSpeed(L_MOTOR, SERVO_VERY_SLOW, SERVO_BACKWARD);
    }
    else
    {
      setSpeed(R_MOTOR, SERVO_VERY_SLOW, SERVO_BACKWARD);
      setSpeed(L_MOTOR, SERVO_VERY_FAST, SERVO_BACKWARD);
    }
  }
}

void backup_180_phase_1()
{
  Serial.print("backup_180_phase_1\n");
  if ((millis() - time) > BACKUP_180_PHASE_1_TIME)
  {
    setState(BACKUP_180_PHASE_2);
  }
  else
  {
    setSpeed(R_MOTOR, SERVO_FAST, SERVO_BACKWARD);
    setSpeed(L_MOTOR, SERVO_FAST, SERVO_BACKWARD);
  }
}

void backup_180_phase_2()
{
  Serial.print("backup_180_phase_2\n");
  if ((millis() - time) > BACKUP_180_PHASE_2_TIME)
  {
    
    setState(FORWARD);
  }
  else
  {
    setSpeed(L_MOTOR, SERVO_FAST, SERVO_BACKWARD);
    setSpeed(R_MOTOR, SERVO_FAST, SERVO_FORWARD);
  }
}

void setup_antenna(int pin,Bounce *a) {
  a->interval(15);
  a->attach(pin);
  digitalWrite(pin,HIGH);
}

void setState(int stateValue)
{
  time = millis();
  state = states[stateValue];
}

void setup()
{
    Serial.begin(115200);
    Serial.print("test\n");
  pinMode(LED_PIN, OUTPUT);

  setup_antenna(AL_PIN,&al);
  setup_antenna(AR_PIN,&ar);
  leftMotor.attach(L_MOTOR);  // attaches the servo on pin 13 to a servo object
  rightMotor.attach(R_MOTOR); // attaches the servo on pin 12 to a servo object

  setState(FORWARD);
  
  ledTime = millis();
  ledVal = 0;
  fadeAmount = 1;
}

void displayLED()
{
  Serial.println(ledVal);
  if (millis() > ledTime)
  {
    dimLed();
    ledTime = millis() + 5;
  }
    analogWrite(LED_PIN, ledVal);

}

void dimLed()
{
  ledVal += fadeAmount;
  if (ledVal == 0 || ledVal == 255)
    fadeAmount = -fadeAmount;
}

void setSpeed(int motor, int motorSpeed, boolean isBackward)
{
  Servo *s = (motor == R_MOTOR) ? &rightMotor : &leftMotor;
  
  if ((motor == R_MOTOR) ^ (isBackward))
  {
    motorSpeed = -motorSpeed;
  }
  
  s->write(90 + motorSpeed);
}

void loop()
{
  alUpdate = al.update();
  arUpdate = ar.update();
  (*state.fn)();
   displayLED();
}
