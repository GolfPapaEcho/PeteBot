/**************************************************************************/
/*!
 @file     main.c

 @section LICENSE

 Software License Agreement (BSD License)

 Copyright (c) 2013, K. Townsend (microBuilder.eu)
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 3. Neither the name of the copyright holders nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**************************************************************************/
#include <stdio.h>
#include "LPC8xx.h"
#include "gpio.h"
#include "mrt.h"
#include "uart.h"

#if defined(__CODE_RED)
#include <cr_section_macros.h>
#include <NXP/crp.h>
__CRP const unsigned int CRP_WORD = CRP_NO_CRP;
#endif

#define EYE_LEDS    (2)
#define HAND_LED    (3)

/* This define should be enabled if you want to      */
/* maintain an SWD/debug connection to the LPC810,   */
/* but it will prevent you from having access to the */
/* LED on the LPC810 Mini Board, which is on the     */
/* SWDIO pin (PIO0_2).                               */
// #define USE_SWD
void configurePins() {
	/* Enable SWM clock */
	//  LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 7);  // this is already done in SystemInit()
	/* Pin Assign 8 bit Configuration */
	/* U0_TXD */
	/* U0_RXD */
	LPC_SWM->PINASSIGN0 = 0xffff0004UL;

	/* Pin Assign 1 bit Configuration */
#if !defined(USE_SWD)
	/* Pin setup generated via Switch Matrix Tool
	 ------------------------------------------------
	 PIO0_5 = RESET
	 PIO0_4 = U0_TXD
	 PIO0_3 = GPIO            - Disables SWDCLK
	 PIO0_2 = GPIO (User LED) - Disables SWDIO
	 PIO0_1 = GPIO
	 PIO0_0 = U0_RXD
	 ------------------------------------------------
	 NOTE: SWD is disabled to free GPIO pins!
	 ------------------------------------------------ */
	LPC_SWM->PINENABLE0 = 0xffffffbfUL;
#else
	/* Pin setup generated via Switch Matrix Tool
	 ------------------------------------------------
	 PIO0_5 = RESET
	 PIO0_4 = U0_TXD
	 PIO0_3 = SWDCLK
	 PIO0_2 = SWDIO
	 PIO0_1 = GPIO
	 PIO0_0 = U0_RXD
	 ------------------------------------------------
	 NOTE: LED on PIO0_2 unavailable due to SWDIO!
	 ------------------------------------------------ */
	LPC_SWM->PINENABLE0 = 0xffffffb3UL;
#endif
}

//Set up Morse Code look up table (My first jagged arrays!)
//First int in each Morse character array is the number of elements in the array starting counting at zero.
const int a[3] = { 2, 1, 3 };
const int b[5] = { 4, 3, 1, 1, 1 };
const int c[5] = { 4, 3, 1, 3, 1 };
const int d[4] = { 3, 3, 1, 1 };
const int e[2] = { 1, 1 };
const int f[5] = { 4, 1, 1, 3, 1 };
const int g[4] = { 3, 3, 3, 1 };
const int h[5] = { 4, 1, 1, 1, 1 };
const int i[3] = { 2, 1, 1 };
const int j[5] = { 4, 1, 3, 3, 3 };
const int k[4] = { 3, 3, 1, 3 };
const int l[5] = { 4, 1, 3, 1, 1 };
const int m[3] = { 2, 3, 3 };
const int n[3] = { 2, 3, 1 };
const int o[4] = { 3, 3, 3, 3 };
const int p[5] = { 4, 1, 3, 3, 1 };
const int q[5] = { 4, 3, 3, 1, 3 };
const int r[4] = { 3, 1, 3, 1 };
const int s[4] = { 3, 1, 1, 1 };
const int t[2] = { 1, 3 };
const int u[4] = { 3, 1, 1, 3 };
const int v[5] = { 4, 1, 1, 1, 3 };
const int w[4] = { 3, 1, 3, 3 };
const int x[5] = { 4, 3, 1, 1, 3 };
const int y[5] = { 4, 3, 1, 3, 3 };
const int z[5] = { 4, 3, 3, 1, 1 };
const int space[2] = { 1, 4 };
const int one[6] = { 5, 1, 3, 3, 3, 3 };
const int two[6] = { 5, 1, 1, 3, 3, 3 };
const int three[6] = { 5, 1, 1, 1, 3, 3 };
const int four[6] = { 5, 1, 1, 1, 1, 3 };
const int five[6] = { 5, 1, 1, 1, 1, 1 };
const int six[6] = { 5, 3, 1, 1, 1, 1 };
const int seven[6] = { 5, 3, 3, 1, 1, 1 };
const int eight[6] = { 5, 3, 3, 3, 1, 1 };
const int nine[6] = { 5, 3, 3, 3, 3, 1 };
const int zero[6] = { 5, 3, 3, 3, 3, 3 };

