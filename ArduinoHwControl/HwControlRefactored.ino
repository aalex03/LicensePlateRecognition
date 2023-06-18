#include<LiquidCrystal_I2C.h>
#include<Wire.h>
#include <Servo.h>
#include <string.h>
#define trigPin1 11
#define echoPin1 10
#define trigPin2 9
#define echoPin2 8
#define trigPin3 A1//7 
#define echoPin3 A2//6 
#define trigPin4 5
#define echoPin4 4
#define LED_RED 13
#define LED_GREEN 12
#define servoEntrPin 3
#define servoExtPin 2
#define BARRIER_DOWN 75
#define BARRIER_RAISED 150
#define ENTR_SERVO 0x00
#define EXT_SERVO 0x01
#define MAX_CNT_PARK 10
#define DIST_SPIKE_FILTER 100
#define BOTTOM_DIST_TRESH 2
#define TOP_DIST_TRESH 20
#define CAR_PRESENT_ENT_S1 1
#define CAR_PRESENT_EXT_S1 2
#define CAR_PRESENT_ENT_S2 3
#define CAR_PRESENT_EXT_S2 4
#define BOTTOM_PHOTO_TRESH 10
#define TOP_PHOTO_TRESH 15 // use 12,13 in real time
#define CAR_ENTERED_PARKING 0x01
#define CAR_EXITED_PARKING 0x02
int pos = 0;
int park_cnt = 5;
long dist_entr = 0, dist_ext = 0;
String str, str_l0, str_l1, str_l2, str_l3;
String LicenseNumber="";
bool Number_Valid = true;
bool car_in_front_of_entry_servo_barrier = false, car_in_front_of_ext_servo_barrier = false;
bool car_in_back_of_entry_servo_barrier = false, car_in_back_of_ext_servo_barrier = false;
int Entr_servo_crr = BARRIER_DOWN, Ext_servo_crr = BARRIER_DOWN;
byte access_denied_flag = 0x00;
byte parking_full_flag = 0x00;
byte state_entr = 0x00, state_ext = 0x00;
byte display_print_cnt = 0;
const byte display_print_cnt_max = 10;
bool trafficLightsON = false;
byte CounterStatus = 0x00;
bool photo_taken_flag=false;
bool LicenseNumberValid=false;
Servo servo_entr, servo_ext;
LiquidCrystal_I2C lcd(0x27, 20, 4);

//flags for the main control routine
bool SetupRequested = false;
bool InitRequested = false;
bool CheckCarRequested = false;
bool CheckBarrierEntryRequested = false;
bool CheckBarrierExitRequested = false;
bool OpengateEntryRequested = false;
bool OpengateExitRequested = false;
bool DisplayRequested = false;
bool CounterRequested = false;
bool LicenseReadRequested = false;
bool LicenseInvalidRequested = false;

// vars for communication

