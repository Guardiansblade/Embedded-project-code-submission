#include "mbed.h"

//Ultra sonic sensor (HC-SR04 most electronics)
InterruptIn echo(D2);
DigitalOut trig(D3);
Timer u_time;
#define max_distance 40.0f		//max distance at which the car	will follow the object
#define min_distance 20.0f		//								will stop moving
#define close_distance 10.0f	//								will back up

volatile float dist = 0;		//distance but renamed, because the compile refuses the name "distance", for some reason

//Motor A (right)
PwmOut ena(D10);
DigitalOut in1(D9);
DigitalOut in2(D8);

//Motor B (left)
PwmOut enb(D6);
DigitalOut in3(D5);
DigitalOut in4(D4);



void u_trig(){	//the function for the sensor's trigger pin
    trig=0;     //safe measure
    wait_us(2); // 2 micro second before trig is pinged, to ensure the sesnor can actually see low state/falling edge to start sending the ultra-sonic pulses
    trig=1;
    wait_us(15);//trig needs to be held high for 10 micro seconds to then send ultra sonic pulses after is turned low,
				//usually 10 micro seconds is enough, but some slack won't hurt, to ensure the sensor registerd the signal to send the pulses

    trig = 0;   // turn trig low to send the ultra-sonic pulses
}



void u_echo_rise(){	//when the mcu receives a signal (rise) on the echo pin, it starts a timer,
					//so the distance can be calculated based on the time the U-sonic pulses take to reach back the sensor, causing a falling edge on echo pin.

	u_time.reset();	//resets timer
    u_time.start();	//starts timer
}

void u_echo_fall(){	//stops the timer when the echo pin falls
    u_time.stop();  //pauses timer
    dist = u_time.elapsed_time().count()*0.0343f/2.0f; //calculates the distance based on the time the sound wave takes to go foward and back
}

void motor_go(){
    //motor A (right)
    in1=0;
    in2=1;
    ena.write(0.35f);   //increased the duty cycle on the right motor, because it's slower than the left one

    //motor B (left)
    in3=0;
    in4=1;
    enb.write(0.3f);
}

void motor_back(){		//same as motor_go() but in the reverse direction
    //motor A (right)
    in1=1;
    in2=0;
    ena.write(0.35f);	//increased the duty cycle on the right motor, because it's slower than the left one

    //motor B (left)
    in3=1;
    in4=0;
    enb.write(0.3f);
}

void motor_stop(){		//no electrical braking, just disconnects the motor terminals
    //motor A (right)
    in1=0;
    in2=0;
    ena.write(0);

    //motor B (left)
    in3=0;
    in4=0;
    enb.write(0);
}



// main() runs in its own thread in the OS
int main()
{
	//this part is basically the same as the setup function in arduino IDE, but without the pin setup

    ena.period_ms(1);	        //pwm frequency 1khz
    enb.period_ms(1);	        //higher frequency causes the motor driver to be unresponsive

    echo.rise(&u_echo_rise);	//starts the echo rise function when the pin detects a rising edge
    echo.fall(&u_echo_fall);	//same thing but falling edge


	//this part is the main loop
    while (true) {
        u_trig();

        printf("distance is %d \n",(int)dist);				//mbed uses by default a limited implementation of printf which doesn't support floating numbers

        if(dist >= min_distance && dist <= max_distance)	//if the object is within distance, it will go forward
        motor_go();
        else if(dist<=close_distance && dist!=0)			//if the object is too close, it will backup. Also in a side note:
        motor_back();										//in ardiuno IDE when using the pulsein function,if the function times out it would return zero,
        else												//which isn't the case here, but I checked just in case. Also on another side note:
        motor_stop();										//this is also had the side effect of pausing the whole code, because it was waiting for the falling edge
															//to end the function or to timeout, unlike here in which it just interrupts the code.

    ThisThread::sleep_for(100ms);							//so the car moves a bit before it polls the sensor again


    }
}

//Hossam-Aldeen Moahmmed Atef Saleh 2200904, no team
