
#include<Key.h> // Use to make key mapping
#include<Keypad.h> // This is library for using Phone Type Keypad
#include "ACS712.h" //This Library is used for measuring current using the ACS71205_BT current Sensor
#include <TimeLib.h> //This is The Time Library is used for reading current time
#include <DS1307RTC.h> //This library is used to interact with DS1307 RTC module
#include <Fuzzy.h> //This Library is used for creating Fuzzyset and Rules
#include <Filters.h> //Easy library to do the calculations
#include <LiquidCrystal_I2C.h>//For communicating to the 16x2 LCD display through I2C Prorocol


Fuzzy *fuzzy = new Fuzzy();    // Instantiating a Fuzzy object


  LiquidCrystal_I2C lcd2(0x27, 16, 2);// Declaring LCD at 0x27 as lcd2
  
//Membership Function Declaration for Fuzzy Input And Output ------------------------------------------------------------------
  //Time
  FuzzySet *daye = new FuzzySet(5, 6, 17, 18);
  FuzzySet *night_1 = new FuzzySet(17, 18, 23, 24);
  FuzzySet *night_2 = new FuzzySet(-5, 0, 5, 6);

  //Load
  FuzzySet *light = new FuzzySet(-5, 0, 20, 21);
  FuzzySet *medium = new FuzzySet(20, 21, 30, 31);

  //Grid
  FuzzySet *no = new FuzzySet(-10, -5, 0, 1);
  FuzzySet *yes = new FuzzySet(0, 1, 250, 250);

  //Battery
  FuzzySet *high_bat = new FuzzySet(11, 12, 13, 13);
  FuzzySet *medium_bat = new FuzzySet(10, 11, 11, 12);
  FuzzySet *nil = new FuzzySet(9, 10, 10, 11);


  //Output-
  FuzzySet *a = new FuzzySet(1, 2, 2, 3); //Connect_battery
  FuzzySet *b = new FuzzySet(0, 1, 1, 2); //Connect_Grid

//------------------------------------------------------------------------------------------------------------------------------

  //RelayPin config for the four appliances
  int ac_b3=9;
  int dc_b3=8;


  //battery realy config
  int bat_rly_a=2;
  int bat_rly_b=3;
  int batcontrol=4;

 //AnalogInput from voltage divider circuits to measure battery status
  int analogInput1 = A12;
  int analogInput2 = A13;


  //Variable to set Grid Availability and Calculate Total Power consumed
  float Grid_Voltage = 0.0;
  float pow_sum = 0.0;

  //Variables to store the battery volatges and to 
  long ch = 0;
  float volt1 =0;
  float vin1 = 0.0;
  float volt2 =0;
  float vin2 = 0.0;
  float vin =0;
  
  //Variables to used to store previous operation values
  long processrem = 0;
  long NoGrid_con = 0;
  int outputrem = 0;


