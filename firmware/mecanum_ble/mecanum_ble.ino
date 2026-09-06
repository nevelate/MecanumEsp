#define MAX_SPEED 200  // максимальная скорость моторов (0-255)

#define MOTOR_TEST 1  // тест моторов
// при запуске крутятся ВПЕРЁД по очереди:
// FL - передний левый
// FR - передний правый
// BL - задний левый
// BR - задний правый

// пины драйверов (_B должен быть ШИМ)
#define MOTOR1_A 1
#define MOTOR1_B 0  // ШИМ!

#define MOTOR2_A 3
#define MOTOR2_B 4  // ШИМ!

#define MOTOR3_A 20
#define MOTOR3_B 21  // ШИМ!

#define MOTOR4_A 7
#define MOTOR4_B 6  // ШИМ!

#include <Bluepad32.h>
#include <GyverMotor2.h>

ControllerPtr myControllers[BP32_MAX_GAMEPADS];
ControllerPtr gamepad = nullptr;

GyverMotor2<GM2::DIR_PWM_INV> motorFL(MOTOR1_A, MOTOR1_B);
GyverMotor2<GM2::DIR_PWM_INV> motorFR(MOTOR2_A, MOTOR2_B);
GyverMotor2<GM2::DIR_PWM_INV> motorBL(MOTOR3_A, MOTOR3_B);
GyverMotor2<GM2::DIR_PWM_INV> motorBR(MOTOR4_A, MOTOR4_B);

void setup() {
  Serial.begin(115200);

  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
  const uint8_t* addr = BP32.localBdAddress();
  Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

  // Setup the Bluepad32 callbacks
  BP32.setup(&onConnectedController, &onDisconnectedController);

  // "forgetBluetoothKeys()" should be called when the user performs
  // a "device factory reset", or similar.
  // Calling "forgetBluetoothKeys" in setup() just as an example.
  // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
  // But it might also fix some connection / re-connection issues.
  // BP32.forgetBluetoothKeys();

#if (MOTOR_TEST == 1)
  Serial.println("front left");
  motorFL.runSpeed(150);
  delay(3000);
  motorFL.stop();
  delay(1000);
  Serial.println("front right");
  motorFR.runSpeed(150);
  delay(3000);
  motorFR.stop();
  delay(1000);
  Serial.println("back left");
  motorBL.runSpeed(150);
  delay(3000);
  motorBL.stop();
  delay(1000);
  Serial.println("back right");
  motorBR.runSpeed(150);
  delay(3000);
  motorBR.stop();
#endif

  // минимальный сигнал на мотор
  motorFR.setMinDuty(80);
  motorBR.setMinDuty(80);
  motorFL.setMinDuty(80);
  motorBL.setMinDuty(80);

  // скорость плавности
  motorFR.setAccel(150);
  motorBR.setAccel(150);
  motorFL.setAccel(150);
  motorBL.setAccel(150);
}

void loop() {
  bool dataUpdated = BP32.update();

  if (dataUpdated && gamepad != nullptr) {
    if (gamepad->isConnected() && gamepad->hasData()) {
      Serial.printf(
        "idx=%d, axis L: %4d, %4d, axis R: %4d, %4d\n",
        gamepad->index(),   // gamepad Index
        gamepad->axisX(),   // (-511 - 512) left X Axis
        gamepad->axisY(),   // (-511 - 512) left Y axis
        gamepad->axisRX(),  // (-511 - 512) right X axis
        gamepad->axisRY()   // (-511 - 512) right Y axis
      );

      // переводим диапазон 0..255 в -MAX_SPEED..MAX_SPEED
      int valLX = map(gamepad->axisX(), -511, 512, -MAX_SPEED, MAX_SPEED);
      int valLY = map(gamepad->axisY(), 512, -511, -MAX_SPEED, MAX_SPEED);
      int valRX = map(gamepad->axisRX(), -511, 512, -MAX_SPEED, MAX_SPEED);
      int valRY = map(gamepad->axisRY(), 512, -511, -MAX_SPEED, MAX_SPEED);

      Serial.printf(
        "LX: %4d, LY: %4d, RX: %4d, RY: %4d\n",
        valLX,  // (-511 - 512) left X Axis
        valLY,  // (-511 - 512) left Y axis
        valRX,  // (-511 - 512) right X axis
        valRY   // (-511 - 512) right Y axis
      );

      int dutyFR = valLY + valLX;
      int dutyFL = valLY - valLX;
      int dutyBR = valLY - valLX;
      int dutyBL = valLY + valLX;

      dutyFR += valRY - valRX;
      dutyFL += valRY + valRX;
      dutyBR += valRY - valRX;
      dutyBL += valRY + valRX;

      Serial.printf(
        "FR: %4d, FL: %4d, BR: %4d, BL: %4d\n\n",
        dutyFR,  // (-511 - 512) left X Axis
        dutyFL,  // (-511 - 512) left Y axis
        dutyBR,  // (-511 - 512) right X axis
        dutyBL   // (-511 - 512) right Y axis
      );

      // ПЛАВНЫЙ контроль скорости, защита от рывков
      motorFR.runSpeed(dutyFR);
      motorBR.runSpeed(dutyBR);
      motorFL.runSpeed(dutyFL);
      motorBL.runSpeed(dutyBL);
    }
  } else {
    // страшно, вырубай
    motorFR.stop();
    motorBR.stop();
    motorFL.stop();
    motorBL.stop();
  }
  motorFR.tick();
  motorBR.tick();
  motorFL.tick();
  motorBL.tick();

  delay(20);
}

void onConnectedController(ControllerPtr ctl) {
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
      // Additionally, you can get certain gamepad properties like:
      // Model, VID, PID, BTAddr, flags, etc.
      ControllerProperties properties = ctl->getProperties();
      Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                    properties.product_id);
      myControllers[i] = ctl;
      if (ctl->isGamepad()) gamepad = ctl;
      foundEmptySlot = true;
      break;
    }
  }
  if (!foundEmptySlot) {
    Serial.println("CALLBACK: Controller connected, but could not found empty slot");
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  bool foundController = false;

  if (gamepad == ctl) gamepad = nullptr;

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
      myControllers[i] = nullptr;
      foundController = true;
      break;
    }
  }

  if (!foundController) {
    Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
  }
}
