/*****************************************************************************     Libraries     ******************************************************************************/
#include <SPI.h>                                // OLED Display Library
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono9pt7b.h>

#include <INA226.h>                             // Using Zanshin INA226 Library ! Zanshin FTW !
/******************************************************************************************************************************************************************************/


/******************************************************************************     PINOUT     ********************************************************************************/
#define    bluetoothLED      A0

#define    AUX_IN_detector   8 

#define    ampStillON        7
#define    ampRelay          10

#define    chargeRelay       11

#define    bluetoothBuck     12

#define    RED_LED           4  
#define    GREEN_LED         5  
#define    BLUE_LED          6  
/******************************************************************************************************************************************************************************/


/*****************************************************************************     Display     ********************************************************************************/
#define OLED_RESET 4
Adafruit_SSD1306 OLED(OLED_RESET);              // Construct an object named "OLED" of type Adafruit_SSD1306. Used to control our display.
/******************************************************************************************************************************************************************************/


/******************************************************************** ********   Power Monitor   ******************************************************************************/
INA226_Class INA226;                            // Construct a power monitor object names "ina"
/******************************************************************************************************************************************************************************/




/* void loop()                                                           
 *                                                             
 * Calls all methods involved in running the show, forever.                                                              
 * 
 * Sets   theTime                   
 */    

   /*********     GLOBAL VARIABLES     *********/
   uint32_t theTime  = 0;
   uint8_t  ampState = 0;
   uint8_t  pwrState = 0;

   /*********    "LOCAL" VARIABLES     *********/
   bool     firstStrike1   = 0;
   uint32_t t_firstStrike1 = 0;

   bool     firstStrike2   = 0;
   uint32_t t_firstStrike2 = 0;

   bool previousChargeEnabledState = 0;




/* void signalAquisitionModule()
 *  
 * Reads and formats all 3 inputs
 *                                                           
 * Sets   state_BTModule
 *        state_AUX_IN
 *        state_EXTERNAL_POWER
 *        state_BATT_ONLINE
 */                                                                                                                                                                                                  

   /*********     GLOBAL VARIABLES     *********/
   uint8_t  state_BTModule       = 0 ;            // ==0 if BT off             ==1 if BT searching          ==2 if BT connected & paused   ==3 if BT connected & playing
   bool     state_AUX_IN         = 0 ;            // ==0 if no AUX             ==1 if AUX connected
   bool     state_EXTERNAL_POWER = 0 ;            // ==0 if running on batt.   ==1 if mains power available
   bool     state_BATT_ONLINE    = 0 ;            // ==0 if battery attached   ==1 if system running on AC

   /*********     "LOCAL" VARIABLES    *********/
   bool     state_BTLED          = 0 ;
   uint32_t t_FirstOnBT          = 0 ;
   uint16_t t_TotalOnBT          = 0 ;

   uint32_t t_lastAUXcheck       = 0 ;




/* bool newPowerMonitorReadings()
 * 
 * Reads INA226 Power Monitor Module and stores results in global variables
 * Updates values every .5s. Return true when new variables are available
 * 
 * Sets   avgCurrent
 *        avgVoltage
 *        avgWatts
 *        negativeCurrent
 *        lowBattery
 */
 
   /*********     GLOBAL VARIABLES     *********/
   int32_t  avgCurrent   = 0;                      // Microamps  [-820 000 ,    820 000]
   uint16_t avgVoltage   = 0;                      // Millivolts [       0 ,     26 000]
   int32_t  avgWatts     = 0;                      // Microwatts [       0 , 21 320 000]
   bool     negativeAmps = 0;
   bool     lowBattery   = 0;

   /*********     "LOCAL" VARIABLES    *********/
   int32_t  sumCurrent = 0;
   uint32_t sumVoltage = 0;
   int32_t  sumWatts   = 0;

   uint32_t lastPowerMeasurement = 0;
   uint16_t readings             = 0;




/* void refreshDisplay()
 * 
 * Draws all the numbers and graphics on the display everytime its called with the current values
 */
 
   /*********     "LOCAL" VARIABLES    *********/
   String current    = String(9);                  // Are used to manipulate read valued and perform a 'lossless' division by 1000
   String voltage    = String(9);
   String wattsORmAh = String(9);
   String temp       = String(9);

   uint16_t mAs        = 0;
   uint16_t mAh        = 0;

   uint16_t constrainedVoltage = 0;
   int16_t currBattLevel       = 0;
   uint16_t prevBattLevel      = 125;
   
   bool battBlinker    = 0;
   



/* String divideBy1000(String)
 * 
 * Divides a String number betw. [1 , 999 999] by 1000 and formats it to always have 3 comma digits
 */

  /*********     "LOCAL" VARIABLES    *********/
  String output       = String(9);




