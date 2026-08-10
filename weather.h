#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>

// =========================================================
// C. AI WEATHER PREDICTOR (TRAINED ON JOHOR DATA 2020-2024)
// =========================================================
// Loaded 532098 rows of Johor weather data.
// Model Trained on Johor Data!
// (Inputs: Temperature, Humidity -> Output: Weather Forecast)

String predictWeatherAI(float t, float h) {
  if (h <= 85.05) {
    if (h <= 64.45) {
      if (h <= 59.75) {
        if (h <= 53.15) {
          if (t <= 32.65) {
            return "Sunny";
          } else { // t > 32.65
            return "Sunny";
          }
        } else { // h > 53.15
          if (t <= 34.05) {
            return "Sunny";
          } else { // t > 34.05
            return "Sunny";
          }
        }
      } else { // h > 59.75
        if (t <= 31.85) {
          if (t <= 26.25) {
            return "Rain";
          } else { // t > 26.25
            return "Sunny";
          }
        } else { // t > 31.85
          if (h <= 62.05) {
            return "Sunny";
          } else { // h > 62.05
            return "Sunny";
          }
        }
      }
    } else { // h > 64.45
      if (h <= 79.05) {
        if (h <= 69.25) {
          if (t <= 31.55) {
            return "Sunny";
          } else { // t > 31.55
            return "Sunny";
          }
        } else { // h > 69.25
          if (t <= 34.95) {
            return "Sunny";
          } else { // t > 34.95
            return "Sunny";
          }
        }
      } else { // h > 79.05
        if (t <= 26.55) {
          if (t <= 24.75) {
            return "Sunny";
          } else { // t > 24.75
            return "Sunny";
          }
        } else { // t > 26.55
          if (t <= 29.55) {
            return "Sunny";
          } else { // t > 29.55
            return "Sunny";
          }
        }
      }
    }
  } else { // h > 85.05
    if (t <= 25.95) {
      if (h <= 91.05) {
        if (t <= 25.55) {
          if (h <= 89.05) {
            return "Cloudy";
          } else { // h > 89.05
            return "Cloudy";
          }
        } else { // t > 25.55
          if (h <= 89.05) {
            return "Cloudy";
          } else { // h > 89.05
            return "Cloudy";
          }
        }
      } else { // h > 91.05
        if (t <= 20.35) {
          if (t <= 9.25) {
            return "Cloudy";
          } else { // t > 9.25
            return "Cloudy";
          }
        } else { // t > 20.35
          if (t <= 23.45) {
            return "Cloudy";
          } else { // t > 23.45
            return "Cloudy";
          }
        }
      }
    } else { // t > 25.95
      if (t <= 38.35) {
        if (t <= 30.85) {
          if (t <= 30.55) {
            return "Cloudy";
          } else { // t > 30.55
            return "Rain";
          }
        } else { // t > 30.85
          if (t <= 31.25) {
            return "Cloudy";
          } else { // t > 31.25
            return "Cloudy";
          }
        }
      } else { // t > 38.35
        if (h <= 98.50) {
          return "Cloudy";
        } else { // h > 98.50
          if (t <= 39.95) {
            return "Rain";
          } else { // t > 39.95
            return "Rain";
          }
        }
      }
    }
  }
}

#endif