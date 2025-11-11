#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <Arduino.h>

struct SensorReading {
  float distance_mm = 0.0;
  bool stable = false;
  bool ch1_active = false;
  bool ch2_active = false;
  unsigned long last_update = 0;
  bool fresh = false;
  
  bool isFresh(unsigned long current_time, unsigned long timeout_ms = 10000) const {
    return (current_time - last_update) < timeout_ms;
  }
};

class SensorDataManager {
public:
  static const int NUM_SENSORS = 4;
  
  SensorDataManager();
  
  void updateSensor(int sensor_id, float distance_mm, bool stable, bool ch1_active, bool ch2_active);
  SensorReading getSensorData(int sensor_id) const;
  void markAllStale();
  String toJSON() const;
  
private:
  SensorReading sensors_[NUM_SENSORS];
};

#endif