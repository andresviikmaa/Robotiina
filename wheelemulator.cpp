
#include "wheelemulator.h"
#include <boost/property_tree/ini_parser.hpp>
#include <boost/thread/thread.hpp>
#include <sstream>
#include <stdio.h>
#include <algorithm>    // std::min

WheelEmulator::WheelEmulator(const std::string &name, boost::asio::io_service &io_service, std::string port, unsigned int baud_rate)
	: SimpleSerial(io_service, port, baud_rate)
{
	stop = false;
    this->name = name;
}
char  WheelEmulator::usb_serial_getchar()
{
	return '\0';
}
uint8_t WheelEmulator::recv_str(char *buf, uint8_t size) 
{
	std::string str = readLine();
	std::cout << name << " IN: " << str << std::endl;
	strncpy(buf, str.c_str(), size);
	return std::min(str.size(), (size_t)size);
	/*
	char data;
	uint8_t count = 0;

	while (count < size) {
		data = usb_serial_getchar();
		//usb_serial_putchar(data);
		if (data == '\r' || data == '\n') {
			*buf = '\0';
			return size;
		}
		if (data >= ' ' && data <= '~') {
			*buf++ = data;
			count++;
		}
	}
	return count;
	*/
}

void WheelEmulator::Run() {
    std::cout << name << " STARTED" << std::endl;

    std::stringstream filename;
    filename << "conf/wheel_eeprom_" << name << ".ini";

    try {
        read_ini(filename.str(), eeprom);
    }catch(...){}

    setup();
    write_ini(filename.str(), eeprom);

//	CLKPR = 0x80;
//	CLKPR = 0x00;
//	usb_init();

	dir = eeprom_read_byte((uint8_t*)0);
	motor_polarity = eeprom_read_byte((uint8_t*)1);
	pgain = eeprom_read_byte((uint8_t*)3);
	igain = eeprom_read_byte((uint8_t*)4);
	dgain = eeprom_read_byte((uint8_t*)5);

	if (pgain == 255) {
		pgain = 6;
		eeprom_update_byte((uint8_t*)3, pgain);
	}
	if (igain == 255) {
		igain = 8;
		eeprom_update_byte((uint8_t*)4, igain);
	}
	/*
	bit_set(DDRC, BIT(LED1));
	bit_set(DDRC, BIT(LED2));
	bit_set(PORTC, BIT(LED1));
	bit_set(PORTC, BIT(LED2));

	//Wait for USB to be configured
	while (!usb_configured());
	_delay_ms(500);

	//PID
	pid_multi = 32;
	imax = 255 * pid_multi;
	err_max = 4000;
	pwm_min = 25;
	//igain = 8;	//divider
	//pgain = 2;
	//dgain = 0;
	intgrl = 0;
	count = 0;
	speed = 0;
	err = 0;

	//timer0
	TCCR0A = 0b00000010;
	TCCR0B = 0b00000101; //prescale 1024
	OCR0A = 250;
	TIMSK0 = 0b00000010;
	TCNT0 = 0;

	//init PCINT0
	DDRB &= 0b11111110;
	PCICR = 1; //enable pin change interrupt
	PCMSK0 = 0b00000001;

	//quadrature decoder, INT0, INT1 interrupt
	bit_clear(DDRD, BIT(0));
	bit_clear(DDRD, BIT(1));
	EICRA = 0b00000101; //both edges
	EIMSK = 0b00000011; //enable mask

	//Mootorite PWM
	bit_set(DDRC, BIT(PWM));

	//TCCR1A = 0b10000001; //phase correct
	//TCCR1B = 0b00000011; //prescale 64, freq 490
	TCCR1A = 0b10000001; //fast pwm
	TCCR1B = 0b00001100; //prescale 64, freq 980		

	OCR1AL = 0;
	//OCR1BL = 0;

	bit_set(DDRB, BIT(DIR1));
	bit_set(DDRB, BIT(DIR2));

	sei();
	*/
	uint8_t n;
	char buf[16];

	while (1) {
		if (update_pid) {
			if (pid_on) {
				pid();
				update_pid = 0;
			}
			if (send_speed) {
				sprintf(response, "<s:%d>\n", speed);
				usb_write(response);
			}
			if ((speed < 10) && (pwm > 250)) {
				stall_counter++;
			}
			else {
				stall_counter = 0;
			}
			fail_counter++;
		}

		if ((fail_counter == 100) && failsafe) {
			sp_pid = 0;
			reset_pid();
			forward(0);
		}
		if (stallChanged) {
			sprintf(response, "<stall:%d>\n", stallLevel);
			usb_write(response);
			stallChanged = 0;
		}
		if (true) {
			n = recv_str(buf, sizeof(buf));
			if (n == sizeof(buf)) {
				parse_and_execute_command(buf);
			}
		}
	}

	/*
	while (!stop) {
        std::string str = readLine();
        std::cout << name << " IN: " <<  str << std::endl;
        parse_and_execute_command(str.c_str());
	}
	*/
    write_ini(filename.str(), eeprom);
    std::cout << name << " STOPED" << std::endl;

}
/**/
void WheelEmulator::usb_write(const char *str) {
    std::cout << name << " OUT: " <<  str << std::endl;
    writeString(str);
}
void WheelEmulator::eeprom_update_byte(uint8_t * __p, uint8_t _value){
    std::stringstream key; key << "byte" << *__p;
    eeprom.put(key.str(), _value);
}
uint8_t WheelEmulator::eeprom_read_byte(const uint8_t * __p) {
    std::stringstream key; key << "byte" << *__p;
    return  eeprom.get<uint8_t>(key.str());
}

