/*
 * 
 * Contributers:  Mustafa Hammood   mustafa.sh@alumni.ubc.ca
 * 
 * Revision;      26 Nov 2016
 * 
 * Bugs:          None
 * 
 * Add:           Sliding control for SoftPot
 * 
 */

#include <Button.h>             // improved button by Andrew Mascolo 
#include <SoftwareSerial.h>     // Serial library to initiate Bluetooth comm protocol


// Delay in between SoftPot pin ADC measurements (milliseconds)
#define SOFTPOT_DELAY 50.0

// Duration of backlight LEDs after detecting motion (milliseconds)
#define BACKLIGHT_TIME 5000.0

// Bluetooth protocol defines
SoftwareSerial Genotronex(10, 11);    // RX, TX
int BluetoothData;                    // the data given from Computer
int ledpin=13;                        // Red LED on pin D13 for debugging --REMOVE


// New SoftPot logic function
int UpperVar = 0;
int LowerVar = 1023;
int LEDrate = 0;
int LEDrate1 = 0;
int trueLEDrate = 0;
int incrementer = 0;
int MeanTing;
float var1,var2,var3;
int InitialMeanTing;
int SoftPotValArray[] = {0,0,0,0,0,0,0,0,0,0};
boolean SlidingTrigger;
boolean ThisTrigger = false;


// LOW = milliseconds, HIGH = microseconds, default is LOW
Button B(LOW);

// Interface buttons
int UpButtonPin = A5;
int DownButtonPin = A3;

// Analog signal filtered by an RC filter (0-5V) to drive LED bar intensity signal
int LEDdriver = 3;


// Sensors
int MotionSensorPin = A2;
int MotionSensor;
int CurrentSampler = A1;


// Backlight LED Logic
int BackLightON;
unsigned long BackLightTimer;

// Analog signal filtered by an RC filter (0-5V) to drive LED bar intensity level
int intensity;

// Analog signal filtered by an RC filter (0-5V) to drive power module intensity signal
int intensityPower;
int intensityPowerPWM;
int intensityPowerPin = 9;

// SoftPot Sensor logic
int SoftPotPin = A4;        // Bias input put (Analog 4)
boolean SoftPotToggle = false;
int SoftPotVal;             // First ADC reading 
int SoftPotValPrev;         // Second ADC reading
int SoftPotValPrevPrev;     // Third ADC reading
float SoftPotSlope1;        // Slope between First and Second ADC reading (SOFTPOT_DELAY interval between the readings)
float SoftPotSlope2;        // Slope between Second and Third ADC reading (SOFTPOT_DELAY interval between the readings)


// Power
int InstantCurrent;

// LED bar driving functionalities
void flipLevelLED( void );  // Check if LED bar is at 100% or 0% and flip its level
void rampUp( void );        // Ramp up the LED bar intensity from 0% to 100%
void rampDown( void );      // Ramp down the LED bar intensity from 100% to 0%


// Interface control and sensing logic
void ButtonsLogic( void );  // Read buttons logic and control intensity
void SoftPotLogic( void );  // Read SoftPot logic and control intensity\

// Bluetooth communication control
void BluetoothLogic( void );

// Control power circuit light intensity signal
void BulbIntensity( void );

// Backlight LEDs logic

void BacklightLogic( void );

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
  pinMode(LEDdriver, OUTPUT);               // Control LED driver IC (analog output to RC filter)
  pinMode(UpButtonPin, INPUT_PULLUP);
  pinMode(DownButtonPin, INPUT_PULLUP);
  pinMode(MotionSensorPin, INPUT);
  pinMode(intensityPowerPin, OUTPUT);
  
  // Set what the button will be when pressed (default is HIGH)
  // and set the hold time (default is 500)
  B.SetStateAndTime(LOW, 100);
}

// the loop function runs over and over again forever
void loop() {
  
  BluetoothLogic();
  SoftPotLogic();
  BulbIntensity();


  // Disconnected features
  
  //BacklightLogic();
  //ButtonsLogic();

}


