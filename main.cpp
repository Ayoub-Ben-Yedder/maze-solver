#include <Arduino.h>
#include <Wire.h>

#define MPU_ADDR 0x68

float gyroZ_offset = 0; // calibration offset
long lastLeft = 10;
long lastFront = 20;
long lastRight = 10;
bool awel_marra = true;

#define IN1 32
#define IN2 33
#define IN3 17
#define IN4 16

#define TRIG_LEFT 27
#define ECHO_LEFT 26

#define TRIG_FRONT 12
#define ECHO_FRONT 13

#define TRIG_RIGHT 4
#define ECHO_RIGHT 18

int16_t readGyroZRaw()
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x47); // GYRO_ZOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true);

  int16_t gz = Wire.read() << 8 | Wire.read();
  return gz;
}

void setupMPU()
{
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // Power management
  Wire.write(0);    // Wake up MPU6050
  Wire.endTransmission(true);

  // --- Calibrate gyro Z ---
  long sum = 0;
  for (int i = 0; i < 500; i++)
  {
    sum += readGyroZRaw();
    delay(3);
  }
  gyroZ_offset = sum / 500.0;
}

void run(int speedLeft, int speedRight)
{
  if (speedRight >= 0)
  {
    analogWrite(IN3, speedRight);
    analogWrite(IN4, 0);
  }
  else
  {
    analogWrite(IN3, 0);
    analogWrite(IN4, -speedRight);
  }

  if (speedLeft >= 0)
  {
    analogWrite(IN1, speedLeft);
    analogWrite(IN2, 0);
  }
  else
  {
    analogWrite(IN1, 0);
    analogWrite(IN2, -speedLeft);
  }
}

void rotate(float targetAngle)
{
  // + ydour ymin
  run(0, 0);
  delay(500);
  float angle = 0.0;
  unsigned long lastTime = millis();

  if (targetAngle < 0)
  {
    run(-90, 90);
    //run(-90, 60);
  }
  else
  {
    run(90, -90);
    //run(60, -90);
  }

  while (abs(angle) < abs(targetAngle))
  {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0;
    lastTime = now;

    int16_t gz_raw = readGyroZRaw();
    float gz = (gz_raw - gyroZ_offset) / 131.0; // deg/sec (±250 dps)

    angle += gz * dt;
  }

  run(0, 0); // stop motors
  delay(500);
}

long readDistance(int TRIG, int ECHO)
{
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000); // timeout 30ms
  long distance = duration * 0.034 / 2;       // cm

  return distance;
}

long filteredDistance(int TRIG, int ECHO, long last)
{
  long d = readDistance(TRIG, ECHO);
  if (d == 0 || d > 150)
    return last;
  return d;
}

long movingAverageDistance(int TRIG, int ECHO, long last)
{
  const int N = 5;
  static long buffer[N];
  static long sum = 0;
  static int index = 0;
  static bool filled = false;

  long d = readDistance(TRIG, ECHO);
  if (d == 0 || d > 150)
    d = last;

  if (filled)
    sum -= buffer[index];

  buffer[index] = d;
  sum += d;

  index = (index + 1) % N;
  if (index == 0) filled = true;

  int count = filled ? N : index;
  return sum / count;
}

void followWall(long d)
{
  int setpoint = 11;//9;
  float error = d - setpoint;

  float kp = 6.0;
  int baseSpeed = 90;

  int speedRight = baseSpeed + kp * error;
  int speedLeft = baseSpeed - kp * error;

  speedRight = constrain(speedRight, -255, 255);
  speedLeft = constrain(speedLeft, -255, 255);

  run(speedLeft, speedRight);

  delay(10);
}

void followWallRight(long d)
{
  int setpoint = 11;//15;//12;
  float error = d - setpoint;

  float kp = 6.0;
  int baseSpeed = 90;

  int speedRight = baseSpeed + kp * error;
  int speedLeft = baseSpeed - kp * error;

  speedRight = constrain(speedRight, -255, 255);
  speedLeft = constrain(speedLeft, -255, 255);

  run(speedRight, speedLeft);

  delay(10);
}