/* turn(char)
 *  
 * Ensures amp stays on for at least 1 minute
 * Ensures amp relay doesn't stick
 * 
 * Ensures bluetooth module stays off for at least 5s
*/

   /*********     GLOBAL VARIABLES     *********/
   uint8_t currentAmpState       = 5;
   uint8_t chargeEnabled         = 5;             
   uint8_t bluetoothON           = 5;

   /*********     "LOCAL" VARIABLES    *********/
   #define ampON               'x'
   #define ampOFF              'y'
   #define chargeON            'z'
   #define chargeOFF           't'
   #define btON                'u'
   #define btOFF               'v'
   
   uint32_t timeAMPswitchedON = 0;
   uint32_t timeBTswitchedOFF = 0;





/* turnLED(char color)
 * 
 * Turns the LED to the desired color (colors defined below)
 */
 
   /*********     "LOCAL" VARIABLES    *********/
   #define    RED                      'r'
   #define    GREEN                    'g'
   #define    BLUE                     'b'
   #define    OFF                      'x'




/* blinkLED(const char color , int onTime , int offTime)
 * 
 * Transparent LED blinker; iff called every 1 ms, the on- and offtime it's being fed will be in ms
 * Feeding in negative values for either onTime or offTime will make the on- or offTime be infinitely long
 */

   /*********     "LOCAL" VARIABLES    *********/
   char previousColor;
   bool redLEDstate   = 0;
   bool greenLEDstate = 0;
   bool blueLEDstate  = 0;

   uint16_t alreadyOnFor  = 0;
   uint16_t alreadyOffFor = 0;
   



/* bluetoothConnectionResuscitator()
 * 
 * Resets Bluetooth Module every 15000 calls to it (~15s iff called every ms)
 */

   /*********     "LOCAL" VARIABLES    *********/
   uint16_t timeAlone = 0;