void WheelEmulator::pid() {
    err_prev = err;
    err = sp_pid - speed;

    if (stallLevel != 2) {

        intgrl += (err * pid_multi) / igain;

        //constrain integral
        if (intgrl < -imax) intgrl = -imax;
        if (intgrl > imax) intgrl = imax;

        if (sp == 0) pwmmin = 0;
        else if (sp < 0) pwmmin = -pwm_min;
        else pwmmin = pwm_min;

        pwm = pwmmin + err*pgain + intgrl/pid_multi;
        //constrain pwm
        if (pwm < -255) pwm = -255;
        if (pwm > 255) pwm = 255;

        prevStallCount = stallCount;
        if ((speed < 5 && currentPWM == 255 || speed > -5 && currentPWM == -255) && stallCount < stallErrorLimit) {
            stallCount++;
        } else if (stallCount > 0) {
            stallCount--;
        }

        if (pwm < 0) {
            pwm *= -1;
            backward(pwm);
        } else {
            forward(pwm);
        }

        if ((stallCount == stallWarningLimit - 1) && (prevStallCount == stallWarningLimit)) {
            stallLevel = 0;
            stallChanged = 1;
        } else if ((stallCount == stallWarningLimit) && (prevStallCount == stallWarningLimit - 1)) {
            stallLevel = 1;
            stallChanged = 1;
        } else if (stallCount == stallErrorLimit) {
            stallLevel = 2;
            stallChanged = 1;
            reset_pid();
        }
    } else {
        stallCount--;
        if (stallCount == 0) {
            stallLevel = 0;
            stallChanged = 1;
        }
    }
    //OCR1AL = (pwm >= 0) ? pwm : -pwm;
}

void WheelEmulator::reset_pid() {
    err = 0;
    err_prev = 0;
    intgrl = 0;
    der = 0;
    sp = 0;
    sp_pid = 0;
    forward(0);
}

void  WheelEmulator::forward(uint8_t pwm) {
    if (dir) {
        bit_clear(PORTB, BIT(DIR1));
        bit_set(PORTB, BIT(DIR2));
    } else {
        bit_set(PORTB, BIT(DIR1));
        bit_clear(PORTB, BIT(DIR2));
    }
    OCR1AL = pwm;
    currentPWM = pwm;
}

void  WheelEmulator::backward(uint8_t pwm) {
    if (dir) {
        bit_set(PORTB, BIT(DIR1));
        bit_clear(PORTB, BIT(DIR2));
    } else {
        bit_clear(PORTB, BIT(DIR1));
        bit_set(PORTB, BIT(DIR2));
    }
    OCR1AL = pwm;
    currentPWM = -pwm;
}

