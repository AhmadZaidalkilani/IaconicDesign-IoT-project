#include <Button.h>
#include <SoftwareSerial.h>    // import the serial library

#define SOFTPOT_DELAY 50.0



SoftwareSerial Genotronex(10, 11);  // RX, TX
int BluetoothData;      // the data given from Computer
int ledpin=13;        // led on D13 will show blink on / off

Button B(LOW);

int UpButtonPin = A5;
int DownButtonPin = A3;

int LEDdriver = 3;
int intensity;

int SoftPotPin = A4;
int SoftPotVal;
int SoftPotValPrev;
int SoftPotValPrevPrev;
float SoftPotSlope1;
float SoftPotSlope2;

void rampUp( void );
void rampDown( void );
void ButtonsLogic( void );
void SoftPotLogic( void );
void BluetoothLogic( void );

// the setup function runs once when you press reset or power the board
void setup() {
  Genotronex.begin(9600);
  Genotronex.println("Bluetooth On please press 1 or 0 blink LED..");
  pinMode(ledpin,OUTPUT);
  
  Serial.begin(115200);

  pinMode(LEDdriver, OUTPUT);
  pinMode(UpButtonPin, INPUT_PULLUP);
  pinMode(DownButtonPin, INPUT_PULLUP);

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



/*
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
*/
