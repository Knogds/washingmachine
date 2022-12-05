
(translated from Croatian)

To understand the code: everything happens in washingmachine.c: systick handler.

Do not use the port: A9

The TRIAC turns on by sending a short pulse:
                                                                                   
for the smallest value - immediately before the zero point AC
for the highest value - immediately after the zero point AC
                                                                                 
The impulse should be as short as possible because if it is too long, then it
heats up unnecessarily. The triac turns off by itself itself with each change
of polarity of the AC.
                                                                                
The LCD I use is ILI9340:
- http://tech.scargill.net/an-arduino-terminal/
- code: https://bitbucket.org/scargill/experimental-terminal


LCD	<-->	MBO	(Default alt.funct.)
--------------------------------------------
SCK         A5 SPI1_SCK
SDO(MISO)   A6 SPI1_MISO
SDI(MOSI)	A7 SPI1_MOSI
D/C         B0
RESET       B1
CS          B10

GND         GND
VCC         3.3
LED         3.3+100_OHM resistor


...


gdje poceti?
sve se odvija u

washingmachine.c: sys_tick_handler

----------------------------------------

NE KORISTI PORTOVE:
A9
-------------------
TRIAC se ukljucuje tako da se salje kratki impuls:

za najmanu vrijednost - neposredno prije nulte tocke AC
za najvecu vrijednost - neposredno poslije nulte tocke AC


Impuls treba biti sto kraci jer ako se cijelo vrijeme drzi ukljucen sa strane
mikrokontrollera, onda se bespotrebno zagrijava. Triac se iskljucuje sam od
sebe prilikom svake promjene polariteta izmjenicne struje.

LCD koji koristim je ILI9340:
- http://tech.scargill.net/an-arduino-terminal/
- kod: https://bitbucket.org/scargill/experimental-terminal