//messages (petebot for EYE_LEDS and buzzer and uncle for HAND_LED and buzzer)
int const* peteBot[10] = { p, e, t, e, b, o, t, space, space, space }; //Eye leds gpio pin 02
int const* uncle[42] = {i,t,s,space,a,space,p,r,i,v,i,l,e,g,e,space,t,o,space,h,a,v,e,space,y,o,u,space,a,s,space,m,y,space,u,n,c,l,e,space,space,space }; //Led in Hand gpio pin 01



int noElementsInCharacter;
int timerUnitMultiplier;

//my circuit uses positive logic

void sendMessagePeteBot() {

	int characterInMessage = 0; //array counting starts at zero
	int noCharactersInMessage = 10;  //Set for peteBot message array (this has 10 characters)
	for (characterInMessage; characterInMessage < noCharactersInMessage; // array element are numbered from zero
			characterInMessage++) {

		int noElementsInCharacter = peteBot[characterInMessage][0];
		if (peteBot[characterInMessage] == space) {
			/* Turn LED Off by setting the GPIO pin low  */
			LPC_GPIO_PORT->CLR0 = 1 << EYE_LEDS;
			mrtDelay(space[1] * 100);
		} else {
			int i = 1; //first di or dah is at array[1] position
			for (i; i <= noElementsInCharacter; i++) {
				timerUnitMultiplier = peteBot[characterInMessage][i];
				/* Turn LED On by setting the GPIO pin high*/
				LPC_GPIO_PORT->SET0 = 1 << EYE_LEDS;
				mrtDelay((timerUnitMultiplier * 100));
				/* Turn LED Off by setting the GPIO pin low */
				LPC_GPIO_PORT->CLR0 = 1 << EYE_LEDS;
				mrtDelay(100);
			}
		}
		mrtDelay(300); //new character delay
	}

}

void sendMessageUncle() {

	int characterInMessage = 0; //array counting starts at zero
	int noCharactersInMessage = 42;  //Set for uncle message array (this has 42 characters)
	for (characterInMessage; characterInMessage < noCharactersInMessage; // array element are numbered from zero
			characterInMessage++) {

		int noElementsInCharacter = uncle[characterInMessage][0];
		if (uncle[characterInMessage] == space) {
			/* Turn LED Off by setting the GPIO pin low  */
			LPC_GPIO_PORT->CLR0 = 1 << HAND_LED;
			mrtDelay(space[1] * 100);
		} else {
			int i = 1; //first di or dah is at array[1] position
			for (i; i <= noElementsInCharacter; i++) {
				timerUnitMultiplier = uncle[characterInMessage][i];
				/* Turn LED On by setting the GPIO pin high*/
				LPC_GPIO_PORT->SET0 = 1 << HAND_LED;
				mrtDelay((timerUnitMultiplier * 100));
				/* Turn LED Off by setting the GPIO pin low */
				LPC_GPIO_PORT->CLR0 = 1 << HAND_LED;
				mrtDelay(100);
			}
		}
		mrtDelay(300); //new character delay
	}

}

int main(void) {
/* Initialise the GPIO block */
gpioInit();

/* Initialise the UART0 block for printf output */
uart0Init(115200);

/* Configure the multi-rate timer for 1ms ticks */
mrtInit(__SYSTEM_CLOCK / 1000);

/* Configure the switch matrix (setup pins for UART0 and GPIO) */
configurePins();

/* Set the LED pin to output (1 = output, 0 = input) */
#if !defined(USE_SWD)
LPC_GPIO_PORT->DIR0 |= (1 << EYE_LEDS);
LPC_GPIO_PORT->DIR0 |= (1 << HAND_LED);
#endif

while (1) {
#if !defined(USE_SWD)

	sendMessagePeteBot();
	sendMessageUncle();


	//Original Blinky this has oppersite logic to my circuit
	/* Turn LED Off by setting the GPIO pin high */
	//LPC_GPIO_PORT->SET0 = 1 << EYE_LEDS;
	//mrtDelay(5000);
	/* Turn LED On by setting the GPIO pin low */
	//LPC_GPIO_PORT->CLR0 = 1 << EYE_LEDS;
	//mrtDelay(500);

#else
	/* Just insert a 1 second delay */
	mrtDelay(1000);
#endif

	/* Send some text (printf is redirected to UART0) */
	printf("Love from Michael x \n\r");
}
}
