

#include <Pololu3piPlus32U4.h>

#define FORWARD_CHAR  'f'
#define BACKWARD_CHAR 'b'
#define LEFT_CHAR     'l'
#define RIGHT_CHAR    'r'
#define AUTO_CHAR     'a'

#define DEFAULT_SPEED 50

LineSensor lineSensor;
Motors motor;
Buzzer buzzer;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  buzzer.playMode(PLAY_CHECK);
}

void loop() {
  static bool musicFlag = false;

  // put your main code h
  switch (getNextChar()) {
    case FORWARD_CHAR:
      // Set motors forward
      motor.setSpeed(DEFAULT_SPEED, DEFUALT_SPEED);
      break;
    
    case BACKWARD_CHAR:
      motor.setSpeed(-DEFAULT_SPEED, -DEFUALT_SPEED);
      break;

    case LEFT_CHAR:
      motor.setSpeed(-DEFAULT_SPEED, DEFUALT_SPEED);
      break;

    case RIGHT_CHAR:
      motor.setSpeed(DEFAULT_SPEED, -DEFUALT_SPEED);
      break;

    case ANOY_D_BOY_CHAR:
      musicFlag = true;
      break;

    case AUTO_CHAR:
      // Follow line (Do later)
      break;

    default:
      motor.setSpeed(0,0);
      break;
  }

  // Play next part of music
  // If finished set musicFlag to False
}

char getNextChar(){
  // Check if anything is in the serial buffer
  if (Serial.avalible()) {
    return Serial.read();
  }
}