void setup()
{
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);
  pinMode(trigPin4, OUTPUT);
  pinMode(echoPin4, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  servo_entr.attach(servoEntrPin);
  servo_entr.write(BARRIER_DOWN);
  servo_ext.attach(servoExtPin);
  servo_ext.write(BARRIER_DOWN);
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  TrafficLights_OFF();
  str_l0 = "Team Rocket Parking";
  str_l1 = "Locuri libere: " + String(park_cnt);
  str_l2 = "Orar parcare 06-23";
  str_l3 = "Bine ati venit!";
  print_to_lcd();
}
void setup_main() {
  str_l0 = "";
  str_l1 = "";
  str_l2 = "";
  str_l3 = "";
  TrafficLights_OFF();
  car_in_front_of_entry_servo_barrier = false;
  car_in_front_of_ext_servo_barrier = false;
  car_in_back_of_entry_servo_barrier = false;
  car_in_back_of_ext_servo_barrier = false;
  str_l0 = "Team Rocket Parking";
  str_l1 = "Locuri libere: " + String(park_cnt);
  str_l2 = "Orar parcare 06-23";
  str_l3 = "Bine ati venit!";
}
//Main functions of the control routine
void Init() {
  TrafficLights_OFF();
  if(photo_taken_flag==true)
        str_l3 = "ACCES RESPINS!";
  print_to_lcd();//temp hack, to update states in main CU later
  access_denied_flag = 0x00;
  parking_full_flag = 0x00;
  state_entr = 0x00;
  state_ext = 0x00;
  trafficLightsON = false;
  CounterStatus = 0x00;
  car_in_front_of_entry_servo_barrier = false;
  car_in_front_of_ext_servo_barrier = false;
  car_in_back_of_entry_servo_barrier = false;
  car_in_back_of_ext_servo_barrier = false;
  photo_taken_flag=true;
}
void CheckCar() {
  byte car_present_ent_s1_val = 0x00, car_present_ext_s1_val = 0x00, car_present_ent_s2_val = 0x00, car_present_ext_s2_val = 0x00;

  car_present_ent_s1_val = check_car_detect(trigPin1, echoPin1, BOTTOM_DIST_TRESH, TOP_DIST_TRESH, ENTR_SERVO);
  car_present_ent_s2_val = check_car_detect(trigPin3, echoPin3, BOTTOM_DIST_TRESH, TOP_DIST_TRESH, EXT_SERVO);
  car_present_ext_s1_val = check_car_detect(trigPin2, echoPin2, BOTTOM_DIST_TRESH, TOP_DIST_TRESH, ENTR_SERVO);
  car_present_ext_s2_val = check_car_detect(trigPin4, echoPin4, BOTTOM_DIST_TRESH, TOP_DIST_TRESH, EXT_SERVO);
#if false
  Serial.print("car_in_front_of_entry_servo_barrier:"); Serial.print(car_in_front_of_entry_servo_barrier);
  Serial.print(" car_in_back_of_entry_servo_barrier:"); Serial.println(car_in_back_of_entry_servo_barrier);
  Serial.print("car_in_front_of_ext_servo_barrier:"); Serial.print(car_in_front_of_ext_servo_barrier);
  Serial.print(" car_in_back_of_ext_servo_barrier:"); Serial.println(car_in_back_of_ext_servo_barrier);
  Serial.println();
  Serial.println();
#endif
  //Serial.print("CAR_ent: ");Serial.println(car_present_ent_val);
  // Serial.print("CAR_ext: ");Serial.println(car_present_ext_val);
  if (car_present_ent_s1_val == CAR_PRESENT_ENT_S1) {
    transmitData("ENT");
  }
  else if (car_present_ent_s2_val == CAR_PRESENT_ENT_S2) {
    transmitData("EXT");
  }
  else
    transmitData("NO_CAR_DETECTED");

}
void CheckBarrierEntry() {
  long dist_check = getSensorDistance(trigPin1, echoPin1);
  if (car_in_back_of_entry_servo_barrier == false && park_cnt > 0 && dist_check >= BOTTOM_PHOTO_TRESH && dist_check <= TOP_PHOTO_TRESH) {
    blink_red_led();
    transmitData("TAKE_PHOTO_1");
    photo_taken_flag=true;
    state_entr = 1;
    trafficLightsON = true;
  } else {
    if (park_cnt < 1)
      parking_full_flag = true;
    transmitData("TAKE_PHOTO_0");
  }

  //Update display line
  str_l0 = "Team Rocket Parking";
  str_l1 = "Locuri libere: " + String(park_cnt);
  str_l2 = "Orar parcare 06-23";
  if (state_entr == 1)
    str_l3 = "ACCES PERMIS!";
  if (access_denied_flag == 1)
    str_l3 = "ACCES RESPINS!";
  if (parking_full_flag == 1)
    str_l3 = "PARCARE PLINA!";
}
void CheckBarrierExit() {
  if (car_in_back_of_ext_servo_barrier == false)
    transmitData("CAR_EXIT_1");
  else
    transmitData("CAR_EXIT_0");
}
void OpengateEntry() {
  long dist1_val, dist2_val;
  moveServo(BARRIER_RAISED, ENTR_SERVO);
  while (car_in_back_of_entry_servo_barrier != true) { // wait for car to enter or leave
#if false
    dist1_val = getSensorDistance(trigPin1, echoPin1);
    dist2_val = getSensorDistance(trigPin2, echoPin2);

    if (dist2_val >= BOTTOM_DIST_TRESH && dist2_val <= TOP_DIST_TRESH) {
      car_in_back_of_entry_servo_barrier = true;
    }
    else {
      car_in_back_of_entry_servo_barrier = false;
    }
#endif
    check_car_detect(trigPin1, echoPin1, BOTTOM_DIST_TRESH, TOP_DIST_TRESH, ENTR_SERVO);
    check_car_detect(trigPin3, echoPin3, BOTTOM_DIST_TRESH, TOP_DIST_TRESH, EXT_SERVO);
    check_car_detect(trigPin2, echoPin2, BOTTOM_DIST_TRESH, TOP_DIST_TRESH, ENTR_SERVO);
    check_car_detect(trigPin4, echoPin4, BOTTOM_DIST_TRESH, TOP_DIST_TRESH, EXT_SERVO);
  }
  if (car_in_back_of_entry_servo_barrier) {
    transmitData("ENTERED_PARKING");
    CounterStatus = CAR_ENTERED_PARKING;
  }
  else
    transmitData("LEFT_PARKING");
  moveServo(BARRIER_DOWN, ENTR_SERVO);
}
void OpengateExit() {
  long dist3_val, dist4_val;
  moveServo(BARRIER_RAISED, EXT_SERVO);
  while (car_in_back_of_ext_servo_barrier != true) { // wait for car to enter or leave
#if false
    dist3_val = getSensorDistance(trigPin3, echoPin3);
    dist4_val = getSensorDistance(trigPin4, echoPin4);

    if (dist4_val >= BOTTOM_DIST_TRESH && dist4_val <= TOP_DIST_TRESH) {
      car_in_back_of_ext_servo_barrier = true;
    }
    else {
      car_in_back_of_ext_servo_barrier = false;
    }
#endif
    check_car_detect(trigPin1, echoPin1, BOTTOM_DIST_TRESH, TOP_DIST_TRESH, ENTR_SERVO);
    check_car_detect(trigPin3, echoPin3, BOTTOM_DIST_TRESH, TOP_DIST_TRESH, EXT_SERVO);
    check_car_detect(trigPin2, echoPin2, BOTTOM_DIST_TRESH, TOP_DIST_TRESH, ENTR_SERVO);
    check_car_detect(trigPin4, echoPin4, BOTTOM_DIST_TRESH, TOP_DIST_TRESH, EXT_SERVO);
  }
  if (car_in_back_of_ext_servo_barrier) {
    transmitData("EXITED_PARKING");
    CounterStatus = CAR_EXITED_PARKING;
  }
  else
    transmitData("RETURNED_PARKING");
  moveServo(BARRIER_DOWN, EXT_SERVO);

}
void Display() {
  str_l0 = "Team Rocket Parking";
  str_l1 = "Locuri libere: " + String(park_cnt);

  
  if(LicenseNumberValid)
  str_l2 = "Nr. inmat: "+LicenseNumber;
  else{
  str_l2 = "Nr. inmat invalid";
  str_l3 = "ACCES RESPINS!";
  }
  
  print_to_lcd();
  if (trafficLightsON == true)
    TrafficLights_ON();
  else
    TrafficLights_OFF();
}
void Counter() {
  if (CounterStatus == CAR_ENTERED_PARKING) {
    if (park_cnt > 0)
      park_cnt--;
  }
  else if (CounterStatus == CAR_EXITED_PARKING)
    if (park_cnt < MAX_CNT_PARK)
      park_cnt++;
  //Serial.print("CntStatus:"); Serial.print(CounterStatus); Serial.print("Cnt:"); Serial.print(park_cnt);
  str_l0 = "Team Rocket Parking";
  str_l1 = "Locuri libere: " + String(park_cnt);
  str_l2 = "Orar parcare 06-23";
  str_l3 = "Bine ati venit!";
  trafficLightsON = false;
}
void LicenseRead(){
  LicenseNumberValid=true;
}
void LicenseInvalid(){
  LicenseNumberValid=false;
}

