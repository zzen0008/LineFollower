

#include <Pololu3piPlus32U4.h>

const char sheet[] PROGMEM =
	"v9 egre-2ddgfed#";

#define FORWARD_CHAR      'f'
#define BACKWARD_CHAR     'b'
#define LEFT_CHAR         'l'
#define RIGHT_CHAR        'r'
#define AUTO_CHAR         'a'
#define ANOY_D_BOY_CHAR   'm'
#define ENABLE_TANK_CHAR  't'

#define DEFAULT_SPEED 50

#define ECHO_DEBUG true

using namespace Pololu3piPlus32U4;

LineSensors lineSensor;
Motors motor;
Buzzer buzzer;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);


  Serial.write("why was 8 scared of 7?");

  while (ECHO_DEBUG) {
    echoServer();
  }
}

void loop() {
  static bool musicFlag = false;

  // put your main code h
  switch (getNextChar()) {

    case ENABLE_TANK_CHAR:
      tankMode = true;
      int x = Serial.parseInt(SKIP_WHITESPACE);
      int y = Serial.parseInt(SKIP_WHITESPACE);
      motor.setSpeeds(x, y);
      break;

    case FORWARD_CHAR:
      // Set motors forward
      motor.setSpeeds(DEFAULT_SPEED, DEFAULT_SPEED);
      break;
    
    case BACKWARD_CHAR:
      motor.setSpeeds(-DEFAULT_SPEED, -DEFAULT_SPEED);
      break;

    case LEFT_CHAR:
      motor.setSpeeds(-DEFAULT_SPEED, DEFAULT_SPEED);
      break;

    case RIGHT_CHAR:
      motor.setSpeeds(DEFAULT_SPEED, -DEFAULT_SPEED);
      break;

    case ANOY_D_BOY_CHAR:
      buzzer.playFromProgramSpace(sheet);
      break;

    case AUTO_CHAR:
      // Follow line (Do later)
      break;

    default:
      motor.setSpeeds(0,0);
      break;
  }

  // Play next part of music
  // If finished set musicFlag to False

}

char getNextChar(){
  // Check if anything is in the serial buffer
  if (Serial.available()) {
    return Serial.read();
  }
  return " ";
}

void echoServer(){
  if (Serial.available()) {
    Serial.print("You Sent: ");
    Serial.write(Serial.read());
    Serial.print("\n\r");
  }
  return;
}

