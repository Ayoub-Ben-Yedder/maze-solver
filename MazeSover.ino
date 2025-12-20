#define IN1 5
#define IN2 6
#define IN3 10
#define IN4 11

#define TRIG 3
#define ECHO 2

#define TRIG_FRONT 13
#define ECHO_FRONT 12

// ---------- SETUP ----------
void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);

  Serial.begin(9600);
}

// ---------- MOTOR FUNCTION ----------
void run(int speedLeft, int speedRight) {
  // Right motor
  if (speedRight >= 0) {
    analogWrite(IN1, speedRight);
    analogWrite(IN2, 0);
  } else {
    analogWrite(IN1, 0);
    analogWrite(IN2, -speedRight);
  }

  // Left motor
  if (speedLeft >= 0) {
    analogWrite(IN3, speedLeft);
    analogWrite(IN4, 0);
  } else {
    analogWrite(IN3, 0);
    analogWrite(IN4, -speedLeft);
  }
}

// ---------- DISTANCE FUNCTION ----------
long readDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000); // timeout 30ms
  long distance = duration * 0.034 / 2;      // cm

  return distance;
}

long readDistanceFront() {
  digitalWrite(TRIG_FRONT, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_FRONT, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_FRONT, LOW);

  long duration = pulseIn(ECHO_FRONT, HIGH, 30000); // timeout 30ms
  long distance = duration * 0.034 / 2;      // cm

  return distance;
}

long lastD = 6;

void maze(){
  long d = readDistance();
  long dFront = readDistanceFront();
  if(dFront < 8 && dFront != 0){
    run(-80,80);
    delay(600);
    return;
  }
  
  if(d==0 || d>20){
    d = lastD;
  }

  int setpoint = 6;         
  float error = d - setpoint;

  float kp = 10.0;
  int baseSpeed = 100;

  int speedRight = baseSpeed - kp * error;
  int speedLeft  = baseSpeed + kp * error;

  speedRight = constrain(speedRight, -255, 255);
  speedLeft  = constrain(speedLeft, -255, 255);

  run(speedLeft, speedRight);

  lastD = d;
  delay(10);
}

void debugSensors(){
  long d = readDistance();
  long dFront = readDistanceFront();
  Serial.print("D: ");
  Serial.print(d);
  Serial.print("\t Dfront: ");
  Serial.println(dFront);
  delay(300);
}
// ---------- LOOP ----------
void loop() {
  // debugSensors();
  maze();
}

