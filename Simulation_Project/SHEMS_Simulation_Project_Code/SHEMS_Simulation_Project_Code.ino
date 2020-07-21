
#include<Key.h> // Use to make key mapping
#include<Keypad.h> // This is library for using Phone Type Keypad
#include "ACS712.h" //This Library is used for measuring current using the ACS71205_BT current Sensor
#include <TimeLib.h> //This is The Time Library is used for reading current time
#include <DS1307RTC.h> //This library is used to interact with DS1307 RTC module
#include <Fuzzy.h> //This Library is used for creating Fuzzyset and Rules

Fuzzy *fuzzy = new Fuzzy();    // Instantiating a Fuzzy object

//Membership Function Declaration for Fuzzy Input And Output ------------------------------------------------------------------
  
  //Time
  FuzzySet *daye = new FuzzySet(5, 6, 17, 18);
  FuzzySet *night_1 = new FuzzySet(17, 18, 23, 24);
  FuzzySet *night_2 = new FuzzySet(-5, 0, 5, 6);

  //Load
  FuzzySet *light = new FuzzySet(-5, 0, 100, 101);
  FuzzySet *medium = new FuzzySet(100, 101, 200, 201);

  //Grid
  FuzzySet *no = new FuzzySet(-10, -5, 0, 1);
  FuzzySet *yes = new FuzzySet(0, 1, 10, 10);

  //Battery
  FuzzySet *high_bat = new FuzzySet(7, 8, 10, 11);
  FuzzySet *medium_bat = new FuzzySet(0, 1, 7, 8);
  FuzzySet *nil = new FuzzySet(-1, 0, 0, 1);


  //Output-
  FuzzySet *a = new FuzzySet(1, 2, 2, 3); //Connect_battery
  FuzzySet *b = new FuzzySet(0, 1, 1, 2); //Connect_Grid

//------------------------------------------------------------------------------------------------------------------------------

  //RelayPin config for the four appliances
  int ac_b1=13;
  int dc_b1=12;
  int ac_b2=11;
  int dc_b2=10;
  int ac_b3=9;
  int dc_b3=8;
  int ac_b4=7;
  int dc_b4=6;

  //battery realy config
  int bat1_rly=2;
  int bat2_rly=3;
  int batcontrol=5;

 //AnalogInput from voltage divider circuits to measure battery status
  int analogInput1 = A12;
  int analogInput2 = A13;

  //Variables too calculate Battery Voltage
  float vout = 0.0;
  float vin = 0.0;
  float vout1 = 0.0;
  float vin1 = 0.0;
  float vout2 = 0.0;
  float vin2 = 0.0;
  float R1 = 100000.0;  
  float R2 = 10000.0; 
  int value = 0;
  int value1 = 0;
  int value2 = 0;

  //Variable to set Grid Availability and Calculate Total Power consumed
  float Grid_Voltage = 0.0;
  float pow_sum = 0.0;

  //Variables to store the battery volatges and to 
  long ch = 0;
  int batvolt2  = 0;
  int batvolt1  = 0;

  //Variables to used to store previous operation values
  long processrem = 0;
  long NoGrid_con = 0;
  int outputrem = 0;

  // Variablrs used in describing the rows and colums of the keypad
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
  
