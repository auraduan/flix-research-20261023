// Copyright (c) 2023 Oleg Kalachev <okalachev@gmail.com>
// Repository: https://github.com/okalachev/flix
// 使用MOSFET的电机输出控制
// Motors output control using MOSFETs

#include "util.h"

// 右前是 CN5，mfr 输出 motor 2
// 无人机供电口是朝前，USB-C 朝后

#define MOTOR_0_PIN 13 // RL左后rear left
#define MOTOR_1_PIN 12 // RR右后rear right
#define MOTOR_2_PIN 10 // FR右前front right
#define MOTOR_3_PIN 9  // FL左前front left

/*
#define MOTOR_0_PIN 10 // RL左后 = S1 → Q4 → CN5
#define MOTOR_1_PIN 12 // RR右后 = S2 → Q1 → CN2
#define MOTOR_2_PIN 9  // FR右前 = S4 → Q3 → CN4
#define MOTOR_3_PIN 13 // FL左前 = S3 → Q2 → CN3
*/

// 78kHz/10bit 在部分 ESP32-S3 + Arduino core 上 ledcAttach 会失败
// 20kHz 足够驱动 MOSFET 有刷电机，分频裕量更大
#define PWM_FREQUENCY 20000
#define PWM_RESOLUTION 10
#define PWM_STOP 0
#define PWM_MIN 0
#define PWM_MAX 1000000 / PWM_FREQUENCY

// 0 = PWM(LEDC), 1 = 仅数字电平(调试/LEDC失败时)
static int motorOutputMode = 0;

// Motors array indexes:
const int MOTOR_REAR_LEFT = 0;
const int MOTOR_REAR_RIGHT = 1;
const int MOTOR_FRONT_RIGHT = 2;
const int MOTOR_FRONT_LEFT = 3;

static bool tryAttach(int pin, uint32_t freq, uint8_t res) {
	if (ledcAttach(pin, freq, res)) return true;
	// 部分 core 需要先挂再改频率
	if (ledcAttach(pin, freq, res)) return true;
	return false;
}

void setupMotors() {
	print("Setup Motors\n");
	print("Chip: %s\n", ESP.getChipModel());

	int pins[4] = {MOTOR_0_PIN, MOTOR_1_PIN, MOTOR_2_PIN, MOTOR_3_PIN};
	int okCount = 0;

	for (int i = 0; i < 4; i++) {
		bool ok = tryAttach(pins[i], PWM_FREQUENCY, PWM_RESOLUTION);
		if (!ok) {
			// 再试 5kHz/8bit
			ok = tryAttach(pins[i], 5000, 8);
		}
		print("Motor%d pin%d: %d\n", i, pins[i], ok);
		if (ok) okCount++;
	}

	if (okCount < 4) {
		motorOutputMode = 1;
		print("LEDC attach failed, fallback to digitalWrite\n");
		for (int i = 0; i < 4; i++) {
			pinMode(pins[i], OUTPUT);
			digitalWrite(pins[i], LOW);
		}
	} else {
		motorOutputMode = 0;
		print("LEDC mode @ %d Hz\n", PWM_FREQUENCY);
	}

	sendMotors();
	print("Motors initialized mode=%d\n", motorOutputMode);
}

int getDutyCycle(float value) {
	value = constrain(value, 0, 1);
	if (motorOutputMode == 1) {
		return value > 0 ? 1 : 0; // 数字模式仅开关
	}
	float pwm = mapf(value, 0, 1, PWM_MIN, PWM_MAX);
	if (value == 0) pwm = PWM_STOP;
	float duty = mapf(pwm, 0, 1000000 / (float)PWM_FREQUENCY, 0, (1 << PWM_RESOLUTION) - 1);
	return round(duty);
}

void sendMotors() {
	if (motorOutputMode == 1) {
		digitalWrite(MOTOR_0_PIN, motors[0] > 0 ? HIGH : LOW);
		digitalWrite(MOTOR_1_PIN, motors[1] > 0 ? HIGH : LOW);
		digitalWrite(MOTOR_2_PIN, motors[2] > 0 ? HIGH : LOW);
		digitalWrite(MOTOR_3_PIN, motors[3] > 0 ? HIGH : LOW);
		return;
	}
	ledcWrite(MOTOR_0_PIN, getDutyCycle(motors[0]));
	ledcWrite(MOTOR_1_PIN, getDutyCycle(motors[1]));
	ledcWrite(MOTOR_2_PIN, getDutyCycle(motors[2]));
	ledcWrite(MOTOR_3_PIN, getDutyCycle(motors[3]));
}

bool motorsActive() {
	return motors[0] != 0 || motors[1] != 0 || motors[2] != 0 || motors[3] != 0;
}

void testMotor(int n) {
	print("Testing motor %d duty=%d mode=%d\n", n, getDutyCycle(1.0f), motorOutputMode);
	motors[n] = 1;
	delay(50);
	sendMotors();
	pause(3);
	motors[n] = 0;
	sendMotors();
	print("Done\n");
}