//Communication interfaces
void transmitData(String data) {

  //print_lcd_line(data, 0, 1);//good for debugging signals
  Serial.println(data);
}

void processMessage(String opcode) {
  //String opcode = message.substring(0, message.indexOf(':'));
  //char opcode[256];
  //sprintf(opcode,"%s",message);
  //Serial.println(opcode);
  //Serial.println(message);
  //good for debugging signals
  //lcd.clear();
  // print_lcd_line(opcode, 0, 0);
  String opcode_license;
  strncpy(opcode_license.c_str(),opcode.c_str(),8);
  opcode_license[8]='\0';
 
  if (!strcmp(opcode.c_str(), "SETUP"))
    SetupRequested = true;
  if (!strcmp(opcode.c_str(), "INIT"))
    InitRequested = true;
  if (!strcmp(opcode.c_str(), "CHECK_CAR")) // if (opcode.equals("CHECK_CAR"))
    CheckCarRequested = true;
  if (!strcmp(opcode.c_str(), "CHECK_BARRIER_ENTRY"))
    CheckBarrierEntryRequested = true;
  if (!strcmp(opcode.c_str(), "CHECK_BARRIER_EXIT"))
    CheckBarrierExitRequested = true;
  if (!strcmp(opcode.c_str(), "OPENGATE_ENT"))
    OpengateEntryRequested = true;
  if (!strcmp(opcode.c_str(), "OPENGATE_EXT"))
    OpengateExitRequested = true;
  if (!strcmp(opcode.c_str(), "DISPLAY"))
    DisplayRequested = true;
  if (!strcmp(opcode.c_str(), "COUNTER"))
    CounterRequested = true;
  if (!strcmp(opcode_license.c_str(), "LICENSE:")){
    LicenseReadRequested = true;
    LicenseNumber=opcode.c_str()+8;
  }
  if (!strcmp(opcode.c_str(), "LICENSE_INVALID"))
    LicenseInvalidRequested = true;
    
}