void setup() {

  //Serial.begin(9600);

  /************************************************************************ Speaker Peripherals *************************************************************************/  
  pinMode(bluetoothLED     , INPUT_PULLUP);                                   // Bluetooth Module LED1   ( == 0 when ON) : blinks rapidly when searching for host / discoverable; slowly when connected
  pinMode(AUX_IN_detector  , INPUT_PULLUP);                                   // AUX input detector      ( == 0 when external jack plugged in - bridges AUDIO_GND to AUX_IN_detector pin)
  pinMode(ampStillON       , INPUT       );                                   // Amps has built-in 5V regulator. We use that to know wether the amp is off or the relay got stuck

  pinMode     (ampRelay    , OUTPUT);
  digitalWrite(ampRelay    , HIGH  );                                         // Equiv to turn(ampOFF);   ---   but that function has a 30s timer for power-off, initially we want it to be off immediately
  pinMode     (chargeRelay , OUTPUT);
  
  pinMode     (RED_LED     , OUTPUT);
  digitalWrite(RED_LED     , HIGH   );
  pinMode     (GREEN_LED   , OUTPUT);
  digitalWrite(GREEN_LED   , HIGH  );
  pinMode     (BLUE_LED    , OUTPUT);
  digitalWrite(BLUE_LED    , HIGH  );


  /************************************************************************* Initialise Switches ************************************************************************/
  turn    (btON);
  turn(chargeON);


  /************************************************************************* Initialise Display *************************************************************************/
  OLED.begin(SSD1306_SWITCHCAPVCC , 0x3C);                                    // initialise Display and set address to 3C  
  OLED.clearDisplay();                                                        // Prevents Adafruit Logo from Appearing : still : thanks Adafruit !!!

  delay(5);

  OLED.setFont(&FreeMono9pt7b);                                               // OCD Friendly Monospaced Font for Alignment Goodness
  OLED.setTextSize(1);
  OLED.setTextColor(WHITE , BLACK);                                           // setTextColor (font color, backgr color)


  /********************************************************************** Initialise Power Monitor **********************************************************************/
  INA226.begin(1,100000);                                                     // Begin calibration for an expected ±1 Amps maximum current and for a 0.1Ohm resistor
  INA226.setAveraging(1);                                                     // Average each reading n-times
  INA226.setBusConversion();                                                  // Maximum conversion time 8.244ms 
  INA226.setShuntConversion();                                                // Maximum conversion time 8.244ms
  INA226.setMode(INA_MODE_CONTINUOUS_BOTH);                                   // Bus/Shunt measured continuously

  delay(5);


  /************************************************************************* Draw Battery Shape *************************************************************************/
  drawBatteryOutline();
  OLED.display();
  delay(200);                                                                 // Somehow the delay makes it cooler


  /************************************************************************* Initialise Variables *************************************************************************/
  
  
}




























   
void loop() {

  delay(1);
  
  theTime = millis();

  signalAquisitionModule();

  if ( newPowerMonitorReadings() ) refreshDisplay();


  switch(ampState) {                                                                // State Machine controlling power to the Bluetooth Module, Amp and the LED

                /*
                 * State 0 : Initial State : Waiting for BT module to boot          [ BT - ON              ; AMP - OFF ]
                 * State 1 : BT Searching / In Pair Mode                            [ BT - ON              ; AMP - OFF ] 
                 * State 2 : BT Connected : Awaiting Music Stream                   [ BT - ON              ; AMP - OFF ]
                 * State 3 : Music Playing from either BT or AUX                    [ BT - ON (OFF if AUX) ; AMP - ON  ]
                 */

                /*
                 * AMP     : OFF
                 * BT      : ON : Booting
                 * LED     : Solid Red
                 */
                case 0: {
                        turn    (ampOFF);
                        turn    (btON);
                        turnLED (RED);

                        bluetoothConnectionResuscitator();                           // Resuscitate (restart) BT module after 15s
                        
                        if      (state_BTModule      == 1)   ampState = 1;
                        else if (state_BTModule      == 2)   ampState = 2;
                        else if (state_BTModule      == 3)   ampState = 3;                                           
                                                           
                        if (state_AUX_IN)                    ampState = 3;
                                            
                        }break;

                /*
                 * AMP     : OFF
                 * BT      : Searching
                 * LED     : Blinking Blue
                 */
                case 1: {
                        turn     (ampOFF);
                        turn     (btON  );
                        
                        if (!lowBattery) blinkLED (BLUE , 200 , 120);

                        bluetoothConnectionResuscitator();                           // Restart BT Module every 15s
                        
                        if      (state_BTModule      == 0)   ampState = 0;
                        else if (state_BTModule      == 2)   ampState = 2;
                        else if (state_BTModule      == 3)   ampState = 3;
                                                           
                        if (state_AUX_IN)                    ampState = 3;
                                                          
                        }break;
                        
                /*
                 * AMP     : OFF
                 * BT      : Connected
                 * LED     : Solid Blue
                 */      
                case 2: {
                        turn    (ampOFF);
                        turn    (btON  );
                        
                        if (!lowBattery) turnLED  (GREEN);
                        
                        if      (state_BTModule      == 0)   ampState = 0;
                        else if (state_BTModule      == 1)   ampState = 1;
                        else if (state_BTModule      == 3)   ampState = 3;
                                                           
                        if (state_AUX_IN)                    ampState = 3;
                                                           
                        }break;
                        
                /*
                 * AMP     : ON
                 * BT      : Playing
                 * LED     : Solid Green
                 */
                case 3: {
                        turn    (ampON);
                        
                        if (!lowBattery) turnLED  (OFF);
                        
                        if (!state_AUX_IN)  {
                                            turn(btON);
                                            
                                            if      (state_BTModule      == 0)   ampState = 0;
                                            else if (state_BTModule      == 1)   ampState = 1;
                                            else if (state_BTModule      == 2)   ampState = 2;
                                            }
                            
                        else turn(btOFF);
                        
                        
                        }break;
  }


  switch(pwrState) {                                                                // State Machine controlling the Change Circuitry. Protects against overcharging and -discharging the battery

                   /*
                    * State 0 : battery IS     being charged
                    * State 1 : battery IS NOT being charged
                    * State 2 : battery is overdischarged - enter low power mode
                    */
    
                   case 0: {
                           turn(chargeON);

                           // 1.  Cond : We don't want to energise this relay when on battery power
                           // 2.  Cond : For when we disconnect battery from system manually (switch on back) and it's plugged in
                           //           Going into chargeOFF mode will mean we never charge battery again untill hard restart
                           // 3.1 Cond : If pack is drawing less than 150mA, we decide that it reached its end of charge (cannot use mA if amp is on and draws current as well
                           // 3.2 Cond : If pack voltage goes over    25.2v, we decide that it reached its end of charge (given variable amp draw, and the fact that the battery will charge very slowly (250-150 = 100mA MAX), this might take a while)
                           
                           if ( negativeAmps           && 
                                (abs(avgCurrent) > 20) && 
                                
                                ( 
                                  ((avgCurrent > -150000) && currentAmpState==0)
                                  ||
                                  ((avgVoltage > 25200  ) && currentAmpState==1) 
                                )    
                                                                                    ) if (!firstStrike1) {
                                                                                                                     firstStrike1 = 1;
                                                                                                                     t_firstStrike1 = theTime;
                                                                                                                     }
                                                                                                   else {
                                                                                                        if ( abs(theTime - t_firstStrike1) > 15000) {
                                                                                                                                                   firstStrike1 = 0;
                                                                                                                                                   firstStrike2 = 0;
                                                                                                                                                   pwrState     = 1;
                                                                                                                                                   }
                                                                                                        }
                           else firstStrike1 = 0;



                           // As batteries age, internal resistance goes up, and at some point they act like resistors, which will mean current will never settle down. Time out after 10 hours
                           if (state_EXTERNAL_POWER && state_BATT_ONLINE) if (!firstStrike2) {
                                                                                                firstStrike2 = 1;
                                                                                                t_firstStrike2 = theTime;
                                                                                                }
                                                                          else {
                                                                               if ( abs(theTime - t_firstStrike2) > 36000000 ) {
                                                                                                                               firstStrike1 = 0;
                                                                                                                               firstStrike2 = 0;
                                                                                                                               pwrState     = 1;
                                                                                                                               }
                                                                               }
                           else firstStrike2 = 0;
                                                                                                                      
                         
                         
                                                       
                           if ( (avgVoltage < 16000) && (state_EXTERNAL_POWER == 0) ) pwrState = 2;
                           
                   }break;


                   case 1: {
                           turn(chargeOFF);

                           // 1. Cond : Once external power no longer present, it doesn't make sense to keep the NO-CHARGE relay energised
                           // 2. Cond : In case we stop charging too soon (ie 10 hours pass and we've listened to music in that time and thus charged the battery at only 100mA), we go back to charging
                           //           In case somehow battery voltage drops while in "storage" or from helping the booster keep the voltage up in case there's more than 250mA of draw from amp
                           if (state_EXTERNAL_POWER == 0 ||
                               avgVoltage           <  24800 ) if (!firstStrike1) {
                                                                                  firstStrike1 = 1;
                                                                                  t_firstStrike1 = theTime;
                                                                                  }
                                                               else               {
                                                                                  if ( abs(theTime - t_firstStrike1) > 10000) {
                                                                                                                              firstStrike1 = 0;
                                                                                                                              firstStrike2 = 0;
                                                                                                                              pwrState     = 0;
                                                                                                                              }
                                                                                  }
                           else firstStrike1 = 0;

                   }break;


                   case 2: {
                           turn(chargeON);                                          // chargeON is connect via NC connection : thus, ON uses no power
                           turn(btOFF);
                           turn(ampOFF);
                           ampState = 10;                                           // deactivate main state machine
                           blinkLED (RED , 600 , 300);

                           if (avgVoltage > 18000) {
                                                   ampState = 0;
                                                   pwrState = 0;
                                                   }
                           
                    
                   }break;
  }
  
}
