//-------KEYPAD-------------------------------------------------------------------------------
  // Variables used in describing the rows and colums of the keypad
  const byte ROWS = 4;
  const byte COLS = 3;
  int keypadval = 0;
  //Description of elements in keypad
  char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
  };
  byte rowPins[ROWS] = {29, 28, 27, 26};
  byte colPins[COLS] = {25, 24, 23};
  //mapping the elements in keypad to the specific row and column pins on arduino
  Keypad keypad = Keypad ( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
  //------------------------------------------------------------------------------------------

  //LED AND BUZZER PIN DECLARATION
  int keypadled = 14;
  int startbuzz = 15;

// Current Sensor Declaration
 ACS712 sensor1(ACS712_20A, A0);


 //Battery Offset
 int offset1 = -205;
 int offset2 = -215;

 // AC-VOLTAGE SENSOR------------------------------------------------
float testFrequency = 50;                     
float windowLength = 40.0/testFrequency;     // how long to average the signal, for statistist
int Sensor = 1; //Sensor analog input, here it's A1

float intercept = -0.04; // to be adjusted based on calibration testing
float slope = 0.0405; // to be adjusted based on calibration testing
float current_Volts; // Voltage

unsigned long printPeriod = 1000; //Refresh rate
unsigned long previousMillis = 0;
 //-------------------------------------------------------------------
 
void setup() { 

   //LCD INTIALIZATION-------------------------
   lcd2.init();
   lcd2.backlight();
   lcd2.begin(16 ,2);
   //------------------------------------------

   //INPUT PINS
   pinMode(analogInput1, INPUT);
   pinMode(analogInput2, INPUT);
   
   //OUTPUT PINS
   pinMode(ac_b3, OUTPUT);
   pinMode(dc_b3, OUTPUT);
   pinMode(keypadled, OUTPUT);
   pinMode(startbuzz, OUTPUT);
   pinMode(bat_rly_a, OUTPUT);
   pinMode(bat_rly_b, OUTPUT);
   pinMode(batcontrol, OUTPUT);
   
   

   //Intially Connect to the Grid
   digitalWrite(ac_b3, LOW);
   digitalWrite(dc_b3, LOW);
   digitalWrite(keypadled, LOW);

   
   
   
   //SETUP START MESSAGE------------------
   lcd2.print("SHEMS TURNED ON!");
   digitalWrite(startbuzz, HIGH);
   delay(100);
   digitalWrite(startbuzz, LOW);
   delay(100);
   digitalWrite(startbuzz, HIGH);
   delay(100);
   digitalWrite(startbuzz, LOW);
   delay(100);
   digitalWrite(startbuzz, HIGH);
   delay(100);
   digitalWrite(startbuzz, LOW);
   delay(1500);
   lcd2.clear();
   //-------------------------------------

//Adding The Membership functions to the fuzzy input and output ------------------------------------------------------------------------------------------------------------------
  
  //Time
  FuzzyInput *toime = new FuzzyInput(1);
  toime->addFuzzySet(daye);
  toime->addFuzzySet(night_1);
  toime->addFuzzySet(night_2);
  fuzzy->addFuzzyInput(toime);

  //Load
  FuzzyInput *load = new FuzzyInput(2);
  load->addFuzzySet(light);
  load->addFuzzySet(medium);
  fuzzy->addFuzzyInput(load);

  //Grid
  FuzzyInput *gride = new FuzzyInput(3);
  gride->addFuzzySet(no);
  gride->addFuzzySet(yes);
  fuzzy->addFuzzyInput(gride);

  //Battery
  FuzzyInput *battery = new FuzzyInput(4);
  battery->addFuzzySet(high_bat);
  battery->addFuzzySet(medium_bat);
  battery->addFuzzySet(nil);
  fuzzy->addFuzzyInput(battery);


  //Output-connection
  FuzzyOutput *connect = new FuzzyOutput(1);
  connect->addFuzzySet(a);
  connect->addFuzzySet(b);
  fuzzy->addFuzzyOutput(connect);

//Fuzzy Rules Created According to the Table --------------------------------------------------------------------------------------------------------------------------------------------
  
  // Building FuzzyRule01
  FuzzyRuleAntecedent *ToimeDayAndLoadLight = new FuzzyRuleAntecedent();
  ToimeDayAndLoadLight->joinWithAND(daye,light);

  FuzzyRuleAntecedent *ToimeDayAndLoadLightAndGrideYes = new FuzzyRuleAntecedent();
  ToimeDayAndLoadLightAndGrideYes->joinWithAND(ToimeDayAndLoadLight,yes);

  FuzzyRuleAntecedent *ifToimeDayAndLoadLightAndGrideYesAndBatteryHigh = new FuzzyRuleAntecedent();
  ifToimeDayAndLoadLightAndGrideYesAndBatteryHigh->joinWithAND(ToimeDayAndLoadLightAndGrideYes,high_bat);

  FuzzyRuleConsequent *thenConnectA = new FuzzyRuleConsequent();
  thenConnectA->addOutput(a);

  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, ifToimeDayAndLoadLightAndGrideYesAndBatteryHigh, thenConnectA);
  fuzzy->addFuzzyRule(fuzzyRule1);


  // Building FuzzyRule02
  FuzzyRuleAntecedent *ifToimeDayAndLoadLightAndGrideYesAndBatteryMedium = new FuzzyRuleAntecedent();
  ifToimeDayAndLoadLightAndGrideYesAndBatteryMedium->joinWithAND(ToimeDayAndLoadLightAndGrideYes,medium_bat);

  FuzzyRule *fuzzyRule2 = new FuzzyRule(2, ifToimeDayAndLoadLightAndGrideYesAndBatteryMedium, thenConnectA);
  fuzzy->addFuzzyRule(fuzzyRule2);


  // Building FuzzyRule03
  FuzzyRuleAntecedent *ifToimeDayAndLoadLightAndGrideYesAndBatteryNil = new FuzzyRuleAntecedent();
  ifToimeDayAndLoadLightAndGrideYesAndBatteryNil->joinWithAND(ToimeDayAndLoadLightAndGrideYes,nil);

  FuzzyRuleConsequent *thenConnectB = new FuzzyRuleConsequent();
  thenConnectB->addOutput(b);

  FuzzyRule *fuzzyRule3 = new FuzzyRule(3, ifToimeDayAndLoadLightAndGrideYesAndBatteryNil, thenConnectB );
  fuzzy->addFuzzyRule(fuzzyRule3);


  
  // Building FuzzyRule04
  FuzzyRuleAntecedent *ToimeDayAndLoadLightAndGrideNo = new FuzzyRuleAntecedent();
  ToimeDayAndLoadLightAndGrideNo->joinWithAND(ToimeDayAndLoadLight,no);

  FuzzyRuleAntecedent *ifToimeDayAndLoadLightAndGrideNoAndBatteryHigh = new FuzzyRuleAntecedent();
  ifToimeDayAndLoadLightAndGrideNoAndBatteryHigh->joinWithAND(ToimeDayAndLoadLightAndGrideNo,high_bat);

  FuzzyRule *fuzzyRule4 = new FuzzyRule(4, ifToimeDayAndLoadLightAndGrideNoAndBatteryHigh, thenConnectA);
  fuzzy->addFuzzyRule(fuzzyRule4);


  // Building FuzzyRule05
  FuzzyRuleAntecedent *ifToimeDayAndLoadLightAndGrideNoAndBatteryMedium = new FuzzyRuleAntecedent();
  ifToimeDayAndLoadLightAndGrideNoAndBatteryMedium->joinWithAND(ToimeDayAndLoadLightAndGrideNo,medium_bat);
  
  FuzzyRule *fuzzyRule5 = new FuzzyRule(5, ifToimeDayAndLoadLightAndGrideNoAndBatteryMedium , thenConnectA);
  fuzzy->addFuzzyRule(fuzzyRule5);


  // Building FuzzyRule06
  FuzzyRuleAntecedent *ifToimeDayAndLoadLightAndGrideNoAndBatteryNil = new FuzzyRuleAntecedent();
  ifToimeDayAndLoadLightAndGrideNoAndBatteryNil->joinWithAND(ToimeDayAndLoadLightAndGrideNo,nil);
  
  FuzzyRule *fuzzyRule6 = new FuzzyRule(6, ifToimeDayAndLoadLightAndGrideNoAndBatteryNil, thenConnectB);
  fuzzy->addFuzzyRule(fuzzyRule6);


  
  // Building FuzzyRule07
  FuzzyRuleAntecedent *ToimeDayAndLoadMedium = new FuzzyRuleAntecedent();
  ToimeDayAndLoadMedium->joinWithAND(daye,medium);

  FuzzyRuleAntecedent *ToimeDayAndLoadMediumAndGrideYes = new FuzzyRuleAntecedent();
  ToimeDayAndLoadMediumAndGrideYes->joinWithAND(ToimeDayAndLoadMedium,yes);

  FuzzyRuleAntecedent *ifToimeDayAndLoadMediumAndGrideYesAndBatteryHigh = new FuzzyRuleAntecedent();
  ifToimeDayAndLoadMediumAndGrideYesAndBatteryHigh->joinWithAND(ToimeDayAndLoadMediumAndGrideYes,high_bat);

  FuzzyRule *fuzzyRule7 = new FuzzyRule(7, ifToimeDayAndLoadMediumAndGrideYesAndBatteryHigh, thenConnectA);
  fuzzy->addFuzzyRule(fuzzyRule7);

  

  // Building FuzzyRule08
  FuzzyRuleAntecedent *ifToimeDayAndLoadMediumAndGrideYesAndBatteryMedium = new FuzzyRuleAntecedent();
  ifToimeDayAndLoadMediumAndGrideYesAndBatteryMedium->joinWithAND(ToimeDayAndLoadMediumAndGrideYes,medium_bat);

  FuzzyRule *fuzzyRule8 = new FuzzyRule(8, ifToimeDayAndLoadMediumAndGrideYesAndBatteryMedium, thenConnectA);
  fuzzy->addFuzzyRule(fuzzyRule8);
  

  // Building FuzzyRule09
  FuzzyRuleAntecedent *ifToimeDayAndLoadMediumAndGrideYesAndBatteryNil = new FuzzyRuleAntecedent();
  ifToimeDayAndLoadMediumAndGrideYesAndBatteryNil->joinWithAND(ToimeDayAndLoadMediumAndGrideYes,nil);

  FuzzyRule *fuzzyRule9 = new FuzzyRule(9, ifToimeDayAndLoadMediumAndGrideYesAndBatteryNil, thenConnectB);
  fuzzy->addFuzzyRule(fuzzyRule9);

  

  // Building FuzzyRule10
  FuzzyRuleAntecedent *ToimeDayAndLoadMediumAndGrideNo = new FuzzyRuleAntecedent();
  ToimeDayAndLoadMediumAndGrideNo->joinWithAND(ToimeDayAndLoadMedium,no);

  FuzzyRuleAntecedent *ifToimeDayAndLoadMediumAndGrideNoAndBatteryHigh = new FuzzyRuleAntecedent();
  ifToimeDayAndLoadMediumAndGrideNoAndBatteryHigh->joinWithAND(ToimeDayAndLoadMediumAndGrideNo,high_bat);
    
  FuzzyRule *fuzzyRule10 = new FuzzyRule(10, ifToimeDayAndLoadMediumAndGrideNoAndBatteryHigh, thenConnectA);
  fuzzy->addFuzzyRule(fuzzyRule10);


  // Building FuzzyRule11
  FuzzyRuleAntecedent *ifToimeDayAndLoadMediumAndGrideNoAndBatteryMedium = new FuzzyRuleAntecedent();
  ifToimeDayAndLoadMediumAndGrideNoAndBatteryMedium->joinWithAND(ToimeDayAndLoadMediumAndGrideNo,medium_bat);
    
  FuzzyRule *fuzzyRule11 = new FuzzyRule(11, ifToimeDayAndLoadMediumAndGrideNoAndBatteryMedium, thenConnectB);
  fuzzy->addFuzzyRule(fuzzyRule11);


  // Building FuzzyRule12
  FuzzyRuleAntecedent *ifToimeDayAndLoadMediumAndGrideNoAndBatteryNil = new FuzzyRuleAntecedent();
  ifToimeDayAndLoadMediumAndGrideNoAndBatteryNil->joinWithAND(ToimeDayAndLoadMediumAndGrideNo,nil);
    
  FuzzyRule *fuzzyRule12 = new FuzzyRule(12, ifToimeDayAndLoadMediumAndGrideNoAndBatteryNil, thenConnectB);
  fuzzy->addFuzzyRule(fuzzyRule12);


  
 // Building FuzzyRule13
  FuzzyRuleAntecedent *ToimeNight_1OrToimeNight_2 = new FuzzyRuleAntecedent();
  ToimeNight_1OrToimeNight_2->joinWithOR(night_1,night_2);

  FuzzyRuleAntecedent *ToimeNight_1OrToimeNight_2AndLoadLight = new FuzzyRuleAntecedent();
  ToimeNight_1OrToimeNight_2AndLoadLight->joinWithAND(ToimeNight_1OrToimeNight_2,light);

  FuzzyRuleAntecedent *ToimeNight_1OrToimeNight_2AndLoadLightAndGrideYes = new FuzzyRuleAntecedent();
  ToimeNight_1OrToimeNight_2AndLoadLightAndGrideYes->joinWithAND(ToimeNight_1OrToimeNight_2AndLoadLight,yes);

  FuzzyRuleAntecedent *ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideYesAndBatteryHigh = new FuzzyRuleAntecedent();
  ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideYesAndBatteryHigh->joinWithAND(ToimeNight_1OrToimeNight_2AndLoadLightAndGrideYes,high_bat);

  FuzzyRule *fuzzyRule13 = new FuzzyRule(13, ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideYesAndBatteryHigh, thenConnectA);
  fuzzy->addFuzzyRule(fuzzyRule13);



 // Building FuzzyRule14
  FuzzyRuleAntecedent *ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideYesAndBatteryMedium = new FuzzyRuleAntecedent();
  ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideYesAndBatteryMedium->joinWithAND(ToimeNight_1OrToimeNight_2AndLoadLightAndGrideYes,medium_bat);

  FuzzyRule *fuzzyRule14 = new FuzzyRule(14, ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideYesAndBatteryMedium, thenConnectB);
  fuzzy->addFuzzyRule(fuzzyRule14);


 // Building FuzzyRule15
  FuzzyRuleAntecedent *ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideYesAndBatteryNil = new FuzzyRuleAntecedent();
  ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideYesAndBatteryNil->joinWithAND(ToimeNight_1OrToimeNight_2AndLoadLightAndGrideYes,nil);

  FuzzyRule *fuzzyRule15 = new FuzzyRule(15, ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideYesAndBatteryNil, thenConnectB);
  fuzzy->addFuzzyRule(fuzzyRule15);



  // Building FuzzyRule16
  FuzzyRuleAntecedent *ToimeNight_1OrToimeNight_2AndLoadLightAndGrideNo = new FuzzyRuleAntecedent();
  ToimeNight_1OrToimeNight_2AndLoadLightAndGrideNo->joinWithAND(ToimeNight_1OrToimeNight_2AndLoadLight,no);

  FuzzyRuleAntecedent *ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideNoAndBatteryHigh = new FuzzyRuleAntecedent();
  ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideNoAndBatteryHigh->joinWithAND(ToimeNight_1OrToimeNight_2AndLoadLightAndGrideNo,high_bat);

  FuzzyRule *fuzzyRule16 = new FuzzyRule(16, ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideNoAndBatteryHigh, thenConnectA);
  fuzzy->addFuzzyRule(fuzzyRule16);


  // Building FuzzyRule17
  FuzzyRuleAntecedent *ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideNoAndBatteryMedium = new FuzzyRuleAntecedent();
  ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideNoAndBatteryMedium->joinWithAND(ToimeNight_1OrToimeNight_2AndLoadLightAndGrideNo,medium_bat);

  FuzzyRule *fuzzyRule17 = new FuzzyRule(17, ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideNoAndBatteryMedium, thenConnectA);
  fuzzy->addFuzzyRule(fuzzyRule17);


  // Building FuzzyRule18
  FuzzyRuleAntecedent *ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideNoAndBatteryNil = new FuzzyRuleAntecedent();
  ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideNoAndBatteryNil->joinWithAND(ToimeNight_1OrToimeNight_2AndLoadLightAndGrideNo,nil);

  FuzzyRule *fuzzyRule18 = new FuzzyRule(18, ifToimeNight_1OrToimeNight_2AndLoadLightAndGrideNoAndBatteryNil, thenConnectB);
  fuzzy->addFuzzyRule(fuzzyRule18);



 // Building FuzzyRule19
  FuzzyRuleAntecedent *ToimeNight_1OrToimeNight_2AndLoadMedium = new FuzzyRuleAntecedent();
  ToimeNight_1OrToimeNight_2AndLoadMedium->joinWithAND(ToimeNight_1OrToimeNight_2,medium);

  FuzzyRuleAntecedent *ToimeNight_1OrToimeNight_2AndLoadMediumAndGrideYes = new FuzzyRuleAntecedent();
  ToimeNight_1OrToimeNight_2AndLoadMediumAndGrideYes->joinWithAND(ToimeNight_1OrToimeNight_2AndLoadMedium,yes);

  FuzzyRuleAntecedent *ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideYesAndBatteryHigh = new FuzzyRuleAntecedent();
  ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideYesAndBatteryHigh->joinWithAND(ToimeNight_1OrToimeNight_2AndLoadMediumAndGrideYes,high_bat);

  FuzzyRule *fuzzyRule19 = new FuzzyRule(19, ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideYesAndBatteryHigh, thenConnectB);
  fuzzy->addFuzzyRule(fuzzyRule19);




 // Building FuzzyRule20
  FuzzyRuleAntecedent *ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideYesAndBatteryMedium = new FuzzyRuleAntecedent();
  ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideYesAndBatteryMedium->joinWithAND(ToimeNight_1OrToimeNight_2AndLoadMediumAndGrideYes,medium_bat);

  FuzzyRule *fuzzyRule20 = new FuzzyRule(20, ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideYesAndBatteryMedium, thenConnectB);
  fuzzy->addFuzzyRule(fuzzyRule20);


 // Building FuzzyRule21
  FuzzyRuleAntecedent *ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideYesAndBatteryNil = new FuzzyRuleAntecedent();
  ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideYesAndBatteryNil->joinWithAND(ToimeNight_1OrToimeNight_2AndLoadMediumAndGrideYes,nil);

  FuzzyRule *fuzzyRule21 = new FuzzyRule(21, ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideYesAndBatteryNil, thenConnectB);
  fuzzy->addFuzzyRule(fuzzyRule21);



 // Building FuzzyRule22
  FuzzyRuleAntecedent *ToimeNight_1OrToimeNight_2AndLoadMediumAndGrideNo = new FuzzyRuleAntecedent();
  ToimeNight_1OrToimeNight_2AndLoadMediumAndGrideNo->joinWithAND(ToimeNight_1OrToimeNight_2AndLoadMedium,no);

  FuzzyRuleAntecedent *ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideNoAndBatteryHigh = new FuzzyRuleAntecedent();
  ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideNoAndBatteryHigh->joinWithAND(ToimeNight_1OrToimeNight_2AndLoadMediumAndGrideNo,high_bat);

  FuzzyRule *fuzzyRule22 = new FuzzyRule(22, ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideNoAndBatteryHigh, thenConnectA);
  fuzzy->addFuzzyRule(fuzzyRule22);


 // Building FuzzyRule23
  FuzzyRuleAntecedent *ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideNoAndBatteryMedium = new FuzzyRuleAntecedent();
  ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideNoAndBatteryMedium->joinWithAND(ToimeNight_1OrToimeNight_2AndLoadMediumAndGrideNo,medium_bat);

  FuzzyRule *fuzzyRule23 = new FuzzyRule(23, ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideNoAndBatteryMedium, thenConnectB);
  fuzzy->addFuzzyRule(fuzzyRule23);

  

 // Building FuzzyRule24
  FuzzyRuleAntecedent *ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideNoAndBatteryNil = new FuzzyRuleAntecedent();
  ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideNoAndBatteryNil->joinWithAND(ToimeNight_1OrToimeNight_2AndLoadMediumAndGrideNo,nil);

  FuzzyRule *fuzzyRule24 = new FuzzyRule(24, ifToimeNight_1OrToimeNight_2AndLoadMediumAndGrideNoAndBatteryNil, thenConnectB);
  fuzzy->addFuzzyRule(fuzzyRule24);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  

}

