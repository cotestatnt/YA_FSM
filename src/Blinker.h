
#ifndef BLINKER_H
#define BLINKER_H

////////////////////// Small class for blink led handling  //////////////////////////////////////
class Blinker
{
  private:
    uint32_t blinkTime;
    uint8_t  ledPin;
    uint32_t halfPeriod;
    int onLevel = HIGH;

  public:
    Blinker(uint8_t pin, uint32_t time) : ledPin(pin), halfPeriod(time) {
      pinMode(ledPin, OUTPUT);
    }

    bool setTime( uint32_t time) {
      halfPeriod = time;
    }

    bool blink(bool active) {
      if (active) {
        if (millis() - blinkTime > halfPeriod ) {
          blinkTime = millis();
          digitalWrite(ledPin, !digitalRead(ledPin));
        }
      }
      else
        digitalWrite(ledPin, !onLevel);
      return active;
    }
};
/////////////////////////////////////////////////////////////////////////////////////////////////

#endif