/* Reads and formats all 3 inputs
 *                                                           
 * Sets   state_BTModule
 *        state_AUX_IN
 *        state_EXTERNAL_POWER
 *        state_BATT_ONLINE
 */        
void signalAquisitionModule() {
   
  /************************************************************************************     BT Mode Analyser     ************************************************************************************/
  if (digitalRead(bluetoothLED) == 0) {
                                      if (state_BTLED == 0) {
                                                            state_BTLED = 1;
                                                            t_FirstOnBT = theTime;                                                            // Record time @ which LED turns on
                                                            }
                                      }
  else                                if (state_BTLED == 1) {
                                                            state_BTLED = 0;
                                                            t_TotalOnBT = abs( theTime - t_FirstOnBT );                                       // Record for how much time the LED has been on

                                                            //Serial.println(t_TotalOnBT);

                                                            if      ( (t_TotalOnBT >  50) && (t_TotalOnBT < 150) ) { state_BTModule = 1; }    // BT Module Searching for Connection (~100ms blinks)
                                                            else if ( (t_TotalOnBT > 250) && (t_TotalOnBT < 350) ) { state_BTModule = 2; }    // BT Module Connected & Paused       (~300ms blinks)
                                                            else if ( (t_TotalOnBT > 450) && (t_TotalOnBT < 550) ) { state_BTModule = 3; }    // BT Module Connected & Playing      (~500ms blinks)
                                                            }
                                                            

  /**********************************************************************************     AUX_IN Usage Detector     *********************************************************************************/
  if (abs(theTime - t_lastAUXcheck) > 500)     {                                                                                              // Only checks every .5s --- debounce reading
                                               if ( digitalRead(AUX_IN_detector) == 0 ) state_AUX_IN = 1;
                                               else                                     state_AUX_IN = 0;

                                               t_lastAUXcheck = theTime;
                                               }


  /**********************************************************************************    External Power Detector    *********************************************************************************/
  if  (avgCurrent < 5000) state_EXTERNAL_POWER = 1;                                                                                           // We can't be drawing this little from the battery without external power being present
  else                    {
                          state_EXTERNAL_POWER = 0;
                          state_BATT_ONLINE    = 1;                                                                                           // If it isn't and we're wrong - we'll die within the second so it's fine to wrongly change this var
                          }


  /**********************************************************************************    Batt Connected Detector    **********************************************************************************/
  if (state_EXTERNAL_POWER) if (chargeEnabled)   if ( (avgVoltage > 24000) && (abs(avgCurrent)<5000) ) state_BATT_ONLINE = 0;                  // Battery must not be connected, otherwise AC supply voltage wouldn't float this high
                                                 else                                                  state_BATT_ONLINE = 1;                  // Battery must     be connected, it'll always pull AC supply down a bit
                            else                 if (avgVoltage > 10000) state_BATT_ONLINE = 1;                                                // We stopped charging the battery, so no current from AC supply can go to Power Monitor. Voltage present must be from batt
                                                 else                    state_BATT_ONLINE = 0;                                                // We stopped charging the battery, no battery being present will leave the Power Monitor reading close to 0 on both ranges
  else                                                                   state_BATT_ONLINE = 1;                                                // In case of no external power batt offline means we die within the second so we must be running on the battery
  
  
}





















 /*Reads INA226 Power Monitor Module and stores results in global variables
 * Updates values every .5s. Return true when new variables are available
 * 
 * Sets   avgCurrent
 *        avgVoltage
 *        avgWatts
 *        negativeCurrent
 *        lowBattery
 */ 
