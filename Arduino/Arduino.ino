#include "Wire.h"
#include <Servo.h>
#include "math.h"
#include <PID_v1.h>
#include <TimerOne.h>
#include <SoftwareSerial.h>   // Incluimos la librería  SoftwareSerial 

SoftwareSerial BT(5,6);    // Definimos los pines RX y TX del Arduino conectados al Bluetooth

const int D1 = A3; // Pin connected to voltage divider output
const int D2 = A2;
const int D3 = A1;
const int D4 = A0;

// Measure the voltage at 5V and the actual resistance of your
// 47k resistor, and enter them below:
const float VCC = 4.98; // Measured voltage of Ardunio 5V line
const float R_DIV = 47500.0; // Measured resistance of 3.3k resistor

// Upload the code, then try to adjust these values to more
// accurately calculate bend degree.
const float STRAIGHT_RESISTANCE = 23500.0;
const float BEND_RESISTANCE = 90000.0;

String data_send; 
int flag = 7;
int target = 0;

/*--------------------------MPU 6050-----------------------------*/
// La dirección del MPU6050 puede ser 0x68 o 0x69, dependiendo 
// del estado de AD0. Si no se especifica, 0x68 estará implicito
//MPU6050 sensor_ang;
// Valores RAW (sin procesar) del acelerometro y giroscopio en los ejes x,y,z
#define SDA 20
#define SCL 21
const int MPU_addr=0x68; // I2C address of the MPU-6050
int16_t ax, ay, az;
int16_t gx, gy, gz, Tmp;
double ax_d, ay_d, az_d;
double  ang_x, ang_y;
float ang_x_prev, ang_y_prev;
long tiempo_prev,tiempo_prev1,tiempo_prev2;
float dt;
int d_max;
float accel_ang_y,accel_ang_x; 
//time out loop count
const int timeout = 200;
void init_MPU_sensor();
void MPU_sensor();
void MPU_read();

int sta;
/*==============================SETUP=================================*/
void setup() { 
  Serial.begin(115200); //************************ IMPORTANT
  //------MPU------
  init_MPU_sensor();
  //------Timer----
  pinMode(7,INPUT);
  pinMode(D1, INPUT);
  Timer1.initialize(20000);         // Dispara cada 250 ms
  Timer1.attachInterrupt(MPU_read); // Activa la interrupcion y la asocia a ISR_Blink
}
/*==============================LOOP==================================*/
void loop() 
{
    MPU_sensor();
  //MPU_read();
}

/*====================================================================*/
void init_MPU_sensor(){
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
}
/*====================================================================*/
void MPU_sensor(){
  /* Leer las aceleraciones y velocidades angulares
  sensor_ang.getAcceleration(&ax, &ay, &az);
  sensor_ang.getRotation(&gx, &gy, &gz);
  // Correxion de Offsets*/
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true); // request a total of 14 registers
  ax=Wire.read()<<8|Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  ay=Wire.read()<<8|Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  az=Wire.read()<<8|Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  gx=Wire.read()<<8|Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  gy=Wire.read()<<8|Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  gz=Wire.read()<<8|Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  ax=ax-1277+1300;
  ay=ay+428-200;
  az=az+1723;
  gx=gx+637;
  gy=gy-319;
  gz=gz-224+35;
  dt = (millis()-tiempo_prev)/1000.0;
  tiempo_prev=millis();
  
  //Calcular los ángulos con acelerometro
  accel_ang_x=atan2(ay,sqrt(pow(ax,2) + pow(az,2)))*(180.0/3.14);
  accel_ang_y=atan2(-ax,sqrt(pow(ay,2) + pow(az,2)))*(180.0/3.14);
  
  //Calcular angulo de rotación con giroscopio y filtro complementario  
  ang_x = 0.98*(ang_x_prev+(gx/127)*dt) + 0.02*accel_ang_x;
  ang_y = 0.98*(ang_y_prev+(gy/160)*dt) + 0.02*accel_ang_y;
   
  ang_x_prev=ang_x;
  ang_y_prev=ang_y;
}

/*=============================ROLL====================================*/
void MPU_read(){
  int dt = millis()-sta;
  target = digitalRead(flag);
  sta = millis();
  
  float d1 = flex_read(D1);
  float d2 = flex_read(D2);
  float d3 = flex_read(D3);
  float d4 = flex_read(D4);
  
  data_send = String("{")+String("'ax':") + String(ax) + String(",") 
                         +String("'ay':") + String(ay) + String(",") 
                         +String("'az':") + String(az) + String(",")
                         +String("'gx':") + String(gx) + String(",")
                         +String("'gy':") + String(gy) + String(",")
                         +String("'gz':") + String(gz) + String(",")
                         +String("'d1':") + String(d1) + String(",")
                         +String("'d2':") + String(d2) + String(",")
                         +String("'d3':") + String(d3) + String(",")
                         +String("'d4':") + String(d4)
            + String("}");
  Serial.println(data_send);
}

float flex_read(int puerto){
  // Read the ADC, and calculate voltage and resistance from it
  int flexADC = analogRead(puerto);
  float flexV = flexADC * VCC / 1023.0;
  float flexR = R_DIV * (VCC / flexV - 1.0);
  float angle = map(flexR, STRAIGHT_RESISTANCE, BEND_RESISTANCE,0, 90.0);

  return angle;
}
