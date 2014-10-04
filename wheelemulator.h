#pragma  once
#include "types.h"
#include "simpleserial.h"
#include <boost/atomic.hpp>
#include <boost/property_tree/ptree.hpp>


/*
	You can use Virtual Serial Port Driver from Eltima Software to bind two COM ports together
	and use this class instead of physical device

	Emulation code is from here:
	https://bitbucket.org/ReikoR/w2012/src
*/
union doublebyte {
    unsigned int value;
    unsigned char bytes[2];
};
#define bit_get(p,m) ((p) & (m))
#define bit_set(p,m) ((p) |= (m))
#define bit_clear(p,m) ((p) &= ~(m))
#define bit_flip(p,m) ((p) ^= (m))
#define bit_write(c,p,m) (c ? bit_set(p,m) : bit_clear(p,m))
#define BIT(x) (0x01 << (x))
#define LONGBIT(x) ((unsigned long)0x00000001 << (x))

//pin definitions
#define PWM PC6
#define DIR1 /*PB6*/0
#define DIR2 /*PB7*/1
#define FAULT PC7
#define LED1 /*PC4*/0
#define LED2 /*PC5*/1
#define ENCA PD0
#define ENCB PD1

class WheelEmulator : public SimpleSerial
{
public:
	boost::atomic<bool> stop;

	WheelEmulator(const std::string &name, boost::asio::io_service &io_service, std::string port = "port", unsigned int baud_rate = 115200);
	void Run();
	void Stop();
	~WheelEmulator();
private:
    boost::property_tree::ptree eeprom;
    std::string name;
    uint8_t enc_dir;
    uint8_t enc_last = 0;
    uint8_t enc_now;
    union doublebyte wcount;
    union doublebyte decoder_count;

    uint8_t dir;
    uint8_t pid_on = 1;
    uint8_t motor_polarity = 1;
    uint8_t fail_counter = 0;
    uint8_t send_speed = 0;
    uint8_t failsafe = 1;
    uint8_t leds_on = 1;

    int16_t sp, sp_pid, der;
    int16_t intgrl, prop;
    int16_t count, speed;
    int16_t csum;
    int16_t err, err_prev;
    uint8_t pgain, igain, dgain;
    int16_t pwm, pwm_min, pwmmin;
    int16_t imax, err_max;
    uint8_t pid_multi, update_pid;
    uint8_t ball = 0;
    uint8_t ir_led;
    uint16_t stall_counter = 0;
    uint16_t stallCount = 0;
    uint16_t prevStallCount = 0;
    uint16_t stallWarningLimit = 60;
    uint16_t stallErrorLimit = 300;
    uint8_t stallLevel = 0;
    uint8_t stallChanged = 0;
    int16_t currentPWM = 0;
    char response[16];
    int8_t OCR1AL;
    int8_t PORTB;
    int8_t PORTC;

    void usb_write(const char *str);
    void parse_and_execute_command(const char *buf);
    void eeprom_update_byte(uint8_t * __p, uint8_t __value);
    uint8_t eeprom_read_byte(const uint8_t * __p);
    void pid();
    void reset_pid() ;
    void forward(uint8_t pwm);
    void backward(uint8_t pwm);
    void setup();
};

