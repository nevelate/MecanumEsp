#define MAX_SPEED 170   // максимальная скорость моторов (0-255)

#define MOTOR_TEST 0    // тест моторов
// при запуске крутятся ВПЕРЁД по очереди:
// FL - передний левый
// FR - передний правый
// BL - задний левый
// BR - задний правый

// пины драйверов (_B должен быть ШИМ)
#define MOTOR1_A 2
#define MOTOR1_B 3  // ШИМ!

#define MOTOR2_A 4
#define MOTOR2_B 5  // ШИМ!

#define MOTOR3_A 7
#define MOTOR3_B 6  // ШИМ!

#define MOTOR4_A 8
#define MOTOR4_B 9  // ШИМ!

#include <GyverMotor2.h>

GyverMotor2<GM2::DIR_PWM_INV> motorBL(MOTOR1_A, MOTOR1_B);
GyverMotor2<GM2::DIR_PWM_INV> motorFL(MOTOR2_A, MOTOR2_B);
GyverMotor2<GM2::DIR_PWM_INV> motorFR(MOTOR3_A, MOTOR3_B);
GyverMotor2<GM2::DIR_PWM_INV> motorBR(MOTOR4_A, MOTOR4_B);

void setup() {
  Serial.begin(115200);
  
#if (MOTOR_TEST == 1)
  Serial.println("front left");
  motorFL.runSpeed(100);
  delay(3000);
  motorFL.stop();
  delay(1000);
  Serial.println("front right");
  motorFR.runSpeed(100);
  delay(3000);
  motorFR.stop();
  delay(1000);
  Serial.println("back left");
  motorBL.runSpeed(100);
  delay(3000);
  motorBL.stop();
  delay(1000);
  Serial.println("back right");
  motorBR.runSpeed(100);
  delay(3000);
  motorBR.stop();
#endif
  // минимальный сигнал на мотор
  motorFR.setMinDuty(30);
  motorBR.setMinDuty(30);
  motorFL.setMinDuty(30);
  motorBL.setMinDuty(30);  

  // скорость плавности
  motorFR.setAccel(60);
  motorBR.setAccel(60);
  motorFL.setAccel(60);
  motorBL.setAccel(60);
}

void loop() {  
  if (success) {
    // переводим диапазон 0..255 в -MAX_SPEED..MAX_SPEED
    int valLX = map(ps2x.Analog(PSS_LX), 0, 256, -MAX_SPEED, MAX_SPEED);
    int valLY = map(ps2x.Analog(PSS_LY), 256, 0, -MAX_SPEED, MAX_SPEED); // инвертируем
    int valRX = map(ps2x.Analog(PSS_RX), 0, 256, -MAX_SPEED, MAX_SPEED);
    int valRY = map(ps2x.Analog(PSS_RY), 256, 0, -MAX_SPEED, MAX_SPEED); // инвертируем

    int dutyFR = valLY + valLX;
    int dutyFL = valLY - valLX;
    int dutyBR = valLY - valLX;
    int dutyBL = valLY + valLX;

    dutyFR += valRY - valRX;
    dutyFL += valRY + valRX;
    dutyBR += valRY - valRX;
    dutyBL += valRY + valRX;

    // ПЛАВНЫЙ контроль скорости, защита от рывков
    motorFR.setSpeed(dutyFR);
    motorBR.setSpeed(dutyBR);
    motorFL.setSpeed(dutyFL);
    motorBL.setSpeed(dutyBL);
  } else {
    // страшно, вырубай
    motorFR.setSpeed(0);
    motorBR.setSpeed(0);
    motorFL.setSpeed(0);
    motorBL.setSpeed(0);
  }
  delay(50);
}