void loop() {
    tmElements_t tm;
    if (RTC.read(tm)) {

  //BATTERY 1 RANDOM INPUT--------------------------------------------------
  vin1 = random(10,13);//RANDOM VALUE
  //------------------------------------------------------------------------
  
 //BATTERY 2 RANDOM INPUT---------------------------------------------------
  vin2 = random(10,11);//RANDOM VALUE
  //------------------------------------------------------------------------

  //TIME RANDOM INPUT-------------------------------------------------------
  int timer = random(0,24);
  //------------------------------------------------------------------------


  
  //AC-VOLAGE SENSOR--------------------------------------------------------
  current_Volts = ac_voltage();
  if( current_Volts < 120)
  current_Volts =0;
  //------------------------------------------------------------------------

  
  lcd2.print("OPERATION: ");
  lcd2.print((processrem+1));
  delay(2000);
  lcd2.clear();
  lcd2.print("Time = ");
  lcd2.print(timer);
  lcd2.print(':');
  lcd2.print(int(tm.Minute));
  lcd2.print(':');
  lcd2.print(int(tm.Second));
  
  lcd2.setCursor(0, 1);
  lcd2.print("Date = ");
  lcd2.print(int(tm.Day));
  lcd2.print('/');
  lcd2.print(int(tm.Month));
  lcd2.print('/');
  lcd2.print(tmYearToCalendar(tm.Year));
  delay(2000);
  //BATTERY SELECTION-----------------------------------
  batcondition();
  //----------------------------------------------------
  
  lcd2.print("BATTERY STATUS: ");
  delay(2000);
  lcd2.clear();
  lcd2.print("BAT_1: ");
  lcd2.print(vin1);
  lcd2.print(" V");
  lcd2.setCursor(0, 1);
  lcd2.print("BAT_2: ");
  lcd2.print(vin2);
  lcd2.print(" V");
  delay(2000);
  lcd2.clear();


  //GRID CHECKING--------------------------------------------------
  lcd2.print("GRID: ");
  if(current_Volts < 120)
  {
    if(processrem == 0)
    NoGrid_con = 1;
    current_Volts = 0;
    Grid_Voltage = 0.0;

    lcd2.print("NO");
    delay(2000);
  }
  if(current_Volts > 120)
  {
    Grid_Voltage = 1.0;

    lcd2.print("YES");
    delay(2000);
  }
  //--------------------------------------------------------------

  
  //FAIL SAFE CONDITION-------------------------------------------
  if(Grid_Voltage == 0 && NoGrid_con == 0 && outputrem == 1 && vin > 1)
  {
    lcd2.clear();
    digitalWrite(keypadled, HIGH);
    digitalWrite(startbuzz, HIGH);
    delay(1000);
    digitalWrite(startbuzz, LOW);
    lcd2.print("----WARNING----");
    delay(2000);
    lcd2.clear();
    lcd2.print("LOAD NOT");
    lcd2.setCursor(0,1);
    lcd2.print("SUPPORTED");
    delay(2000);
    lcd2.clear();
    lcd2.print("TO CHANGE LOAD ");
    lcd2.setCursor(0,1);
    lcd2.print("PRESS 1!");
    int keypadval = GetNumber();
    if( keypadval == 1)
    {
      digitalWrite(keypadled, LOW);
      NoGrid_con = 1;
 
      lcd2.clear();
      lcd2.print("LOAD CHANGED");
      delay(2000);
    }
  }
  //--------------------------------------------------------------------------------------



  //MEASUREMENT USING BATTERY WHEN NO GRID IS OPTED---------------------------------------
  if(Grid_Voltage == 0 && NoGrid_con == 1 && vin > 1 && (outputrem == 0 || outputrem == 1))
  {
  lcd2.clear();
  lcd2.print("BATTERY ON FOR");
  lcd2.setCursor(0,1);
  lcd2.print("MEASUREMENT!!");
  digitalWrite(ac_b3, HIGH);
  digitalWrite(dc_b3, HIGH); 
  delay(2000);
  lcd2.clear();
  }

  
  //LOAD CHECKING------------------------------------------------------------------------
  delay(3000);
  lcd2.clear();
  sensor1.setZeroPoint(setpoint_sel());
  delay(1500);
  float average1 = get_c();
  pow_sum = 230*average1;
  /*lcd2.clear();
  lcd2.print("ACTUAL LOAD: ");
  lcd2.setCursor(0,1);
  lcd2.print(pow_sum);
  lcd2.print(" WATTS");
  delay(3000);*/
  float powda = pow_sel(pow_sum);
  lcd2.clear();
  lcd2.print("LOAD: ");
  lcd2.setCursor(0,1);
  lcd2.print(powda);
  lcd2.print(" WATTS");
  delay(3000);
  lcd2.clear();
  //------------------------------------------------------------------------------------


  
  //INTIALIZING FUZZY INPUTS------------------------------------------------------------
  int input1 = timer;
  int input2 = int(powda);
  int input3 = int(current_Volts);
  int input4 = int(vin); 
  //------------------------------------------------------------------------------------

  //BATTERY MEASUREMNT IS OVER----------------------------------------------------------
  if (Grid_Voltage == 0 && NoGrid_con == 1 && (outputrem == 0 || outputrem == 1))
  {
  digitalWrite(ac_b3, LOW);
  digitalWrite(dc_b3, LOW);
  lcd2.clear();
  lcd2.print("MESUREMENT USING");
  lcd2.setCursor(0,1);
  lcd2.print("BATTERY OVER");
  delay(2000);
  lcd2.clear();
  }
  //-----------------------------------------------------------------------------------
  
  NoGrid_con = Grid_Voltage;// Storing the past grid value for future refference

  lcd2.clear();
  lcd2.print("Fuzzy Input:-");
  delay(2000);
  lcd2.clear();
  lcd2.print("Time: ");
  lcd2.print(input1);
  lcd2.print(":");
  lcd2.print(int(tm.Minute));
  lcd2.print(" HRS");
  lcd2.setCursor(0,1);
  lcd2.print("Load: ");
  lcd2.print(input2);
  lcd2.print(" WATTS");
  delay(3000);
  lcd2.clear();
  lcd2.print("Grid: ");
  lcd2.print(input3);
  lcd2.setCursor(0,1);
  lcd2.print("Battery: ");
  lcd2.print(vin);
  lcd2.print(" V");
  delay(3000);
  lcd2.clear();
  
  
   //SET INPUTS TO THE INPUT PARAMETERS OF THE FUZZY LOGIC
  fuzzy->setInput(1, input1);
  fuzzy->setInput(2, input2);
  fuzzy->setInput(3, input3);
  fuzzy->setInput(4, input4);

  //CREATES THE OUTPUT FROM THE SETPOINT
  fuzzy->fuzzify();

  
  //Displaying The Actual output
  //If Output is 0 - No Change in Connection
  //If Output is 1 - Connection is given to Grid
  //If Output is 2 - Connection is given to Battery
  
  //OUTPUT AFTER DEFUZZIFICATION
  float output = fuzzy->defuzzify(1);

  
  //CONNECT TO GRID
  if(output == 1)
  {
  lcd2.print("CONNECTION:");
  lcd2.setCursor(0,1);
  lcd2.print("GRID");
  digitalWrite(ac_b3, LOW);
  digitalWrite(dc_b3, LOW);
  digitalWrite(keypadled, HIGH);
  digitalWrite(startbuzz, HIGH);
  delay(100);
  digitalWrite(startbuzz, LOW);
  digitalWrite(keypadled, LOW);
  outputrem = 1;
  }

  //CONNECT TO BATTERY
  if(output == 2)
  {
  lcd2.print("CONNECTION:");
  lcd2.setCursor(0,1);
  lcd2.print("BATTERY");
  digitalWrite(ac_b3, HIGH);
  digitalWrite(dc_b3, HIGH);
  digitalWrite(keypadled, HIGH);
  digitalWrite(startbuzz, HIGH);
  delay(100);
  digitalWrite(startbuzz, LOW);
  digitalWrite(keypadled, LOW);
  delay(100);
  digitalWrite(keypadled, HIGH);
  digitalWrite(startbuzz, HIGH);
  delay(100);
  digitalWrite(startbuzz, LOW);
  digitalWrite(keypadled, LOW);
  outputrem = 2;
  }

  //NO CONNECTIONS
  if(output == 0)
  {
  lcd2.print("CONNECTION:");
  lcd2.setCursor(0,1);
  lcd2.print("NO CHANGE");

  }
  processrem +=1;
  delay(100);
   }
   
    else {
    lcd2.clear();
    if (RTC.chipPresent()) {
      lcd2.print("DS3231 STOPPED");
    } else {
      lcd2.print("DS3231");
      lcd2.setCursor(0,1);
      lcd2.print("readerror!");
    }
    delay(1000);
  }
  delay(5000);
  lcd2.clear();
}


