#include <ESP32Servo.h>

#define TRIG_PIN   33
#define ECHO_PIN   32
#define SERVO_PIN  25

#define IN1 18
#define IN2 19
#define IN3 21
#define IN4 22
#define ENA 5
#define ENB 17

Servo scanServo;

const int LEFT_SPEED = 180;
const int RIGHT_SPEED = 160;

const int OBSTACLE_DISTANCE = 12;

const int CENTER_ANGLE = 90;

const unsigned long MAX_ROW_TIME = 2200;

const int TURN_90_TIME = 800;

const int SHIFT_TIME = 300;

unsigned long rowStartTime = 0;

bool turnRightAtRowEnd = true;

void stopCar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, LEFT_SPEED);
  analogWrite(ENB, RIGHT_SPEED);
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, LEFT_SPEED);
  analogWrite(ENB, RIGHT_SPEED);
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, LEFT_SPEED);
  analogWrite(ENB, RIGHT_SPEED);
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, LEFT_SPEED);
  analogWrite(ENB, RIGHT_SPEED);
}

long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return 999;
  }

  long distance = duration * 0.034 / 2;
  return distance;
}

long readDistanceAverage(int samples = 3) {
  long sum = 0;
  int validCount = 0;

  for (int i = 0; i < samples; i++) {
    long d = readDistanceCM();

    if (d > 0 && d < 500) {
      sum += d;
      validCount++;
    }

    delay(30);
  }

  if (validCount == 0) {
    return 999;
  }

  return sum / validCount;
}

void rotateLeft90() {
  turnLeft();
  delay(TURN_90_TIME);

  stopCar();
  delay(180);
}

void rotateRight90() {
  turnRight();
  delay(TURN_90_TIME);

  stopCar();
  delay(180);
}

void goToNextRow() {
  Serial.println("=== END OF ROW ===");

  stopCar();
  delay(250);

  moveBackward();
  delay(250);

  stopCar();
  delay(200);

  if (turnRightAtRowEnd) {
    rotateRight90();

    moveForward();
    delay(SHIFT_TIME);

    stopCar();
    delay(200);

    rotateRight90();
  } 
  else {
    rotateLeft90();

    moveForward();
    delay(SHIFT_TIME);

    stopCar();
    delay(200);

    rotateLeft90();
  }

  turnRightAtRowEnd = !turnRightAtRowEnd;
  rowStartTime = millis();
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  scanServo.setPeriodHertz(50);
  scanServo.attach(SERVO_PIN, 500, 2400);

  scanServo.write(CENTER_ANGLE);

  stopCar();
  delay(1000);

  rowStartTime = millis();
}

void loop() {
  scanServo.write(CENTER_ANGLE);
  delay(120);

  long frontDistance = readDistanceAverage(3);
  unsigned long currentRowTime = millis() - rowStartTime;

  Serial.print("Front: ");
  Serial.print(frontDistance);
  Serial.print(" cm | Row time: ");
  Serial.println(currentRowTime);

  if (frontDistance > OBSTACLE_DISTANCE && currentRowTime < MAX_ROW_TIME) {
    moveForward();
    delay(80);
    return;
  }

  goToNextRow();
}