void WheelEmulator::parse_and_execute_command(const char *buf) {
    const char *command;
    int16_t par1;
    command = buf;
    if ((command[0] == 's') && (command[1] == 'd')) {
        //set motor speed with pid setpoint
        pid_on = 1;
        par1 = atoi(command+2);
        sp_pid = par1;
        if (sp_pid == 0) reset_pid();
        fail_counter = 0;
    } else if ((command[0] == 'w') && (command[1] == 'l')) {
        //set motor speed with pwm
        pid_on = 0;
        par1 = atoi(command+2);
        if (par1 < 0) {
            par1 *= -1;
            backward(par1);
        } else {
            forward(par1);

        }
        fail_counter = 0;
    } else if ((command[0] == 'g') && (command[1] == 'b')) {
        //get ball
        sprintf(response, "<b:%d>\n", ball);
        usb_write(response);
    } else if ((command[0] == 'd') && (command[1] == 'r')) {
        //toggle motor direction
        par1 = atoi(command+2);
        if (dir ^ par1) motor_polarity ^= 1;
        dir = par1;
        eeprom_update_byte((uint8_t*)0, dir);
        eeprom_update_byte((uint8_t*)1, motor_polarity);
    } else if ((command[0] == 's') && (command[1] == 't')) {
        //perform setup
        setup();
    } else if ((command[0] == 'm') && (command[1] == 'p')) {
        //toggle motor polarity
        par1 = atoi(command+2);
        motor_polarity = par1;
        eeprom_update_byte((uint8_t*)1, motor_polarity);
    } else if ((command[0] == 'p') && (command[1] == 'd')) {
        //toggle pid
        par1 = atoi(command+2);
        pid_on = par1;
        if (!pid_on) reset_pid();
    } else if ((command[0] == 'g') && (command[1] == 's')) {
        //toggle get speed on every pid cycle
        par1 = atoi(command+2);
        send_speed = par1;
    } else if ((command[0] == 'f') && (command[1] == 's')) {
        //toggle failsafe
        par1 = atoi(command+2);
        failsafe = par1;
    } else if (command[0] == 's') {
        //get speed
        sprintf(response, "<s:%d>\n", speed);
        usb_write(response);
    } else if ((command[0] == 'i') && (command[1] == 'd')) {
        //set id
        par1 = atoi(command+2);
        eeprom_update_byte((uint8_t*)2, par1);
    } else if (command[0] == '?') {
        //get info: id
        par1 = eeprom_read_byte((uint8_t*)2);
        sprintf(response, "<id:%d>\n", par1);
        usb_write(response);
    } else if ((command[0] == 'p') && (command[1] == 'g')) {
        //set pgain
        par1 = atoi(command+2);
        eeprom_update_byte((uint8_t*)3, par1);
        pgain = par1;
    } else if ((command[0] == 'i') && (command[1] == 'g')) {
        //set igain
        par1 = atoi(command+2);
        eeprom_update_byte((uint8_t*)4, par1);
        igain = par1;
    } else if ((command[0] == 'd') && (command[1] == 'g')) {
        //set dgain
        par1 = atoi(command+2);
        eeprom_update_byte((uint8_t*)5, par1);
        dgain = par1;
    } else if ((command[0] == 'g') && (command[1] == 'g')) {
        //get pid gains
        sprintf(response, "<pid:%d,%d>\n", pgain, igain);
        usb_write(response);
    } else if ((command[0] == 't') && (command[1] == 'l')) {
        //toggle leds
        par1 = atoi(command+2);
        leds_on = par1;
        if (leds_on == 0) {
            bit_clear(PORTC, BIT(LED1));
            bit_clear(PORTC, BIT(LED2));
        }
    } else {
        //bit_flip(PORTC, BIT(LED1));
        sprintf(response, "%s\n", command);
        usb_write(response);
    }
}

void WheelEmulator::setup() {
    //uint8_t pid_on_current = pid_on;
    wcount.value = 0;
    sp_pid = 0;
    pid_on = 1;
}

void WheelEmulator::Stop() {
    std::stringstream filename;
    filename << "conf/wheel_eeprom_" << name << ".ini";
    write_ini(filename.str(), eeprom);
    serial.cancel();
	stop = true;
}

WheelEmulator::~WheelEmulator()
{
}


int main(int argc, char *argv[])
{
    if (argc < 4){
        std::cout << "usage ./wheelemulator <port1> <port2> <port3>" << std::endl;
        exit(-1);
    }
	boost::thread_group threads;
    boost::asio::io_service io;

    WheelEmulator * we_left;
	WheelEmulator * we_right;
	WheelEmulator * we_back;


    try
    {

		we_left = new WheelEmulator("left", io, argv[1], 115200);
		we_right = new WheelEmulator("right", io, argv[2], 115200);
		we_back = new WheelEmulator("back", io, argv[3], 115200);

		threads.create_thread(boost::bind(&WheelEmulator::Run, we_left));
		threads.create_thread(boost::bind(&WheelEmulator::Run, we_right));
		threads.create_thread(boost::bind(&WheelEmulator::Run, we_back));

        unsigned char a;
        while(getchar()){
            break;
        }
		we_left->Stop();
		we_right->Stop();
		we_back->Stop();

		threads.join_all();

		delete we_left;
		delete we_right;
		delete we_back;



    }
    catch (std::exception &e)
    {
        std::cout << "ups, " << e.what() << std::endl;
    }
    catch (const std::string &e)
    {
        std::cout << "ups, " << e << std::endl;
    }
    catch (...)
    {
        std::cout << "ups, did not see that coming."<< std::endl;
    }

}