//Function To Set Battery Connection---------------------------------------------------------------------------------------
void batcondition()
{
  lcd2.clear();
    if(vin1 >= 12 && vin2 >= 12)
  {
    digitalWrite(bat_rly_a, LOW);
    digitalWrite(bat_rly_b, LOW);
    ch = 0;
  }
  if((vin1 < vin2 && vin1 >= 11) && ch == 0 )
  {
    digitalWrite(bat_rly_a, LOW);
    digitalWrite(bat_rly_b, LOW);
  }

  if((vin2 < vin1 && vin2 >= 11) && ch == 0 )
  {
    digitalWrite(bat_rly_a, HIGH);
    digitalWrite(bat_rly_b, HIGH);
    ch = 1;
  }
  
  if( vin1 < 11 && ch == 0)
  {
    digitalWrite(bat_rly_a, HIGH);
    digitalWrite(bat_rly_b, HIGH);
    ch = 1;
  }
    if( vin2 < 11)
  {
    digitalWrite(bat_rly_a, LOW);
    digitalWrite(bat_rly_b, LOW);
    ch = 0;
  }
  if(ch == 0)
  {
  lcd2.print("BAT_1 CONNECTED");
  vin = vin1;
  }
  if(ch == 1)
  {
  lcd2.print("BAT_2 CONNECTED");
  vin = vin2;
  }
  if(vin >= 11 )
  {
   digitalWrite(batcontrol, HIGH);
  }
  if(vin < 11 )
  {
   vin = 10;
   digitalWrite(batcontrol, LOW);
  }
  delay(5000);
  lcd2.clear();
}