void processRequests() {

  if (SetupRequested) {
    setup_main();
    SetupRequested = false;
  }
  if (InitRequested) {
    Init();
    InitRequested = false;
  }
  if (CheckCarRequested) {
    CheckCar();
    CheckCarRequested = false;
  }
  if (CheckBarrierEntryRequested) {
    CheckBarrierEntry();
    CheckBarrierEntryRequested = false;
  }
  if (CheckBarrierExitRequested) {
    CheckBarrierExit();
    CheckBarrierExitRequested = false;
  }
  if (OpengateEntryRequested) {
    OpengateEntry();
    OpengateEntryRequested = false;
  }
  if (OpengateExitRequested) {
    OpengateExit();
    OpengateExitRequested = false;
  }
  if (DisplayRequested) {
    Display();
    DisplayRequested = false;
  }
  if (CounterRequested) {
    Counter();
    CounterRequested = false;
  }
  if(LicenseReadRequested){
    LicenseRead();
    LicenseReadRequested=false;
  }
  if(LicenseInvalidRequested){
    LicenseInvalid();
    LicenseInvalidRequested=false;
  }
}
//Auxiliary functions
void TrafficLights_ON()
{
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);
}

void TrafficLights_OFF()
{
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);
}
void blink_red_led(){
  int delay_blink=120;
  digitalWrite(LED_RED, LOW);
  delay(delay_blink);
  digitalWrite(LED_RED, HIGH);
      delay(delay_blink);
    digitalWrite(LED_RED, LOW);
        delay(delay_blink);
  digitalWrite(LED_RED, HIGH);
    delay(delay_blink);
    digitalWrite(LED_RED, LOW);
        delay(delay_blink);
  digitalWrite(LED_RED, HIGH);
  delay(delay_blink);
}

