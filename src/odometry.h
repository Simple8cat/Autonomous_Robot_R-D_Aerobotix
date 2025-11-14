#pragma once
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
const double wheel_base = 176.9;   
const double wheel_radius = 33.3; 
const double tick_per_revolution = 100; // Encoder ticks per wheel revolution
int delta_left = 0;               // Change in left wheel encoder counts
int delta_right = 0;              // Change in right wheel encoder counts

typedef struct
{
    double x;
    double y;
    double theta;
} robot_pose_t;

robot_pose_t current_pose = {0.0, 0.0, 0.0};

void update_pose(void *parameter)
{
    while (1)
    {
        double dl = delta_left/tick_per_revolution*2*M_PI*wheel_radius;
        double dr = delta_right/tick_per_revolution*2*M_PI*wheel_radius;
        double delta_theta = (dr - dl) / wheel_base;
        current_pose.x += (dr+dl) / 2 * cos(current_pose.theta + delta_theta / 2);
        current_pose.y += (dr+dl) / 2 * sin(current_pose.theta + delta_theta / 2);
        current_pose.theta += delta_theta;
        delta_left = 0;
        delta_right = 0;
        vTaskDelay(100 / portTICK_PERIOD_MS); // Update every 100 ms
    }
} 