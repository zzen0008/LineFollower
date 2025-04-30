/*
   Updated 3pi+ code for serial communication using bitstrings instead of characters.
   The robot uses only the "standard" configuration (no LCD). It accepts commands as binary 
   packets (bitstrings) with the following command codes:
     
     0x01 - STOP           -> Stops line following (motors are stopped).
     0x02 - GO             -> Enables line following.
     0x03 - CALIBRATE      -> Runs the sensor calibration routine.
     0x04 - SET_SPEED      -> Followed by two bytes (big-endian) that set baseSpeed.
     0x05 - INFO           -> Returns current info on line position and error.
     
   All values in the range 0x10-0x1F are reserved as placeholders for future commands.
*/
#include <Pololu3piPlus32U4.h>
#include <Arduino.h>
using namespace Pololu3piPlus32U4;
  
// Global object declarations:
Buzzer buzzer;
LineSensors lineSensors;
Motors motors;
  
// Constants and variables for line following:
int16_t lastError = 0;
#define NUM_SENSORS 5
unsigned int lineSensorValues[NUM_SENSORS];
  
// Speed and PID constants – defined for the standard configuration:
uint16_t maxSpeed;
int16_t minSpeed;
uint16_t baseSpeed;
uint16_t calibrationSpeed;
  
// PID configuration: these constants are multiplied by 256 so that a P coefficient
// of 1/4 is stored as 64 and a D coefficient of 1 is stored as 256.
uint16_t proportional; // Coefficient for the proportional term (P*256)
uint16_t derivative;   // Coefficient for the derivative term (D*256)
  
// State flag: when false the robot does not run its line-following controller.
bool lineFollowingActive = true;
  
// Calibrates the sensors by rotating the robot so that the sensors see both dark and light.
void calibrateSensors()
{
  // Short delay before calibration starts.
  delay(1000);
  for(uint16_t i = 0; i < 80; i++)
  {
    // Alternate rotation direction to properly sweep the sensor array over the line.
    if (i > 20 && i <= 60)
      motors.setSpeeds(-((int16_t)calibrationSpeed), calibrationSpeed);
    else
      motors.setSpeeds(calibrationSpeed, -((int16_t)calibrationSpeed));
      
    lineSensors.calibrate();
  }
  motors.setSpeeds(0, 0);
}
  
// Process a received command given as a byte.
// For commands with parameters (like SET_SPEED) the function waits for the needed extra bytes.
void processCommandByte(uint8_t command)
{
  switch(command)
  {
    case 0x01:  // STOP command
      lineFollowingActive = false;
      motors.setSpeeds(0,0);
      // Send an acknowledgement (for example, 0x81 means ACK for STOP)
      Serial.write(0x81);
      break;
      
    case 0x02:  // GO command: enable line following.
      lineFollowingActive = true;
      Serial.write(0x82);
      break;
      
    case 0x03:  // CALIBRATE command
      Serial.write(0x83); // Acknowledgement: beginning calibration.
      calibrateSensors();
      Serial.write(0x84); // Calibration complete
      break;
      
    case 0x04:  // SET_SPEED command - expect 2 extra bytes (big-endian speed value)
      {
        // Wait until two more bytes are available.
        while(Serial.available() < 2) { /* wait */ }
        uint8_t high = Serial.read();
        uint8_t low = Serial.read();
        int16_t newSpeed = ((int16_t)high << 8) | low;
        baseSpeed = newSpeed;
        Serial.write(0x85); // ACK for speed set.
        // Optionally, send the new speed back as confirmation:
        Serial.write(high);
        Serial.write(low);
      }
      break;
      
    case 0x05:  // INFO command: return sensor info.
      {
        // Read the current line position
        uint16_t position = lineSensors.readLineBlack(lineSensorValues);
        // Compute the error from an ideal center of 2000.
        int16_t error = position - 2000;
        // Format: header 0x86 followed by two bytes (big endian) for position and two bytes for error.
        Serial.write(0x86);
        Serial.write((byte)(position >> 8));
        Serial.write((byte)(position & 0xFF));
        Serial.write((byte)(error >> 8));
        Serial.write((byte)(error & 0xFF));
      }
      break;
      
    // Placeholder for additional commands: 0x10-0x1F are reserved for future expansion.
    default:
      if(command >= 0x10 && command <= 0x1F)
      {
        // For now, simply send back a reserved command acknowledgement.
        Serial.write(0xF0);
      }
      else
      {
        // If an unknown command is received, send an error byte.
        Serial.write(0xFF);
      }
      break;
  }
}
  
// setup() runs one time at startup.
void setup()
{
  // Start the serial port with a baud rate of 9600.
  Serial.begin(9600);
  // Delay briefly so that the external connection can be established.
  delay(1000);
  
  // (Optional) Play a welcome tune.
  buzzer.play(">g32>>c32");
  
  // Use the standard configuration.
  maxSpeed = 200;
  minSpeed = 0;
  baseSpeed = maxSpeed;
  calibrationSpeed = 60;
  proportional = 64;  // 1/4 (64/256)
  derivative = 256;   // 1   (256/256)
  
  // Calibrate the sensors on startup.
  Serial.write("Calibrating sensors...\n");
  calibrateSensors();
  Serial.write("Calibration complete. Send command 0x02 to enable line following.\n");
}
  
// loop() runs repeatedly.
void loop()
{
  // --- Check for incoming serial commands ---
  // If data is available, read one command byte and process it.
  if(Serial.available() > 0)
  {
    uint8_t cmd = Serial.read();
    processCommandByte(cmd);
  }
  
  // --- Run the line following controller if enabled ---
  if(lineFollowingActive)
  {
    // 1. Read the line sensors. The function readLineBlack returns a value from 0 to 4000.
    int16_t position = lineSensors.readLineBlack(lineSensorValues);
    
    // 2. Compute the error based on a desired center position of 2000.
    int16_t error = position - 2000;
    
    // 3. Calculate the motor speed adjustment using proportional and derivative terms.
    //    The formula scales the error parameters (multiplied by 256) and divides by 256.
    int16_t speedDifference = (int32_t)error * proportional / 256 +
                              (int32_t)(error - lastError) * derivative / 256;
    lastError = error;
    
    // 4. Compute individual motor speeds by adjusting the base speed.
    int16_t leftSpeed = baseSpeed + speedDifference;
    int16_t rightSpeed = baseSpeed - speedDifference;
    
    // 5. Constrain motor speeds to the range [minSpeed, maxSpeed].
    leftSpeed = constrain(leftSpeed, minSpeed, (int16_t)maxSpeed);
    rightSpeed = constrain(rightSpeed, minSpeed, (int16_t)maxSpeed);
    
    // 6. Command the motors with these speeds.
    motors.setSpeeds(leftSpeed, rightSpeed);
  }
  
  // A small delay to avoid saturating CPU and serial resources.
  delay(10);
}