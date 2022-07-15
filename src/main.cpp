#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <ardukit.h>

using namespace adk;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void on_ready(EventEmitter::event &ev);
void on_starting(EventEmitter::event &ev);
void on_passing(EventEmitter::event &ev);

tick_t tm_start = 0;

class PhotoDetector : public EdgeDetector {
protected:
    int m_pin_id = -1;
    int m_offset = 0; // distance(cm) from start position(sensor1)

public:
    PhotoDetector(int pin_id, int offset, bool starter = false)
        : EdgeDetector(0, 1), m_pin_id(pin_id), m_offset(offset)
    {
        set_scan_interval(0);
        start_input_scan();
        if (starter) {
            on("rising", on_ready, this);
            on("falling", on_starting, this);
        }
        else {
            on("rising", on_passing, this);
        }
    }

    int pin_id()
    {
        return m_pin_id;
    }
    int offset()
    {
        return m_offset;
    }

    virtual float read_input()
    {
        return (float)(digitalRead(m_pin_id)); // convert int to float
    }
};

PhotoDetector sensor1(2, 0, true);
PhotoDetector sensor2(3, 10);
PhotoDetector sensor3(4, 30);
PhotoDetector sensor4(5, 60);

void on_ready(EventEmitter::event &ev)
{
    PhotoDetector *inst = (PhotoDetector *)ev.data;
    dmsg("Ready! Pin ID=%d\n", inst->pin_id());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ready! PinID = ");
    lcd.print(inst->pin_id());
}

void on_starting(EventEmitter::event &ev)
{
    PhotoDetector *inst = (PhotoDetector *)ev.data;
    dmsg("Start falling. PinID=%d\n", inst->pin_id());
    tm_start = ticks();
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("cm/s^2");
    lcd.setCursor(0, 0);
}

void on_passing(EventEmitter::event &ev)
{
    PhotoDetector *inst = (PhotoDetector *)ev.data;
    msec_t tm_elapsed = elapsed_msec(tm_start);
    float g = (2 * inst->offset() / 100.0) / ((tm_elapsed / 1000.0) * (tm_elapsed / 1000.0));
    int g_int = round(g * 100.0);
    dmsg("Passing PinID=%d elapsed=%ld g=%d\n", inst->pin_id(), tm_elapsed, g_int);
    lcd.print(g_int);
    lcd.print("/");
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("gravitational");
    lcd.setCursor(0, 1);
    lcd.print("acceleration");
}

void loop()
{
    adk::run();
}