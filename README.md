# final-project

## Workflow

To begin the development of a new feature, create a new branch with a brief descriptive name (eg. ferro-sensor) beside the main branch and checkout that branch. Commit any related changes to the code to the feature branch (and not the main branch). When debugging, branch again off of the feature branch and use a similar name (eg. ferro-sensor-debug). Instead of overwriting your attempts, commit each iteration of your debugging process to the debugging branch. When you have successfully debugged the code, either merge the debugging branch back into the new branch, or checkout the feature branch, make the necessary changes, and commit there (if the changes are minor or if the debugged code has become too messy). When the feature is complete and debugged, merge the feature branch back into the main branch. Other branches can then be rebased if they need to utilize the recently merged updates.

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
