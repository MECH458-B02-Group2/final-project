# final-project

## Notes from the top of the skeleton code

Open up the document in START -> WinAVR -> AVR LibC -> User Manual -> avr/interrupt.h
Chapter 15, in Full Manual... THIS HAS A LOT OF IMPORTANT INFO...I have mentioned this at least 3 times!!!

For those that are still having major problems, I've seen about 1/3 of the class with major problems in
code structure. If you are still having major problems with your code, it's time to do a VERY quick overhaul.
I've provided a skeleton structure with an example using two input capture interrupts on PORTDA0 and A3
Please try this in the debugger.

Create a watch variable on STATE. To do this right click on the variable STATE and then
Add Watch 'STATE'. You can see how the variable changes as you click on PINDA0 or PINDA3. Note that the interrupt
catches a rising edge. You modify this to suit your needs.
