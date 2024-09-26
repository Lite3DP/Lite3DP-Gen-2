**We will list here the issues found on the Crowd Supply campaign machines, and their solutions**

First of all, PLEASE READ THOROUGHLY:

1) [Quick start guide and slicer configuration](https://github.com/Lite3DP/Lite3DP-Gen-2/tree/main/Gen%202%20-%20Quick%20Start%20Guide)

2) [Crowd Supply updates](https://www.crowdsupply.com/lite3dp/lite3dp-gen-2/updates). Important information was included here that cannot be found elsewhere.

For those new to MSLA 3D printing, building knowledge will take some time. Be patient and enjoy the process, study and consult different online sources, there is a lot of information that does not make sense to replicate here. Trial and error will make you an expert.

The machines were rigorously tested before being shipped. However, we have heard of some alterations due to the long journey around the globe:

1) First of all, a damaged micro SD card can affect the startup and operation of the machine. Please make sure to start and test it without inserting any card.

2) If the touchscreen display does not display anything when you turn on the machine, or displays a blurry image, or does not respond to touch:

   2.1) The flexible flat cable connecting the touchscreen board to the main board may be losing contact with one of the two connectors. Please loosen the 4 M3x12 screws on the front of the machine and check that the cable is connected properly.

   2.2) If the above did not resolve the issue, it may be that there is an exposed trace on the main board that is touching the black aluminum rectangular tube, the one under the mask display. To check, please carefully remove the 2 M2 screws holding the linear guide to the rear aluminum plate of the cover (the bottom one and the top one, those who have the nuts). Then remove all 12 black M3 screws from the base of the machine. Disconnect the 4-wire cable that connects the main board to the base board. Check that there are no exposed traces around the display (it will look gold), at the bottom of the main board and around the mask display. If so, simply place a piece of electrical tape over the exposed trace and reassemble.

3) Some thermoformed plastic covers may have contracted during manufacturing, causing them to scratch against the folded aluminum plate. Here you can simply remove the 4 M3 screws and turn the black POM wheels over, leaving the letters "L" and "R" facing the inside of the machine, like the following image:

![IMG_20240926_074132](https://github.com/user-attachments/assets/6bf5756a-f067-475d-871a-aac65bc21569)


**Some clarifications:**

1) All machines have been delivered with firmware v1.0 loaded.

2) Next to the power button, at the bottom of the main board, works the LDO voltage regulator, which can reach 90°C/200°F in normal operation.

3) In the menu, after selecting the micro SD card folder, you should be able to see a brief preview with the sections of the part/s to be printed. If not, check that the Slicer configuration is correct or that the micro SD card is in FAT32 format (and not damaged).
