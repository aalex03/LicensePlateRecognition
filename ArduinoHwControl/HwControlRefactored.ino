#include<LiquidCrystal_I2C.h>
#include<Wire.h>
#include <Servo.h>
#include <string.h>
#define trigPin1 11
#define echoPin1 10
#define trigPin2 9
#define echoPin2 8
#define trigPin3 7
#define echoPin3 6
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
int pos = 0;
int park_cnt = 2;
long dist_entr = 0, dist_ext = 0;
String str;
bool Number_Valid = true;
bool car_in_front_of_entry_servo_barrier = false, car_in_front_of_ext_servo_barrier = false;
bool car_in_back_of_entry_servo_barrier = false, car_in_back_of_ext_servo_barrier = false;
int Entr_servo_crr = BARRIER_DOWN, Ext_servo_crr = BARRIER_DOWN;
byte access_denied_flag = 0x00;
byte parking_full_flag = 0x00;
byte state_entr = 0x00, state_ext = 0x00;
byte display_print_cnt = 0;
const byte display_print_cnt_max = 10;
Servo servo_entr, servo_ext;
LiquidCrystal_I2C lcd(0x27, 20, 4);

//flags for the main control routine
bool SetupRequested=false;
bool InitRequested=false;
bool CheckCarRequested=false;
bool CheckBarrierEntryRequested=false;
bool CheckBarrierExitRequested=false;
bool OpengateEntryRequested=false;
bool OpengateExitRequested=false;
bool DisplayRequested=false;
bool CounterRequested=false;

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
}
//Main functions of the control routine
void Init(){

}
void CheckCar(){
    transmitData("ENT");
}
void CheckBarrierEntry(){
    transmitData("TAKE_PHOTO_1");  
}
void CheckBarrierExit(){
    transmitData("CAR_EXIT_1");      
}
void OpengateEntry(){
    transmitData("ENTERED_PARKING");      
}
void OpengateExit(){
    transmitData("EXITED_PARKING");       
}
void Display(){

}
void Counter(){
    
}

//Communication interfaces
void transmitData(String data) {
  Serial.println(data);
}

void processMessage(String message) {
  //String opcode = message.substring(0, message.indexOf(':'));
  char opcode[256];
  sprintf(opcode,"%s",message);

    if (strcmp(opcode,"SETUP")) 
     SetupRequested=true;
    if (strcmp(opcode,"INIT")) 
     InitRequested=true;
    if (strcmp(opcode,"CHECK_CAR")) 
     CheckCarRequested=true;
    if (strcmp(opcode,"CHECK_BARRIER_ENTRY"))  
     CheckBarrierEntryRequested=true;
    if (strcmp(opcode,"CHECK_BARRIER_EXIT")) 
     CheckBarrierExitRequested=true;
    if (strcmp(opcode,"OPENGATE_ENT")) 
     OpengateEntryRequested=true;
    if (strcmp(opcode,"OPENGATE_EXT")) 
     OpengateExitRequested=true;
    if (strcmp(opcode,"DISPLAY")) 
     DisplayRequested=true;
    if (strcmp(opcode,"COUNTER")) 
     CounterRequested=true;
  
}

void processRequests(){

    if (SetupRequested) {
        setup();
        SetupRequested=false;
    }
    if (InitRequested) {
        Init();
        InitRequested=false;
    }
    if (CheckCarRequested){
        CheckCar();
        CheckCarRequested=false;
    }
    if (CheckBarrierEntryRequested){
        CheckBarrierEntry();
        CheckBarrierEntryRequested=false;
    }
    if (CheckBarrierExitRequested){
        CheckBarrierExit();
        CheckBarrierExitRequested=false;
    }
    if (OpengateEntryRequested){
        OpengateEntry();
        OpengateEntryRequested=false;
    }
    if (OpengateExitRequested){
        OpengateExit();
        OpengateExitRequested=false;
    }
    if (DisplayRequested){
        Display();
        DisplayRequested=false;
    }
    if (CounterRequested) {
        Counter();
        CounterRequested=false;
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

void loop(){
    if(Serial.available()){
        String receivedData=Serial.readStringUntil('\n');
        processMessage(receivedData);
    }
    processRequests();
}