bool newPowerMonitorReadings() {

  /**********************************************************************************  Get Readings on Every Call  **********************************************************************************/
  sumCurrent += INA226.getBusMicroAmps () ;              
  sumVoltage += INA226.getBusMilliVolts() ;              
  sumWatts   += INA226.getBusMicroWatts() ;              
  readings++;


  /********************************************************************************  Compute Average once every .5s  ********************************************************************************/
  if ( abs(theTime - lastPowerMeasurement) > 499 ) {
                                                  avgCurrent = sumCurrent / readings ;
                                                  avgVoltage = sumVoltage / readings ;
                                                  avgWatts   = sumWatts   / readings ;

  /******************************************************************************  Set lowBattery & negativeAmps Flags  *****************************************************************************/
                                                  if (state_BATT_ONLINE) {
                                                                         if (avgCurrent > 0)     negativeAmps = 0;
                                                                         else                    negativeAmps = 1;
                       
                                                                         if (avgVoltage < 21500) lowBattery = 1;
                                                                         else                    lowBattery = 0;
                                                                         }
                                                  else                   {
                                                                         negativeAmps = 0;
                                                                         lowBattery   = 0;
                                                                         }

                                                  readings             = 0;
                                                  sumCurrent           = 0;
                                                  sumVoltage           = 0;
                                                  sumWatts             = 0;
                                                  lastPowerMeasurement = theTime;

                                                  return 1;
                                                  }
  else                                            return 0;
  
}













/* Draws all the numbers and graphics on the display everytime its called with the current values
 */
