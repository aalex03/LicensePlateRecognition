#include<LiquidCrystal_I2C.h>
#include<Wire.h>
#include <Servo.h>
#define trigPin1 10
#define echoPin1 11
#define trigPin3 7
#define echoPin3 8
#define trigPin2 6
#define echoPin2 5
#define trigPin4 4
#define echoPin4 3
#define LED_RED 13
#define LED_GREEN 12
#define servoEntrPin 9
#define servoExtPin 2
#define BARRIER_DOWN 75
#define BARRIER_RAISED 150
#define ENTR_SERVO 0x00
#define EXT_SERVO 0x01
#define MAX_CNT_PARK 20
#define DIST_SPIKE_FILTER 100
int pos = 0;
int park_cnt = 5;
long dist_entr = 0, dist_ext = 0;
String str;
byte state_entr = 0x00, state_ext = 0x00;
const byte window_sampling_size=5;
long ENT_SERV_S1_buffer[window_sampling_size];
long ENT_SERV_S2_buffer[window_sampling_size];
long EXT_SERV_S1_buffer[window_sampling_size];
long EXT_SERV_S2_buffer[window_sampling_size];
long ENT_SERV_S1_sum=0;
long ENT_SERV_S2_sum=0;
long EXT_SERV_S1_sum=0;
long EXT_SERV_S2_sum=0;
long ENT_SERV_S1_avg = 0;
long ENT_SERV_S2_avg = 0;
long EXT_SERV_S1_avg = 0;
long EXT_SERV_S2_avg = 0;
byte buff_index = 0;
byte display_print_cnt=0;
const byte display_print_cnt_max=5;
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
  pinMode(LED_RED,OUTPUT);
  pinMode(LED_GREEN,OUTPUT);
  servo_entr.attach(servoEntrPin);
  servo_entr.write(BARRIER_DOWN);
  servo_ext.attach(servoExtPin);
  servo_ext.write(BARRIER_DOWN);
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
  TrafficLights_OFF();
}

void TrafficLights_ON()
{
  digitalWrite(LED_GREEN,HIGH);
  digitalWrite(LED_RED,LOW);
}

void TrafficLights_OFF()
{
  digitalWrite(LED_RED,HIGH);
  digitalWrite(LED_GREEN,LOW);
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
  distance=distance<=DIST_SPIKE_FILTER?distance:DIST_SPIKE_FILTER;
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
  if (pos >= 0 && pos <= 180) {
    switch (servo_id) {
      case ENTR_SERVO:
        servo_entr.write(pos);
        break;
      case EXT_SERVO:
        servo_ext.write(pos);
        break;
    }
  }
}
void raise_barrier_routine(long distance_in, byte lower_tresh, byte upper_tresh, byte servo_id ) {

    if (distance_in >= lower_tresh && distance_in <= upper_tresh) {
      if (servo_id == ENTR_SERVO){
      if (park_cnt > 0){
      TrafficLights_ON();
        state_entr = 1;
      }
      }
      else if (servo_id == EXT_SERVO){
        state_ext = 1;
    }
    if (!(park_cnt <=0&& servo_id == ENTR_SERVO))
    moveServo(BARRIER_RAISED, servo_id);
    }
   
}
void lower_barrier_routine(long distance_in, byte lower_tresh, byte upper_tresh, byte servo_id) {

    if (distance_in >= lower_tresh && distance_in <= upper_tresh) {
      if (servo_id == ENTR_SERVO) {
        if (state_entr == 1){
          if(park_cnt>0)
          park_cnt--;
          TrafficLights_OFF();
        }
        state_entr = 0;

      } else if (servo_id == EXT_SERVO) {
        if (state_ext == 1){
          if(park_cnt<MAX_CNT_PARK)
          park_cnt++;
        }
        state_ext = 0;
      }
      moveServo(BARRIER_DOWN, servo_id);
    }
  
}
void parking_entry_detect_routine() {


  str = "Locuri disp: " + String(park_cnt);
  print_lcd_line(str, 0, 0);

  dist_entr = getSensorDistance(trigPin1, echoPin1);
  ENT_SERV_S1_buffer[buff_index] = dist_entr;

  str = "Dist sensor1: " + String(ENT_SERV_S1_avg) + " cm";
  print_lcd_line(str, 0, 1);
  raise_barrier_routine(ENT_SERV_S1_avg, 2, 10, ENTR_SERVO);

  dist_entr = getSensorDistance(trigPin2, echoPin2);
  ENT_SERV_S2_buffer[buff_index] = dist_entr;
  
  str = "Dist sensor2: " + String(ENT_SERV_S2_avg) + " cm";
  print_lcd_line(str, 0, 2);
  lower_barrier_routine(ENT_SERV_S2_avg, 2, 10, ENTR_SERVO);
  
  if (state_entr == 1)
    str = "ACCES PERMIS";
  else
    str = "ACCES RESPINS";

  print_lcd_line(str, 0, 3);
  delay(200);
  lcd.clear();
 // delay(50);
}
void parking_exit_detect_routine() {

  dist_ext = getSensorDistance(trigPin3, echoPin3);
  EXT_SERV_S1_buffer[buff_index] = dist_ext;
  raise_barrier_routine(EXT_SERV_S1_avg, 2, 10, EXT_SERVO);

  dist_ext = getSensorDistance(trigPin4, echoPin4);
  EXT_SERV_S2_buffer[buff_index] = dist_ext;
  lower_barrier_routine(EXT_SERV_S2_avg, 2, 10, EXT_SERVO);

}
void compute_average_for_buffers(){
    buff_index = (buff_index + 1) % window_sampling_size;
 ENT_SERV_S1_sum=0;
ENT_SERV_S2_sum=0;
 EXT_SERV_S1_sum=0;
EXT_SERV_S2_sum=0;
      for (byte i = 0; i < window_sampling_size; i++) {
      ENT_SERV_S1_sum += ENT_SERV_S1_buffer[i];
      ENT_SERV_S2_sum += ENT_SERV_S2_buffer[i];
      EXT_SERV_S1_sum += EXT_SERV_S1_buffer[i];
      EXT_SERV_S2_sum += EXT_SERV_S2_buffer[i];
    }
    ENT_SERV_S1_avg = (long)ENT_SERV_S1_sum / window_sampling_size;
    ENT_SERV_S2_avg = (long)ENT_SERV_S2_sum / window_sampling_size;
    EXT_SERV_S1_avg = (long)EXT_SERV_S1_sum / window_sampling_size;
    EXT_SERV_S2_avg = (long)EXT_SERV_S2_sum / window_sampling_size;
    if(false){
    Serial.print("ENT S1 AVG: ");
    Serial.print(ENT_SERV_S1_avg);
    Serial.print(" ENT S2 AVG: ");
    Serial.print(ENT_SERV_S2_avg);
    Serial.print(" EXT S1 AVG: ");
    Serial.print(EXT_SERV_S1_avg);
    Serial.print(" EXT S2 AVG: ");
    Serial.println(EXT_SERV_S2_avg);
    Serial.println("");
    }
}
void loop()
{
  parking_entry_detect_routine();
  parking_exit_detect_routine();
  compute_average_for_buffers();
  if(display_print_cnt<display_print_cnt_max)
  display_print_cnt++;
  else
  display_print_cnt=0;
}