void BulbIntensity( void )
{
  intensityPower = map( trueLEDrate, 0, 180, 20, 200 );
  analogWrite( intensityPowerPin, intensityPower );
  
  InstantCurrent = analogRead(CurrentSampler);
  Serial.print("InstantCurrent" );
  Serial.println(InstantCurrent);

}

void BacklightLogic( void )
{
  MotionSensor = analogRead( MotionSensorPin );

  // If signal is low then motion is detected
  if( MotionSensor < 500 )
  {
    digitalWrite(ledpin,1);

    if( BackLightON == 0 )
      rampUp();

    BackLightTimer = millis();
    BackLightON = 1;
  }
  else
  {
    digitalWrite(ledpin,0);
  
    if( BackLightON == 1 && millis() >= BackLightTimer+BACKLIGHT_TIME )
    {
      BackLightON = 0;
      rampDown();  
    }
  } 
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


  // command that reads the voltage corresponding to where the user is pressing at that moment in time
   SoftPotVal = analogRead( SoftPotPin );
   /*
  Serial.print("SoftPotVal" );
  Serial.println(SoftPotVal);
  Serial.print("MeanTing");
  Serial.println(MeanTing);
  Serial.print("initialMeanTing" );
  Serial.println(InitialMeanTing );
  Serial.print("SlidingTrigger");
  Serial.println(SlidingTrigger);
  Serial.print("UpperVar");
  Serial.println(UpperVar);
  Serial.print("LowerVar");
  Serial.println(LowerVar);
  Serial.print("Array");
  Serial.println(SoftPotValArray[0]);
  Serial.print("LEDrate");
  Serial.println(LEDrate);
  Serial.print("LEDrate1");
  Serial.println(LEDrate1);
  Serial.println();  */


  // these two if statements decide which LEDrate should be used to write to the LED driver (this will make more sense once you
  // see what's going on in the code later
  if (ThisTrigger){
     analogWrite( LEDdriver,  LEDrate1);
     trueLEDrate = LEDrate1;
  }
  if (!ThisTrigger){
     analogWrite( LEDdriver,  LEDrate);
     trueLEDrate = LEDrate;
  }

  // this for loop basically adds the new sampled SoftPotVal into the array holding the last 10 sampled SoftPotVal values
    for(int x = 9; x > 0; x--){
        SoftPotValArray[x] = SoftPotValArray[x-1];
    }
    SoftPotValArray[0] = SoftPotVal; 


  // here are a big group of nested if statements.  This is where the core of the logic is.
  // First the big if statements basically checks if the user is applying pressure to the switch
  // i.e. if the user is interacting with the switch
  if (SoftPotVal > 5) {

  // we want ThisTrigger to be turned on only in a specific nested if statements, otherwise it should always
  // be false, so here we just make sure it's false.
    ThisTrigger = false;

  // This incrementer keeps track of how many loops have passed since the user began interacting with the switch 
    incrementer++;


  // reset the values for LowerVar and UpperVar so that their values can be recalculated based on the newly updated array
  // of 10 samples
    LowerVar = 1023;
    UpperVar = 0;

  // the for loop that does the updating of the 2 variables based on the newly updated array of 10 samples
    for(int i = 0; i < 10; i++){
      if (SoftPotValArray[i] > UpperVar){
        UpperVar = SoftPotValArray[i];
      }
      if (SoftPotValArray[i] < LowerVar){
        LowerVar = SoftPotValArray[i];
      }
    }


  // at this point all 10 samples have been collected in the array in order to make sense from an "average"
    if (incrementer > 9){

      // This will be the first calculated average which is a special one, because this average tells us where the user
      // applied pressure on the switch for the first time or instance
      if (incrementer == 10){
       InitialMeanTing = (UpperVar + LowerVar)/2;
    }

    // now after 10 increments, we'll keep calculating the average and comparing it to the initial calculated average
      if (incrementer > 10){
       MeanTing = (UpperVar + LowerVar)/2;

    // if the MeanTing average is equal to the InitialMeanTing average (within a certain tolerance), then that means the
    // user clicked the switch, not slid across it.  Otherwise, the user slid across. 
       if (MeanTing > InitialMeanTing+10 || MeanTing < InitialMeanTing-10 || SlidingTrigger == true){


            var1 = MeanTing - InitialMeanTing;
            var2 = var1*180;
            var3 = var2/1023;
            LEDrate1 = (int)LEDrate + var3;  

        // this trigger becomes true because now the variable LEDrate1 is the variable that holds the true value that corresponds
        // to the position of the user's finger.  As long as this is happening, we must keep writing LEDrate1 to the LED driver.
        // To do that, we would need this trigger to tell us that it's time to do that.
        ThisTrigger = true;

        // The fact that we're in this place of the code means that the user triggered the if statements checking if the user slid
        // their finger.  With that being said, SlidingTrigger should be true at this point
        SlidingTrigger = true;

            
        // this code makes sure that the variables don't contain values outside of their meant range.
            if (LEDrate1 > 180) {
              LEDrate1 = 180;
            }
            if (LEDrate1 < 0 ){
              LEDrate1 = 0;
            }

        // this toggle variable corresponds to whether or not the lights are on.  If the lights are partially on, then technically
        // they're still on.  Which means this variable would be true.  Here, if the user swipes
        // the light switch all the way down to zero, this variable should become false
        if (LEDrate1 == 0){
        SoftPotToggle = false;
        }
        if (LEDrate1 != 0){
          SoftPotToggle = true;
        }

       }
       else {
        // if you're at this point of the code, that means the if statements decided that the initial average is approximately equal to
        // the actual average, which means it executed this part of the code, which means the user clicked, so the sliding trigger would be off
       SlidingTrigger = false;
      }
  } 
    
  }


   // this is the "else" part of the "big" if statement, you get here then the user stopped interacting with the switch.
   // So at this point,  we there are 2 different things to be come based on whether or not the user clicked or swiped
   // in that elapsed session.
  } else if (SoftPotVal <= 5){
    UpperVar = 0;
    LowerVar = 1023;
    ThisTrigger = false;
    LEDrate = LEDrate1;
    InitialMeanTing = 0;
    MeanTing = 0;

  // if the user released before the completion of 10 cycles, we just assume that the user clicked.  Otherwise, we know
  // that the user clicked if the SlidingTrigger is false
 if (!SlidingTrigger || (incrementer > 0 && incrementer < 9)){
    ThisTrigger = false;
    // here you turn all the LEDs off if they're supposed to have been on (you'd know this based on the SoftPotToggle variable)
    if (SoftPotToggle){
      rampDown();
      LEDrate1 = 0;
      // here you toggle this variable because now they've been turned on, so you have to update the variable
      SoftPotToggle = !SoftPotToggle;
      SlidingTrigger = true;
    }
    // here you turn all the LEDs on if they're supposed to have been off.
    else if (!SoftPotToggle){
      rampUp();
      LEDrate1 = 180;
      SoftPotToggle = !SoftPotToggle;
      SlidingTrigger = true;
    }
   }
   // reset the incrementer because the user released the switch, so we need the incrementer to be null in order to use it in the
   // next session (if there is one)
       incrementer = 0;
  }



  /*Serial.println(SoftPotVal);
  if( SoftPotVal != 0 )
  {
    SoftPotValPrev = SoftPotVal;
    delay( SOFTPOT_DELAY );
    SoftPotVal = analogRead( SoftPotPin );
    SoftPotSlope1 = ( SoftPotVal - SoftPotValPrev ) / SOFTPOT_DELAY;
    // User could have possibly on pressed or sliding down
    if( abs(SoftPotSlope1) > 0.2 ){
      if (!SoftPotToggle){
         rampUp();
         SoftPotToggle = !SoftPotToggle;
      }
      else{
        rampDown();
         SoftPotToggle = !SoftPotToggle;
      }
      
    }
   
}*/
}


// Old softPot logic function, unused at the moment
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
  int maximum = 180;
  int rampDelay = 5;
  
  while( LEDrate < maximum)
  {
    analogWrite( LEDdriver, LEDrate );
    LEDrate++;
    delay(rampDelay);
  }

}

void rampDown( void )
{
  int maximum = 0;
  int rampDelay = 5;
  
  while( LEDrate > maximum )
  {
    analogWrite( LEDdriver, LEDrate );
    LEDrate--;
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