void maze()
{
  long dLeft = filteredDistance(TRIG_LEFT, ECHO_LEFT, lastLeft);
  long dFront = movingAverageDistance(TRIG_FRONT, ECHO_FRONT, lastFront);
  long dRight = filteredDistance(TRIG_RIGHT, ECHO_RIGHT, lastRight);

  lastLeft = dLeft;
  lastFront = dFront;
  lastRight = dRight;
  if (dLeft > 30)
  { // left ma7loula
    if(dFront < 30) {
      // 3anna 7it goddam
      // nkamlo 7atta nousloulo
      while(dFront < 15){
        dLeft = filteredDistance(TRIG_LEFT, ECHO_LEFT, lastLeft);
        dFront = movingAverageDistance(TRIG_FRONT, ECHO_FRONT, lastFront);
        dRight = filteredDistance(TRIG_RIGHT, ECHO_RIGHT, lastRight);

        lastLeft = dLeft;
        lastFront = dFront;
        lastRight = dRight;
        if(dRight < 30){
          // 3anna 7it ymin net3awno bih bech nouslo
          followWallRight(dRight);
        }else{
          // ma3nnach donc nkamlo forward w ya rabi te5dm
          run(90,90);
        }
      }
    }else{
      // ma3annach 7it el goddam (mazel b3id)
      // ngadmo chwaya( bech nouslou fi center el cellule) w na3mlo rotate, w mba3d ngaddmo chwyaa (bech nouslou fi center el cellule elli bjnabha)
      
      // ajmo na3mlo faza bech yemchi mestwi, ye yet3awen bel 7it el ymin
      unsigned long now_fel_7asla = millis();
      while(millis() - now_fel_7asla < 400){
        dLeft = filteredDistance(TRIG_LEFT, ECHO_LEFT, lastLeft);
        dFront = movingAverageDistance(TRIG_FRONT, ECHO_FRONT, lastFront);
        dRight = filteredDistance(TRIG_RIGHT, ECHO_RIGHT, lastRight);

        lastLeft = dLeft;
        lastFront = dFront;
        lastRight = dRight;
        if(dRight < 30){
          // 3anna 7it ymin net3awno bih bech nouslo
          followWallRight(dRight);
        }else{
          // ma3nnach donc nkamlo forward w ya rabi te5dm
          run(90,90);
        }
      }
      //rotate(-90);
      rotate(-80);
      now_fel_7asla = millis();
      while(millis() - now_fel_7asla < 500){
        dLeft = filteredDistance(TRIG_LEFT, ECHO_LEFT, lastLeft);
        dFront = movingAverageDistance(TRIG_FRONT, ECHO_FRONT, lastFront);
        dRight = filteredDistance(TRIG_RIGHT, ECHO_RIGHT, lastRight);

        lastLeft = dLeft;
        lastFront = dFront;
        lastRight = dRight;
        if(dRight < 30){
            // 3anna 7it ymin net3awno bih bech nouslo
            followWallRight(dRight);
          }else{
            // ma3nnach donc nkamlo forward w ya rabi te5dm
            run(90,90);
          }
      }
    }
    return;
  }
  else if (dFront > 10)
  { // front ma7loula
    followWall(dLeft);
    return;
  }
  else if (dRight > 30)
  { // right ma7loula
    //rotate(90);
    rotate(80);
    return;
  }
  else
  { // msakra (dead-end) arja3 mnin jit
    rotate(180);
    return; 
  }
}

void debugSensors()
{
  long dLeft = filteredDistance(TRIG_LEFT, ECHO_LEFT, lastLeft);
  long dFront = movingAverageDistance(TRIG_FRONT, ECHO_FRONT, lastFront);
  long dRight = filteredDistance(TRIG_RIGHT, ECHO_RIGHT, lastRight);
  int16_t gz = readGyroZRaw();
  Serial.print("Dleft: ");
  Serial.print(dLeft);
  Serial.print("\t Dfront: ");
  Serial.print(dFront);
  Serial.print("\t Dright: ");
  Serial.print(dRight);
  Serial.print("\t GyroZ: ");
  Serial.println(gz);
  delay(300);
}

void debugActuators()
{
  run(90, 90);
  delay(1000);
  run(-90, -90);
  delay(1000);
  run(90, -90);
  delay(1000);
  run(-90, 90);
  delay(1000);

  rotate(90);
  delay(1000);
  rotate(-90);
  delay(1000);
  rotate(180);
  delay(1000);
  rotate(-180);
  delay(1000);
}

void setup()
{
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  run(0, 0);

  pinMode(TRIG_LEFT, OUTPUT);
  pinMode(ECHO_LEFT, INPUT);
  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_RIGHT, OUTPUT);
  pinMode(ECHO_RIGHT, INPUT);

  setupMPU();

  Serial.begin(115200);
}

void loop()
{
  //debugSensors();
  maze();
}
