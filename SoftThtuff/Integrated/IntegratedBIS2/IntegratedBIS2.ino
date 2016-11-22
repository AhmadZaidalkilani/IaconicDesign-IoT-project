/*
 * 
 * Contributers:  Mustafa Hammood   mustafa.sh@alumni.ubc.ca
 * 
 * Revision;      22 Nov 2016
 * 
 * Bugs:          None
 * 
 * Add:           Sliding control for SoftPot
 * 
 */

#include <Button.h>             // improved button by Andrew Mascolo 
#include <SoftwareSerial.h>     // Serial library to initiate Bluetooth comm protocol


// delay in between SoftPot pin ADC measurements (in milliseconds)
#define SOFTPOT_DELAY 50.0


// Bluetooth protocol defines
SoftwareSerial Genotronex(10, 11);    // RX, TX
int BluetoothData;                    // the data given from Computer
int ledpin=13;                        // Red LED on pin D13 for debugging --REMOVE


// LOW = milliseconds, HIGH = microseconds, default is LOW
Button B(LOW);

// Interface buttons
int UpButtonPin = A5;
int DownButtonPin = A3;

// Analog signal filtered by an RC filter (0-5V) to drive LED bar intensity signal
int LEDdriver = 3;


// Analog signal filtered by an RC filter (0-5V) to drive LED bar intensity level
int intensity;
+
// Analog signal filtered by an RC filter (0-5V) to drive power module intensity signal
int intensityPower;

// SoftPot Sensor logic
int SoftPotPin = A4;        // Bias input put (Analog 4)
int SoftPotVal;             // First ADC reading 
int SoftPotValPrev;         // Second ADC reading
int SoftPotValPrevPrev;     // Third ADC reading
float SoftPotSlope1;        // Slope between First and Second ADC reading (SOFTPOT_DELAY interval between the readings)
float SoftPotSlope2;        // Slope between Second and Third ADC reading (SOFTPOT_DELAY interval between the readings)


// LED bar driving functionalities
void flipLevelLED( void );  // Check if LED bar is at 100% or 0% and flip its level
void rampUp( void );        // Ramp up the LED bar intensity from 0% to 100%
void rampDown( void );      // Ramp down the LED bar intensity from 100% to 0%


// Interface control and sensing logic
void ButtonsLogic( void );  // Read buttons logic and control intensity
void SoftPotLogic( void );  // Read SoftPot logic and control intensity\

// Bluetooth communication control
void BluetoothLogic( void );

// the setup function runs once when you press reset or power the board
void setup() {

  // Intiate Bluetooth comm protocol (MCU-Bluetooth-MCU)
  Genotronex.begin(9600);
  Genotronex.println("Bluetooth, input 1 or 0 to control Debug LED and intensity");

  // Define debug LED pin as output --REMOVE
  pinMode(ledpin,OUTPUT);


  // Initiate serial comm protocol (MCU-PC)
  Serial.begin(115200);

  // Define output and input pins
  pinMode(LEDdriver, OUTPUT);
  pinMode(UpButtonPin, INPUT_PULLUP);
  pinMode(DownButtonPin, INPUT_PULLUP);

  // Set what the button will be when pressed (default is HIGH)
  // and set the hold time (default is 500)
  B.SetStateAndTime(LOW, 100);
}

// the loop function runs over and over again forever
void loop() {

  BluetoothLogic();
    
  ButtonsLogic();
  SoftPotLogic();
}