void refreshDisplay() {
  
  /*************************************************************************************  Format Numeric Data  **************************************************************************************/
  current = divideBy1000( String(avgCurrent , DEC) );                        // Divide microAmps   by 1000 and keep decimals  ( = mA )
  voltage = divideBy1000( String(avgVoltage , DEC) );                        // Divide milliwVolts by 1000 and keep decimals  ( = V  )


  /*********************************************************************** When NOT Charging : show W consumed by entire system *********************************************************************/
  if (state_BATT_ONLINE) if(!state_EXTERNAL_POWER) {                                                        // Only compute this stuff when battery is connected                     
                                                   temp = String(avgWatts   , DEC)  ;                
                                                   temp.remove(temp.length() - 3);                          // Obtain mW from uW without regarding decimals            
                                                   wattsORmAh = divideBy1000( temp );                       // Divide milliWats by 1000 and keep decimals    ( = W  )

                                                   mAh = 0;
                                                   }
                    

  /************************************************************************ When Charging : show mAh sinked into battery pack ***********************************************************************/
                         else                      {                                                         
                                                   mAs += -avgCurrent / 2000;                                // Convert uA to mA (/1000) and mA/.5s to mA/s (/2)

                                                   if (mAs>=3600) {                                          // Accrue mAh every 2 seconds (4 half-seconds)
                                                                  mAh += mAs / 3600;
                                                                  mAs -= (mAs / 3600) * 3600;                // Recycle remainder
                                                                  }

                                                   wattsORmAh = String(mAh , DEC);
                                                   }

  /***************************************************************** Display current, voltage, wattsORmAh and draw Battery Shape *******************************************************************/
  OLED.clearDisplay();                                                              // Erase entire display buffer (custom font doesn't support WHITE on BLACK)


  /***************************************************************************************  Render Battery  ****************************************************************************************/
  drawBatteryOutline();


  if (state_BATT_ONLINE) {                                                                                                   // Only render fill if a battery is actually present !
                         /* 
                          * Compute the size of the bar inside the battery shape :
                          * @ > 4.0 v we show full  battery when disconnected from power : this allows us to display 100% for a longer amount of time
                          * @ > 4.16v we show full  battery when charging                : this discourages users from disconnecting @ 24V
                          * @ < 3.5 v we show empty battery : the voltage is expected to plumet after this point
                          */
                         
                         
                         if (!negativeAmps)  currBattLevel = map(avgVoltage , 21500 , 24000 , 4 , 122);                      // According to SANYO 18650 datasheet : below 3.5v voltage plummets & between 3.5v and 4.1v discharge curve is ~linear
                         else                currBattLevel = map(avgVoltage , 23000 , 25200 , 4 , 122);     
                         currBattLevel = constrain( currBattLevel , 4 , 122);

//                         if (!negativeAmps && !state_EXTERNAL_POWER) if (currBattLevel < prevBattLevel) prevBattLevel = currBattLevel;                // Mask momentary voltage recoveries in the fill bar (allow them when charging)
//                                                                     else                               currBattLevel = prevBattLevel;
                     

                         OLED.fillRoundRect( 2 , 2 , currBattLevel , 12 , 2 , WHITE);
   

                         if (negativeAmps) {                                                                                 // Indicate if the battery is being charged by an arrow '>' in- or outside the battery bar
                                           if (currBattLevel <= 10) currBattLevel += 15;                                     // Bar is too narrow to fit '>' : we draw it outside
                                           else                     OLED.setTextColor(BLACK);                                // Bar is wide enough now : draw it inside, with BLACK color
                          
                                           OLED.setCursor (currBattLevel - 10 , 12);
                                           OLED.print(">");
                          
                                           OLED.setTextColor(WHITE);                                                         // Reset the color back to WHITE
                                           }

                         if (!chargeEnabled) {                                                                               // Indicate when battery is full and disconnected from power
                                             OLED.setTextColor(BLACK);
                                             OLED.setCursor(9 , 12);
                                             OLED.print("Batt. Full");
                                             OLED.setTextColor(WHITE);
                                             }

                         if      (  (lowBattery) && battBlinker ) {                                                          // Blink the entire Battery when it's low
                                                                  OLED.fillRect(0 , 0 , 128 , 16 , BLACK);
                                                                  battBlinker = 0;
                                                                  turnLED(OFF);
                                                                  }
                         else if (  (lowBattery) && !battBlinker ) {
                                                                   battBlinker = 1;
                                                                   turnLED(RED);
                                                                   }
                         }
  else                   {
                         OLED.setCursor(24 , 12);
                         OLED.print("No Batt.");
                         }

  /*****************************************************************************************  Render Text  *****************************************************************************************/

  if (negativeAmps)  { 
                     OLED.setCursor(0 ,28);                                                           // Add '-' in case battery is charging
                     OLED.print("-");
                     }

  OLED.setCursor(10,28);
  OLED.print( current );
  
  OLED.setCursor(96,28);
  OLED.print("mA");       //"mA"



  OLED.setCursor(10,45);
  OLED.print( voltage );

  OLED.setCursor(96,45);
  OLED.print("V");        //"V"



  if (state_BATT_ONLINE && chargeEnabled) {                                                                            // Only display W or mAh when Battery is Connected and in circuit
                                          if (!negativeAmps) {
                                                             OLED.setCursor(96,62);
                                                             OLED.print("W");
                                                             }
                                          else               {
                                                             OLED.setCursor(96,62);
                                                             OLED.print("mAh");
                                                             }
                                      
                                          OLED.setCursor(10,62);
                                          if (negativeAmps) switch( wattsORmAh.length() ) {                                                   // Place mAh number right next to "mAh", and let it grow to the left
                                                                                          case 1 : OLED.print("      "); break;
                                                                                          case 2 : OLED.print("     ");  break;
                                                                                          case 3 : OLED.print("    ");   break;
                                                                                          case 4 : OLED.print("   ");    break;
                                                                                          case 5 : OLED.print("  ");     break;
                                                                                          }
                                          OLED.print( wattsORmAh );
                                          }
  else                                    OLED.fillRect(0 , 46 , 127 , 17 , BLACK);                                                            // Erase the 3rd line when battery is disconnected


  /**************************************************************************************  Show Buffer on OLED  ************************************************************************************/
  OLED.display();
}