//FUNCTION TO READ INTEGER VALUES FROM KEYPAD---------------------------------------------------------------------------
int GetNumber()
{
   int num = 0;
   char key = keypad.getKey();
   while(key != '#')
   {
      switch (key)
      {
         case NO_KEY:
            break;

         case '0': case '1': case '2': case '3': case '4':
         case '5': case '6': case '7': case '8': case '9':
            lcd2.print(key);
            num = num * 10 + (key - '0');
            break;

         case '*':
            num = 0;
            lcd2.clear();
            break;
      }

      key = keypad.getKey();
   }

   return num;
}

//FUNCTION TO CALCULATE AC VOLTAGE-----------------------------------------------------------------------
float ac_voltage()
{
    RunningStatistics inputStats;   //Easy life lines, actual calculation of the RMS(Root Mean Square)requires a load of coding
                                    //create statistics to look at the raw test signal
  inputStats.setWindowSecs( windowLength );
  int i =0;
  while( i<10 ) {   
    Sensor = analogRead(A1);  // read the analog in value:
    inputStats.input(Sensor);  // log to Stats function
        
    if((unsigned long)(millis() - previousMillis) >= printPeriod) {
      previousMillis = millis();   // update time every second
            
 
      
      current_Volts = intercept + slope * inputStats.sigma(); //Calibartions for offset and amplitude
      current_Volts= current_Volts*(40.3231);                //Further calibrations for the amplitude
      i = i+1;
    }
  }
  current_Volts = current_Volts+100;
  return current_Volts;
}