void setup() {
  
  // Set the Serial output
   //Setting Input And OutputPins  
   pinMode(analogInput1, INPUT);
   pinMode(analogInput2, INPUT);
   pinMode(ac_b1, OUTPUT);
   pinMode(dc_b1, OUTPUT);
   pinMode(ac_b2, OUTPUT);
   pinMode(dc_b2, OUTPUT);
   pinMode(ac_b3, OUTPUT);
   pinMode(dc_b3, OUTPUT);
   pinMode(ac_b4, OUTPUT);
   pinMode(dc_b4, OUTPUT);
   pinMode(bat1_rly, OUTPUT);
   pinMode(bat2_rly, OUTPUT);
   pinMode(batcontrol, OUTPUT);
   Serial.begin(9600);

   //Intially Connect to the Grid
   digitalWrite(ac_b1, LOW);
   digitalWrite(dc_b1, LOW);
   digitalWrite(ac_b2, LOW);
   digitalWrite(dc_b2, LOW);
   digitalWrite(ac_b3, LOW);
   digitalWrite(dc_b3, LOW);
   digitalWrite(ac_b4, LOW);
   digitalWrite(dc_b4, LOW);

   
   while (!Serial) ; // wait for serial
   delay(200);
   randomSeed(analogRead(0));
   Serial.println("SMART HYBRID ENERGY MANAGEMENT SYSTEM"); //Startup Message
   Serial.println("-------------------------------------------------------------------------------------");

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
  //Collecting Input
  //Provide Time,Load,Grid Voltage,Battery_Voltage as input1,input2...etc
  //Intial connect to grid
  // read the value at analog input

  //Battery1 Voltage
  delay(500);
  value1 = analogRead(analogInput1);
  vout1 = (value1 * 5.0) / 1024.0;
  vin1 = vout1 / (R2/(R1+R2));
  
 //Battery2 Voltage
  value2 = analogRead(analogInput2);
  vout2 = (value2 * 5.0) / 1024.0;
  vin2 = vout2 / (R2/(R1+R2)); 
  
  Serial.print("------------ Operation No:");
  Serial.print(processrem+1);
  Serial.println(" ------------");
  Serial.println("");
  Serial.println("------------ Time Status ------------");
  Serial.print("Ok, Time = ");
  Serial.print(int(tm.Hour));
  Serial.print(':');
  Serial.print(int(tm.Minute));
  Serial.print(':');
  Serial.print(int(tm.Second));
  Serial.print(", Date (D/M/Y) = ");
  Serial.print(int(tm.Day));
  Serial.print('/');
  Serial.print(int(tm.Month));
  Serial.print('/');
  Serial.print(tmYearToCalendar(tm.Year));
  Serial.println();
  Serial.println("");
  Serial.println("------------ Battery Status ------------");
  Serial.print("Battery 1: ");
  Serial.print(vin1);
  Serial.print(" Volts, ");
  Serial.print("\t Battery 2: ");
  Serial.print(vin2);
  Serial.println(" Volts");
  batcondition();//Setting the Battery Connection Based on Pre-Set Conditions.
  Serial.println("");
  delay(500);
    //Setting Grid Availability Based on Current Sensed
  Serial.println("------------ Grid Status ------------");
  Serial.print("Grid: ");
  if(getCurrent(A5)== 0)
  {
    if(processrem == 0)
    NoGrid_con = 1;
    
    Grid_Voltage = 0.0;
    Serial.println("No");
  }
  if(getCurrent(A5) > 0)
  {
    Grid_Voltage = 1.0;
    Serial.println("Yes");
  }
  if(Grid_Voltage == 0 && NoGrid_con == 0 && outputrem == 1 && vin > 1)
  {
    Serial.println("---------------- WARNING ----------------");
    Serial.println("The previous load isn't supported.");
    Serial.println("Please Switch some appliances off and Enter 1 on the Keypad to test");
    Serial.println("Else the operation will continue same as before !!");
    Serial.println("Enter the value on Keypad:");
    int keypadval = GetNumber();
    if( keypadval == 1)
    {
      NoGrid_con = 1;
      Serial.println("");
    }
  }
  if(Grid_Voltage == 0 && NoGrid_con == 1 && vin > 1 && (outputrem == 0 || outputrem == 1))
  {
  Serial.println("---------------- Battery is switched ON for measurement purpose ----------------");
  digitalWrite(ac_b1, HIGH);
  digitalWrite(dc_b1, HIGH);
  digitalWrite(ac_b2, HIGH);
  digitalWrite(dc_b2, HIGH);
  digitalWrite(ac_b3, HIGH);
  digitalWrite(dc_b3, HIGH);
  digitalWrite(ac_b4, HIGH);
  digitalWrite(dc_b4, HIGH);
  }
  Serial.println("");
  
  //Power Consumed By Each Element
  delay(1000);
  Serial.println("------------ Load Status ------------");
  Serial.print("Load 1 - Power: ");
  Serial.print((230*getCurrent(A1)));  
  Serial.print(" Watts, ");
  Serial.print("\t Load 2 - Power: ");
  Serial.print((230*getCurrent(A2)));  
  Serial.println(" Watts");
  Serial.print("Load 3 - Power: ");
  Serial.print((230*getCurrent(A3)));  
  Serial.print(" Watts, ");
  Serial.print("\t Load 4 - Power: ");
  Serial.print((230*getCurrent(A4)));  
  Serial.println(" Watts");
  delay(500);
  pow_sum = (230*getCurrent(A1))+(230*getCurrent(A2))+(230*getCurrent(A3))+(230*getCurrent(A4));
  Serial.print("Total Power Consumed: ");
  Serial.print(pow_sum);
  Serial.println(" Watts");
  Serial.println("");
  delay(1000);


  //Setting Input Variables of the Fuzzy Logic
  int input1 = tm.Hour;// To Set any input randomly we can also use the random fuction ( random(x,y); )
  int input2 = int(pow_sum);
  int input3 = int(Grid_Voltage);
  int input4 = int(vin); 
  
  if (Grid_Voltage == 0 && NoGrid_con == 1 && (outputrem == 0 || outputrem == 1))
  {
  Serial.println("---------------- Measurement under battery is over ! ----------------");
  digitalWrite(ac_b1, LOW);
  digitalWrite(dc_b1, LOW);
  digitalWrite(ac_b2, LOW);
  digitalWrite(dc_b2, LOW);
  digitalWrite(ac_b3, LOW);
  digitalWrite(dc_b3, LOW);
  digitalWrite(ac_b4, LOW);
  digitalWrite(dc_b4, LOW);
  }
  NoGrid_con = Grid_Voltage;// Storing the past grid value for future refference
  //Displaying Real-Time simulation Input
  Serial.println("------------ Fuzzy Input ------------");
  Serial.print("\tTime: ");
  Serial.print(input1);
  Serial.print(", Load: ");
  Serial.print(input2);
  Serial.print(", Grid: ");
  Serial.print(input3);
  Serial.print(", and Battery: ");
  Serial.println(input4);
  Serial.println("");
  
   // Set input into the Parameters of The Fuzzy Logic
  fuzzy->setInput(1, input1);
  fuzzy->setInput(2, input2);
  fuzzy->setInput(3, input3);
  fuzzy->setInput(4, input4);

  //Creates the Output from the setInput
  fuzzy->fuzzify();
  Serial.println("------------ Membership Function Pertinence ------------");
  //Displaying Input----------------------------------------------------------------------
  Serial.print("\tTime: Day-> ");
  Serial.print(daye->getPertinence());  
  Serial.print(", Night_1-> ");
  Serial.print(night_1->getPertinence());
  Serial.print(", Night_2-> ");
  Serial.println(night_2->getPertinence());

  Serial.print("\tLoad: Light-> ");
  Serial.print(light->getPertinence());
  Serial.print(", Medium-> ");
  Serial.println(medium->getPertinence());

  Serial.print("\tGrid: No-> ");
  Serial.print(no->getPertinence());
  Serial.print(", Yes-> ");
  Serial.println(yes->getPertinence());
  
  Serial.print("\tBattery_Status: High_Bat-> ");
  Serial.print(high_bat->getPertinence());
  Serial.print(", Medium_Bat-> ");
  Serial.print(medium_bat->getPertinence());
  Serial.print(", Nil-> ");
  Serial.println(nil->getPertinence());
//------------------------------------------------------------------------------------------------------------------------------------------
 
  float output = fuzzy->defuzzify(1);
  //Deriving Output Value
  
  //Displaying The Actual output
  //If Output is 0 - No Change in Connection
  //If Output is 1 - Connection is given to Grid
  //If Output is 2 - Connection is given to Battery
  Serial.println("");
  Serial.println("------------ Fuzzy Output ------------");
  Serial.print("\t Connection:- ");
  if(output == 1)
  {
  Serial.println("GRID");
  digitalWrite(ac_b1, LOW);
  digitalWrite(dc_b1, LOW);
  digitalWrite(ac_b2, LOW);
  digitalWrite(dc_b2, LOW);
  digitalWrite(ac_b3, LOW);
  digitalWrite(dc_b3, LOW);
  digitalWrite(ac_b4, LOW);
  digitalWrite(dc_b4, LOW);
  outputrem = 1;
  }
  if(output == 2)
  {
  Serial.println("Battery");
  digitalWrite(ac_b1, HIGH);
  digitalWrite(dc_b1, HIGH);
  digitalWrite(ac_b2, HIGH);
  digitalWrite(dc_b2, HIGH);
  digitalWrite(ac_b3, HIGH);
  digitalWrite(dc_b3, HIGH);
  digitalWrite(ac_b4, HIGH);
  digitalWrite(dc_b4, HIGH);
  outputrem = 2;
  }
  if(output == 0)
  Serial.println("No Change");
  
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  processrem +=1;
  delay(100);
   }
    else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
    delay(1000);
  }
  delay(3000);
}
//Function to calculate current
float getCurrent(int _pin) {
  float average = 0;
  ACS712 sensor(ACS712_30A, _pin );
  sensor.calibrate();
  average = sensor.getCurrentAC();
return average;
}