byte print_lcd_line(String str_in, byte disp_row, byte disp_col) {
  byte lcd_fault = 0x00; //default
  if (!(disp_row < 20 && disp_col < 4))
    lcd_fault = 0x01;
  else {
    lcd.setCursor(disp_row, disp_col);
    lcd.print(str_in);
  }
  return lcd_fault;
}
void print_to_lcd() {
  lcd.clear();
  print_lcd_line(str_l0, 0, 0);
  print_lcd_line(str_l1, 0, 1);
  print_lcd_line(str_l2, 0, 2);
  print_lcd_line(str_l3, 0, 3);
#if false
  if (display_print_cnt < display_print_cnt_max) // de mutat toate afisarile in o singura functie
    display_print_cnt++;
  else {
    //lcd.clear();//sau
    lcd_clear_lines();
    display_print_cnt = 0;

    str = "Locuri disp: " + String(park_cnt);
    print_lcd_line(str, 0, 0);
    str = "Dist sensor1: " + String(ENT_SERV_S1_avg) + " cm";
    print_lcd_line(str, 0, 1);
    str = "Dist sensor2: " + String(ENT_SERV_S2_avg) + " cm";
    print_lcd_line(str, 0, 2);

    str_l0 = "Team Rocket Parking";
    print_lcd_line(str, 0, 0);
    str_l1 = "Locuri libere: " + String(park_cnt);
    print_lcd_line(str, 0, 1);
    str_l2 = "Orar parcare 06-23";
    print_lcd_line(str, 0, 2);
    str_l3 = "Bine ati venit!";

    if (state_entr == 1)
      str_l3 = "ACCES PERMIS!";
    if (access_denied_flag == 1)
      str_l3 = "ACCES RESPINS!";
    if (parking_full_flag == 1)
      str_l3 = "PARCARE PLINA!";
    print_lcd_line(str, 0, 3);
  }
#endif
}
byte check_car_detect(int trigPin, int echoPin, const byte lower_tresh, const byte upper_tresh, byte servo_id ) {
  byte check_flag = 0x00;
  long distance_in = getSensorDistance(trigPin, echoPin);
  if (distance_in >= lower_tresh && distance_in <= upper_tresh) {
    if (servo_id == ENTR_SERVO) {
      if (trigPin == trigPin1 && echoPin == echoPin1) {
        check_flag = CAR_PRESENT_ENT_S1; //ENT signal
        car_in_front_of_entry_servo_barrier = true;
      } else if (trigPin == trigPin2 && echoPin == echoPin2) {
        check_flag = CAR_PRESENT_EXT_S1;
        car_in_back_of_entry_servo_barrier = true;
      }
    }
    else if (servo_id == EXT_SERVO) {
      if (trigPin == trigPin3 && echoPin == echoPin3) {
        check_flag = CAR_PRESENT_ENT_S2; //EXT signal
        car_in_front_of_ext_servo_barrier = true;
      } else if (trigPin == trigPin4 && echoPin == echoPin4) {
        check_flag = CAR_PRESENT_EXT_S2;
        car_in_back_of_ext_servo_barrier = true;
      }
    }
  } else {
    if (servo_id == ENTR_SERVO) {
      if (trigPin == trigPin1 && echoPin == echoPin1)
        car_in_front_of_entry_servo_barrier = false;
      else if (trigPin == trigPin2 && echoPin == echoPin2)
        car_in_back_of_entry_servo_barrier = false;

    } else if (servo_id == EXT_SERVO) {
      if (trigPin == trigPin3 && echoPin == echoPin3)
        car_in_front_of_ext_servo_barrier = false;
      else if (trigPin == trigPin4 && echoPin == echoPin4)
        car_in_back_of_ext_servo_barrier = false;
    }
  }
  return check_flag;
}
long getSensorDistance(int trigPin, int echoPin)
{
  long distance, duration;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2.0) / 29.1;
  distance = distance <= DIST_SPIKE_FILTER ? distance : DIST_SPIKE_FILTER;
  //Serial.print("Trig");Serial.print(trigPin);Serial.print(" Echo");Serial.print(echoPin);Serial.print(" Dist:");Serial.print(distance);Serial.println();
  return distance;
}
void moveServo(int pos, byte servo_id) {
  //if (Serial.available()) {
  //char cmd=Serial.read();
  //pos=Serial.parseInt();
  if ((Entr_servo_crr != pos && servo_id == ENTR_SERVO) || (Ext_servo_crr != pos && servo_id == EXT_SERVO)) {
    //Serial.print("Servo id :");
    //Serial.print(servo_id);
    //Serial.print(" Servo pos :");
    //Serial.println(pos);
    if (pos >= 0 && pos <= 180) {
      switch (servo_id) {
        case ENTR_SERVO:
          servo_entr.write(pos);
          Entr_servo_crr = pos;
          break;
        case EXT_SERVO:
          servo_ext.write(pos);
          Ext_servo_crr = pos;
          break;
      }
    }
  }
}
void loop() {
  if (Serial.available()) {
    String receivedData = Serial.readStringUntil('\n');
    processMessage(receivedData);
  }
  processRequests();
}