#ifndef _PID_HPP_
#define _PID_HPP_
#include <iostream>
#include <chrono>

class PIDController
{
public:
    PIDController(double kp, double ki, double kd)
        : kp_(kp), ki_(ki), kd_(kd),integral_(0),pre_error_(0) {};
    ~PIDController() {};
    double update(double setpoint, double actual_position)
    {
        double error = setpoint - actual_position;
        integral_ += error;
        double derivative = error - pre_error_;
        double output = kp_ * error + ki_ * integral_ + kd_ * derivative;

        pre_error_ = error;
        return output;
    }

private:
    double kp_;
    double ki_;
    double kd_;
    double pre_error_;
    double integral_;
};

#endif // _PID_HPP_