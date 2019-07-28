// Include the Servo library 
#include <Servo.h> 
// Declare the Servo pin 
int servoPin = 6; 
int pin = 7;
int val = 0;
// Create a servo object 
Servo Servo1; 
void setup() { 
   // We need to attach the servo to the used pin number 
   Servo1.attach(servoPin);
   pinMode(pin,INPUT) ;
}
void loop(){ 

  val = digitalRead(pin); 
  if(val==HIGH){
   // Make servo go to 0 degrees 
   Servo1.write(-5); 
   delay(1000); 
   // Make servo go to 90 degrees  c
   Servo1.write(100); 
   delay(5000); 
  Servo1.write(0); 
   delay(1000); 
   
   // Make servo go to 180 degrees 
    }
}