/* Draws Battery Outline to the Display Buffer
 */
void drawBatteryOutline() {

  OLED.drawRoundRect(0   , 0 , 126 , 16 , 2 , WHITE);                                                 // drawRoundRect (int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),
  OLED.fillRect     (125 , 5 , 3   , 6      , WHITE);                                                 // fillRect      (int16_t x,  int16_t y,  int16_t w, int16_t h, uint16_t color)
  
}







/* Divides a String number betw. [1 , 999 999] by 1000 and formats it to always have 3 comma digits
 */
String divideBy1000(String input) {

  output = "";
  
  if ( input.startsWith("-") ) input.remove(0,1);
  
  switch( input.length() ) {
                               case 1 : {                                                   // 1 ->       "___.001_"
                                        output.concat( "   .00" );
                                        output.concat( input );
                                        output.concat( " " );
                               }break;

                               case 2 : {                                                   // 12 ->      "___.012_"
                                        output.concat( "   .0" );
                                        output.concat( input );
                                        output.concat( " " );
                               }break;

                               case 3 : {                                                   // 123 ->     "___.123_"
                                        output.concat( "   ." );
                                        output.concat( input );
                                        output.concat( " " );
                               }break;

                               case 4 : {                                                   // 1234 ->    "__1.234_"
                                        output.concat( "  " );
                                        output.concat( input.substring(0,1) );
                                        output.concat( "." );
                                        output.concat( input.substring(1)   );
                                        output.concat( " " );
                               }break;

                               case 5 : {                                                   // 12345 ->   "_12.345_"
                                        output.concat( " " );
                                        output.concat( input.substring(0,2) );
                                        output.concat( "." );
                                        output.concat( input.substring(2)   );
                                        output.concat( " " );
                               }break;
                                        
                               case 6 : {                                                    // 123456 -> "123.456_"
                                        output.concat( input.substring(0,3) );
                                        output.concat( "." );
                                        output.concat( input.substring(3)   );
                                        output.concat( " " );
                               }break;
                            
                              }

  return output;
  
}










/* Resets Bluetooth Module every 50000 calls to it
 */
void bluetoothConnectionResuscitator() {
     
     if  (timeAlone == 30000) {                                  // Reboot BT Module to ensure it always keeps trying to pair
                              timeAlone = 0;
                
                              turn    (btOFF);         
                
                                                if (chargeEnabled) turn(chargeOFF);
                                                else               turn(chargeON );
                                                                                                                         
                              OLED.clearDisplay();
                              OLED.setCursor(15,30);
                              OLED.print("Resetting");
                              OLED.setCursor(15,50);
                              OLED.print("Bluetooth");
                              OLED.display();
                      
                              turn    (btON );                   // Includes 5000 ms delay to ensure BT Module properly resets
                
                                                if (chargeEnabled) turn(chargeOFF);
                                                else               turn(chargeON );
                                                                 
                              }
     else timeAlone++;
}
















/* Manages all the relays and modules and ensures they function properly
 * 
 * Ensures amp stays on for at least 1 minute
 * Ensures amp relay doesn't stick
 * 
 * Ensures bluetooth module stays off for at least 5s
*/
void turn(char desiredAction) {

  switch(desiredAction) {
                        case ampON     : {
                                         timeAMPswitchedON = theTime;
                            
                                         if  (currentAmpState == 1) return;                          // Amp already ON
                                         
                                         currentAmpState = 1;


                                         digitalWrite(ampRelay , LOW);                               // Turn Amp ON (low side triggered relay)
                                         }break;

                        case ampOFF    : {                                            
                                         if  (currentAmpState == 0) return;                          // Amp already OFF
                                         

                                         if  ( abs(theTime - timeAMPswitchedON) < 60000 ) return;   // Only turn amp off after it's been on for at least 45s (all calls before then will be ignored)
                                         
                                         currentAmpState =  0;

                                         digitalWrite(ampRelay , HIGH);                              // Turn Amp OFF (low side triggered relay)
                                         delay(500);

                                         while(digitalRead(ampStillON) != 0)                         // @24V the relay sometimes sticks : needs more tries to disengage
                                              {
                                              digitalWrite(ampRelay , LOW );
                                              delay(15);
                                              digitalWrite(ampRelay , HIGH);
                                              delay(15);
                                              digitalWrite(ampRelay , LOW );
                                              delay(15);
                                              digitalWrite(ampRelay , HIGH);
                                              delay(15);
                                              digitalWrite(ampRelay , LOW );
                                              delay(15);
                                              digitalWrite(ampRelay , HIGH);
                                              delay(500);
                                              }
                                         }break;

                        case chargeON  : {
                                         if (chargeEnabled == 1) return;
                                         
                                         chargeEnabled = 1;
                                         digitalWrite(chargeRelay , LOW);  
                                         
                                         }break;

                        case chargeOFF : {
                                         if  (chargeEnabled == 0) return;
                                         
                                         chargeEnabled =  0;
                                         digitalWrite(chargeRelay , HIGH);  
                                         
                                         }break;

                        case btON : {
                                         if  (bluetoothON == 1) return;
                                         else {
                                              if ( abs(theTime - timeBTswitchedOFF) < 5000 ) delay( 5000 - abs(theTime - timeBTswitchedOFF) );         // Ensures BT Module Completely Powers Down !
                                              
                                              bluetoothON =  1;
                                              
                                              pinMode(bluetoothBuck , INPUT);           // EN : floating = ON   0V = OFF
                                              }

                                         
                                         }break;

                        case btOFF : {
                                         if  (bluetoothON == 0) return;
                                         else {
                                              bluetoothON =  0;
                                              
                                              timeBTswitchedOFF = theTime;
                                              
                                              pinMode     (bluetoothBuck , OUTPUT);     // EN : floating = ON   0V = OFF
                                              digitalWrite(bluetoothBuck , LOW);
                                              }

                                         
                                         }break;
                        }
    
  
}











