#include<LiquidCrystal_I2C.h>
#include<Wire.h>
#include <Servo.h>
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
  return distance;
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
void moveServo(int pos, byte servo_id) {
  //if (Serial.available()) {
  //char cmd=Serial.read();
  //pos=Serial.parseInt();
  if ((Entr_servo_crr != pos && servo_id == ENTR_SERVO) || (Ext_servo_crr != pos && servo_id == EXT_SERVO)) {
    Serial.print("Servo id :");
    Serial.print(servo_id);
    Serial.print(" Servo pos :");
    Serial.println(pos);
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
void raise_barrier_routine(long distance_in, byte lower_tresh, byte upper_tresh, byte servo_id ) {
  if (distance_in >= lower_tresh && distance_in <= upper_tresh) {
    if (servo_id == ENTR_SERVO&&car_in_back_of_entry_servo_barrier==false) {
      car_in_front_of_entry_servo_barrier = true;
      if (park_cnt > 0 && Number_Valid == true) {
        TrafficLights_ON();
        state_entr = 1;
      }
      else {
        TrafficLights_OFF;
        state_entr = 0;
      }
    }
    else if (servo_id == EXT_SERVO&&car_in_back_of_ext_servo_barrier==false) {
      car_in_front_of_ext_servo_barrier = true;
      state_ext = 1;
    }

  } else {
    if (servo_id == ENTR_SERVO)
      car_in_front_of_entry_servo_barrier = false;
    else if (servo_id == EXT_SERVO)
      car_in_front_of_ext_servo_barrier = false;
  }
  
  if (car_in_front_of_entry_servo_barrier == true&&servo_id == ENTR_SERVO)
    if (!(park_cnt <= 0 && servo_id == ENTR_SERVO))
      moveServo(BARRIER_RAISED, ENTR_SERVO);
  if(car_in_front_of_ext_servo_barrier == true&&servo_id == EXT_SERVO)
    if(  !(park_cnt >= MAX_CNT_PARK && servo_id == EXT_SERVO))
    moveServo(BARRIER_RAISED, EXT_SERVO);

  if (Number_Valid == false)
    access_denied_flag = 1;
  else
    access_denied_flag = 0;
  if (park_cnt < 1)
    parking_full_flag = 1;
  else
    parking_full_flag = 0;
}
void lower_barrier_routine(long distance_in, byte lower_tresh, byte upper_tresh, byte servo_id) {

  if (distance_in >= lower_tresh && distance_in <= upper_tresh) {
    if (servo_id == ENTR_SERVO && car_in_front_of_entry_servo_barrier==false ) {
      car_in_back_of_entry_servo_barrier=true;
      if (state_entr == 1) {
        if (park_cnt > 0)
          park_cnt--;
        TrafficLights_OFF();
      }
      state_entr = 0;

    } else if (servo_id == EXT_SERVO&&car_in_front_of_ext_servo_barrier==false) {
      car_in_back_of_ext_servo_barrier=true;
      if (state_ext == 1) {
        if (park_cnt < MAX_CNT_PARK)
          park_cnt++;
      }
      state_ext = 0;
    }
  }   else {
    if (servo_id == ENTR_SERVO)
      car_in_back_of_entry_servo_barrier = false;
    else if (servo_id == EXT_SERVO)
      car_in_back_of_ext_servo_barrier = false;
  }
  Serial.print("back entry flag: ");
  Serial.print(car_in_back_of_entry_servo_barrier);
    Serial.print("back ext flag: ");
  Serial.println(car_in_back_of_ext_servo_barrier);
    if (car_in_back_of_entry_servo_barrier == true&&servo_id == ENTR_SERVO||car_in_back_of_ext_servo_barrier == true&&servo_id == EXT_SERVO)
      moveServo(BARRIER_DOWN, servo_id);

}
void parking_entry_detect_routine() {

  dist_entr = getSensorDistance(trigPin1, echoPin1);
  raise_barrier_routine(dist_entr, 2, 5, ENTR_SERVO);

  dist_entr = getSensorDistance(trigPin2, echoPin2);
  lower_barrier_routine(dist_entr, 2, 5, ENTR_SERVO);
}
void parking_exit_detect_routine() {

  dist_ext = getSensorDistance(trigPin3, echoPin3);
  raise_barrier_routine(dist_ext, 2, 5, EXT_SERVO);

  dist_ext = getSensorDistance(trigPin4, echoPin4);
  lower_barrier_routine(dist_ext, 2, 5, EXT_SERVO);

}
void lcd_clear_lines() {
  str = "                    "; //20 char
  for (int index = 0; index < 4; index++)
    print_lcd_line(str, 0, index);
}
void print_to_lcd() {
  if (display_print_cnt < display_print_cnt_max) // de mutat toate afisarile in o singura functie
    display_print_cnt++;
  else {
    //lcd.clear();//sau
    lcd_clear_lines();
    display_print_cnt = 0;
#if false
    str = "Locuri disp: " + String(park_cnt);
    print_lcd_line(str, 0, 0);
    str = "Dist sensor1: " + String(ENT_SERV_S1_avg) + " cm";
    print_lcd_line(str, 0, 1);
    str = "Dist sensor2: " + String(ENT_SERV_S2_avg) + " cm";
    print_lcd_line(str, 0, 2);
#endif
    str = "Team Rocket Parking";
    print_lcd_line(str, 0, 0);
    str = "Locuri libere: " + String(park_cnt);
    print_lcd_line(str, 0, 1);
    str = "Orar parcare 06-23";
    print_lcd_line(str, 0, 2);
    str = "Bine ati venit!";

    if (state_entr == 1)
      str = "ACCES PERMIS!";
    if (access_denied_flag == 1)
      str = "ACCES RESPINS!";
    if (parking_full_flag == 1)
      str = "PARCARE PLINA!";
    print_lcd_line(str, 0, 3);
  }
}
void loop()
{
  parking_entry_detect_routine();
  parking_exit_detect_routine();
  print_to_lcd();
} //to do workaround to floating point operations for buffers