void BluetoothLogic( void )
{

  // put your main code here, to run repeatedly:
  if (Genotronex.available()){
    BluetoothData=Genotronex.read();
    
    if(BluetoothData=='1')
    {    // if number 1 pressed ....
        if( intensity == 100 )
        {
          rampDown();
          intensity = 0;
        }
        else
        {
          rampUp();
          intensity = 100;
        }      
      digitalWrite(ledpin,1);
      Genotronex.println("LED  On D13 ON ! ");
    }
    
    
    if (BluetoothData=='0')
    {    // if number 0 pressed ....
        if( intensity == 100 )
        {
          rampDown();
          intensity = 0;
        }
        else
        {
          rampUp();
          intensity = 100;
        }
      digitalWrite(ledpin,0);
      Genotronex.println("LED  On D13 Off ! ");
    }
  }
  delay(10);   // prepare for next data ...

}
// Function to execute the touch sensor functionality (sliding and clicking)
void SoftPotLogic( void )
{

  /*
  else if( SoftPotVal > 100 )
    analogWrite( LEDdriver, (SoftPotVal-100)/5.6);
  */
  SoftPotVal = analogRead( SoftPotPin );

  if( SoftPotVal != 0 )
  {
    SoftPotValPrev = SoftPotVal;
    delay( SOFTPOT_DELAY );
    SoftPotVal = analogRead( SoftPotPin );

    SoftPotSlope1 = ( SoftPotVal - SoftPotValPrev ) / SOFTPOT_DELAY;

    // User could have possibly on pressed or sliding down
    if( SoftPotSlope1 < -0.2 )
    {
      
      // If current reading is zero, user must've only pressed
      if( SoftPotVal == 0 )
      {
        if( intensity == 100 )
        {
          rampDown();
          intensity = 0;
        }
        else
        {
          rampUp();
          intensity = 100;
        }
      }
          
    }
    else if( SoftPotSlope1 > 0.2 )
    {
      
    }
   else
   {
    SoftPotValPrev = 0;
    SoftPotValPrevPrev = 0;
   }
    
  }
  
  Serial.print("SoftPotValPrevPrev: ");
  Serial.println( SoftPotValPrevPrev );

  Serial.print("SoftPotValPrev: ");
  Serial.println( SoftPotValPrev );

  Serial.print("SoftPotVal: ");
  Serial.println( SoftPotVal );

  Serial.print("Slope1: ");
  Serial.println( SoftPotSlope1);

  Serial.print("Slope2: ");
  Serial.println( SoftPotSlope2);
}


void SoftPotLogicOkayish( void )
{

  /*
  else if( SoftPotVal > 100 )
    analogWrite( LEDdriver, (SoftPotVal-100)/5.6);
  */
  
  SoftPotValPrevPrev = analogRead( SoftPotPin );
  delay( 50 );
  SoftPotValPrev = analogRead( SoftPotPin );

  SoftPotSlope1 = ( SoftPotValPrev - SoftPotValPrevPrev ) / 50.0;
  
  delay( 50 );
  SoftPotVal = analogRead( SoftPotPin );

  SoftPotSlope2 = ( SoftPotVal - SoftPotValPrev ) / 50.0;
  
  
  if( ( SoftPotSlope1 >= 0.02  || SoftPotSlope2 >= 0.02 ) && SoftPotValPrevPrev == 0 )
  {
    flipLevelLED();
  }

  Serial.print("SoftPotValPrevPrev: ");
  Serial.println( SoftPotValPrevPrev );

  Serial.print("SoftPotValPrev: ");
  Serial.println( SoftPotValPrev );

  Serial.print("SoftPotVal: ");
  Serial.println( SoftPotVal );

  Serial.print("Slope1: ");
  Serial.println( SoftPotSlope1);

  Serial.print("Slope2: ");
  Serial.println( SoftPotSlope2);
}


void ButtonsLogic( void )
{

  byte UpButton = B.checkButton(UpButtonPin);
  byte DownButton = B.checkButton(DownButtonPin);
  

  if (UpButton) // if UpButton is anything but 0, it is true
  {
    switch (UpButton)
    {
      case PRESSED:
        Serial.print("ButtonUp was Pressed ");
        flipLevelLED();
        break;
        
      case HELD:
        Serial.print("ButtonUp is Held:");
        break;
        
      case RELEASED:
        Serial.print("ButtonUp was Released ");
        break;
      default: break;
    }
    Serial.println(B.GetHeldTime(SECONDS));
  }
  
  else if (DownButton) // if DownButton is anything but 0, it is true
  {
    switch (DownButton)
    {
      case PRESSED:
        Serial.print("ButtonDown was Pressed ");
        flipLevelLED();
        break;
      case HELD:
        Serial.print("ButtonDown is Held:");
        break;
      case RELEASED:
        Serial.print("ButtonDown was Released ");
        break;
      default: break;
    }
    Serial.println(B.GetHeldTime(SECONDS));
  }
  
}
void rampUp( void )
{
  int rate = 0;
  int maximum = 180;
  int rampDelay = 5;
  
  for( rate = 0; rate < maximum; rate++ )
  {
    analogWrite( LEDdriver, rate );
    delay(rampDelay);
  }

  intensity = rate;
}

void rampDown( void )
{
  int rate = 180;
  int maximum = 0;
  int rampDelay = 5;
  
  while( rate > maximum )
  {
    analogWrite( LEDdriver, rate );
    rate--;
    delay(rampDelay);
  }
  
}

void flipLevelLED( void )
{
  if( intensity == 100 )
  {
    rampDown();
    intensity = 0;
  }
  else
  {
    rampUp();
    intensity = 100;
  }
}

