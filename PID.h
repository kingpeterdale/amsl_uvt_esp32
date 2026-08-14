#include <cmath>

class PID {
public:
  PID(float Kp, float Ki, float Kd) {
    kp = Kp;
    ki = Ki;
    kd = Kd;
  }
  PID(float Kp, float Ki, float Kd, float Limit) {
    kp = Kp;
    ki = Ki;
    kd = Kd;
    limit = Limit;
  }

  void updateGains(float Kp, float Ki, float Kd) {
    kp = Kp;
    ki = Ki;
    kd = Kd;
  }

  void genStatus(char* buf, size_t len) {
    snprintf(buf, len, "KP:%04.1f  KI:%04.1f  KD:%04.1f", kp, ki, kd);
  }

  float run(float sp, float in, float dt_ms) {
    float err = sp - in;
    float out =  kp * err;

    return fmax(-limit, fmin(limit, out));
  }


private:
  float kp = 0.0;
  float ki = 0.0;
  float kd = 0.0;
  float limit = 100;
};