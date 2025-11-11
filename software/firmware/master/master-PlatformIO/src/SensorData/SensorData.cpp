#include "SensorData.h"
#include <ArduinoJson.h>

SensorDataManager::SensorDataManager() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensors_[i] = SensorReading();
  }
}

void SensorDataManager::updateSensor(int sensor_id, float distance_mm, bool stable, bool ch1_active, bool ch2_active) {
  if (sensor_id >= 0 && sensor_id < NUM_SENSORS) {
    sensors_[sensor_id].distance_mm = distance_mm;
    sensors_[sensor_id].stable = stable;
    sensors_[sensor_id].ch1_active = ch1_active;
    sensors_[sensor_id].ch2_active = ch2_active;
    sensors_[sensor_id].last_update = millis();
    sensors_[sensor_id].fresh = true;
  }
}

SensorReading SensorDataManager::getSensorData(int sensor_id) const {
  if (sensor_id >= 0 && sensor_id < NUM_SENSORS) {
    return sensors_[sensor_id];
  }
  return SensorReading();
}

void SensorDataManager::markAllStale() {
  unsigned long current_time = millis();
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensors_[i].fresh = sensors_[i].isFresh(current_time);
  }
}

String SensorDataManager::toJSON() const {
  DynamicJsonDocument doc(2048);
  doc["timestamp"] = millis();
  
  JsonArray sensors = doc.createNestedArray("sensors");
  for (int i = 0; i < NUM_SENSORS; i++) {
    JsonObject sensor = sensors.createNestedObject();
    sensor["id"] = i;
    sensor["distance_mm"] = sensors_[i].distance_mm;
    sensor["stable"] = sensors_[i].stable;
    sensor["ch1_active"] = sensors_[i].ch1_active;
    sensor["ch2_active"] = sensors_[i].ch2_active;
    sensor["fresh"] = sensors_[i].isFresh(millis());
  }
  
  String output;
  serializeJson(doc, output);
  return output;
}