//FUNCTION TO CALCULATE CURREBNT FLOWING THROUGH THE CIRCUIT--------------------------------------------------------
float get_c()
{
  int i=0;
  float value =0;
  for(i=0; i<10; i++)
  {
    value = sensor1.getCurrentAC();
  }
  return value;
}


//FUNCTION TO DEFINE THE LOAD BASED ON POWER MEASURED FOR THE DIFFERENT LOADS---------------------------------------
float pow_sel(float I)
{
  float i = 0;
  if(I < 20.00)
  {
  if(I <= 11.24)
  i = 0;
  else
  i = 10;
  }
  if(I > 20.00 && I < 35.00 )
  {
  i = 20;
  }
  if(I > 35.00)
  {
  i = 30;
  }
  return i;
}


//FUNCTION TO SELECT THE ZEROPOINT OF CURRENT SENSOR FOR CALIBRATION BASED ON RELAY CONNECTIONS-----------------------------------
int setpoint_sel()
{
  lcd2.clear();
  int i =0;
  int setpoint = 0;
  i = sensor1.calibrate();
  setpoint = i;
  /*if(digitalRead(ac_b3) == LOW && digitalRead(dc_b3) == LOW && digitalRead(bat_rly_a) == LOW && digitalRead(bat_rly_b) == LOW && digitalRead(batcontrol) == LOW){
  setpoint = 510;  
  }
  else if(digitalRead(ac_b3) == LOW && digitalRead(dc_b3) == LOW && digitalRead(bat_rly_a) == LOW && digitalRead(bat_rly_b) == LOW && digitalRead(batcontrol) == HIGH){
  setpoint = 508;
  }
  else if(digitalRead(ac_b3) == LOW && digitalRead(dc_b3) == LOW && digitalRead(bat_rly_a) == HIGH && digitalRead(bat_rly_b) == HIGH && digitalRead(batcontrol) == LOW){
  setpoint = 505;

  }
  else if(digitalRead(ac_b3) == LOW && digitalRead(dc_b3) == LOW && digitalRead(bat_rly_a) == HIGH && digitalRead(bat_rly_b) == HIGH && digitalRead(batcontrol) == HIGH){
  setpoint = 503;
  }
  else if(digitalRead(ac_b3) == HIGH && digitalRead(dc_b3) == HIGH && digitalRead(bat_rly_a) == LOW && digitalRead(bat_rly_b) == LOW && digitalRead(batcontrol) == LOW){
  setpoint = 510;
  }
  else if(digitalRead(ac_b3) == HIGH && digitalRead(dc_b3) == HIGH && digitalRead(bat_rly_a) == LOW && digitalRead(bat_rly_b) == LOW && digitalRead(batcontrol) == HIGH){
  setpoint = 507;
  }
  else if(digitalRead(ac_b3) == HIGH && digitalRead(dc_b3) == HIGH && digitalRead(bat_rly_a) == HIGH && digitalRead(bat_rly_b) == HIGH && digitalRead(batcontrol) == LOW){
  setpoint = 502;
  }
  else if(digitalRead(ac_b3) == HIGH && digitalRead(dc_b3) == HIGH && digitalRead(bat_rly_a) == HIGH && digitalRead(bat_rly_b) == HIGH && digitalRead(batcontrol) == HIGH){
  setpoint = 503;
  }
  else
  {
  lcd2.clear();
  lcd2.print("NOT CALIBRATED");  
  i = sensor1.calibrate();
  setpoint = 1;
  lcd2.setCursor(0,1);
  lcd2.print(i);
  delay(1000);
  }*/
  return setpoint;
}