/*  Transparent LED blinker; iff called every 1 ms, the on- and offtime it's being fed will be in ms
 *  Feeding in negative values for either onTime or offTime will make the on- or offTime be infinitely long
 */
void blinkLED(const char color , int onTime , int offTime) {

    // Reset state variables when changing color
    if (color != previousColor) {
                                redLEDstate   = 0;
                                greenLEDstate = 0;
                                blueLEDstate  = 0;
                                
                                alreadyOffFor = 0;
                                alreadyOnFor  = 0;                                
                                
                                previousColor = color;
                                }
  
    if (color == RED)        if (redLEDstate == 0)   if (alreadyOffFor >= offTime) {
                                                                                   alreadyOffFor = 0;
                                                                                   turnLED(RED);
                                                                                   redLEDstate = 1;
                                                                                   }
                                                     else                          alreadyOffFor++;
                      
                             else                    if (alreadyOnFor >= onTime)   {
                                                                                   alreadyOnFor = 0;
                                                                                   turnLED(OFF);
                                                                                   redLEDstate = 0;
                                                                                   }
                                                     else                          alreadyOnFor++;
                      

    else if (color == GREEN) if (greenLEDstate == 0) if (alreadyOffFor >= offTime) {
                                                                                   alreadyOffFor = 0;
                                                                                   turnLED(GREEN);
                                                                                   greenLEDstate = 1;
                                                                                   }
                                                     else                          alreadyOffFor++;
                      
                             else                    if (alreadyOnFor >= onTime)   {
                                                                                   alreadyOnFor = 0;
                                                                                   turnLED(OFF);
                                                                                   greenLEDstate = 0;
                                                                                   }
                                                     else                          alreadyOnFor++;
                      

    else if (color == BLUE) if (blueLEDstate == 0)  if (alreadyOffFor >= offTime)  {
                                                                                   alreadyOffFor = 0;
                                                                                   turnLED(BLUE);
                                                                                   blueLEDstate = 1;
                                                                                   }
                                                    else                           alreadyOffFor++;
                      
                            else                    if (alreadyOnFor >= onTime)    {
                                                                                   alreadyOnFor = 0;
                                                                                   turnLED(OFF);
                                                                                   blueLEDstate = 0;
                                                                                   }
                                                    else                           alreadyOnFor++;
                      
}














void turnLED(char color) {

  /* All LEDs on the front pannel have a common POSITIVE, so switching will be done on the low side */

  if      (color == RED)   {
                           digitalWrite(GREEN_LED , HIGH);
                           digitalWrite(BLUE_LED  , HIGH);
                           digitalWrite(RED_LED   , LOW );
                           }

  else if (color == GREEN) {
                           digitalWrite(RED_LED   , HIGH);
                           digitalWrite(BLUE_LED  , HIGH);
                           digitalWrite(GREEN_LED , LOW );
                           }

  else if (color == BLUE)  {
                           digitalWrite(RED_LED   , HIGH);
                           digitalWrite(GREEN_LED , HIGH);
                           digitalWrite(BLUE_LED  , LOW );
                           }

  else if (color == OFF)   {
                           digitalWrite(RED_LED   , HIGH);
                           digitalWrite(GREEN_LED , HIGH);
                           digitalWrite(BLUE_LED  , HIGH);
                           }
  
}