//Function To Set Battery Connection
void batcondition()
{

    if(vin1 >= 10 && vin2 >= 10)
  {
    digitalWrite(bat2_rly, LOW);
    digitalWrite(bat1_rly, HIGH);
    ch = 0;
  }
  if((vin1 < vin2 && vin1 > 1) && ch == 0 )
  {
    digitalWrite(bat2_rly, LOW);
    digitalWrite(bat1_rly, HIGH);
  }

  if((vin2 < vin1 && vin2 > 1) && ch == 0 )
  {
    digitalWrite(bat2_rly, HIGH);
    digitalWrite(bat1_rly, LOW);
    ch = 1;
  }
  
  if( vin1 < 1 && ch == 0)
  {
    digitalWrite(bat2_rly, HIGH);
    digitalWrite(bat1_rly, LOW);
    ch = 1;
  }
    if( vin2 < 1)
  {
    digitalWrite(bat2_rly, LOW);
    digitalWrite(bat1_rly, HIGH);
    ch = 0;
  }
  if(ch == 0)
  {
  Serial.println("Battery 1 is connected !!");
  vin = vin1;
  }
  if(ch == 1)
  {
  Serial.println("Battery 2 is connected !!");
  vin = vin2;
  }
  if(vin > 1 )
  {
   digitalWrite(batcontrol, HIGH);
  }
  delay(2000);
}
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
            //lcd.print(key);
            Serial.print(key);
            num = num * 10 + (key - '0');
            break;

         case '*':
            num = 0;
            //lcd.clear();
            break;
      }

      key = keypad.getKey();
   }

   return num;
}
