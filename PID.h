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
    // Error
    float err = sp - in;

    // Proportional Term
    float p_term =  kp * err;

    // Time step between updates
    float dt = dt_ms / 1000.0;

    // Integral Term
    integral += (err * dt);
    float i_term = ki * integral;

    // Derivative Term
    float derivative = (err - prev_err) / dt;
    float d_term = kd * derivative;
    prev_err = err;

    // PID Output
    float out = p_term + i_term + d_term;

    // Restrict to limit
    return fmax(-limit, fmin(limit, out));
  }


private:
  float kp = 0.0;
  float ki = 0.0;
  float kd = 0.0;
  float integral = 0.0;
  float prev_err = 0.0;
  float limit = 100;
};