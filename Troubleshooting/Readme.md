**We will list here the issues found on the Crowd Supply campaign machines, and their solutions**

First of all, PLEASE READ THOROUGHLY:

1) [Quick start guide and slicer configuration](https://github.com/Lite3DP/Lite3DP-Gen-2/tree/main/Gen%202%20-%20Quick%20Start%20Guide)

2) [Crowd Supply updates](https://www.crowdsupply.com/lite3dp/lite3dp-gen-2/updates). Important information was included here that cannot be found elsewhere.

For those new to MSLA 3D printing, building knowledge will take some time. Be patient and enjoy the process, study and consult different online sources, there is a lot of information that does not make sense to replicate here. Trial and error will make you an expert.

The machines were rigorously tested before being shipped. However, we have heard of some alterations due to the long journey around the globe:

1) First of all, a damaged micro SD card can affect the startup and operation of the machine. Please make sure to start and test it without inserting any card.

2) If the touchscreen display does not display anything when you turn on the machine, or displays a blurry image, or does not respond to touch:

There is most likely **an exposed trace on the main board that is touching the black aluminum rectangular tube**, the one under the mask display. To check, please carefully remove the 2 M2 screws holding the linear guide to the rear aluminum plate of the cover (the bottom one and the top one, those who have the nuts). Then remove all 12 black M3 screws from the base of the machine. Disconnect the 4-wire cable that connects the main board to the base board. Be careful of the loose inner reflective cone. Look for scratched traces around the display, yellow area in the image below. If so, **simply place a piece of electrical tape over the exposed trace and reassemble.** (Before reassembling, please check that the flexible flat cable that connects the touch board to the main board is correctly positioned.) 

![image](https://github.com/user-attachments/assets/bdaee06c-19ef-478f-b6a8-3973f620e76d)

Example of what an exposed trace looks like:

![370587735-61a19d3a-b51b-4551-bf8a-fbd302da99e3](https://github.com/user-attachments/assets/1c40bf3e-3446-4f5a-a473-d0db40d9e2e6)


3) Some thermoformed plastic covers may have contracted during manufacturing, causing them to scratch against the folded aluminum plate. Here you can simply remove the 4 M3 screws and turn the black POM wheels over, leaving the letters "L" and "R" facing the inside of the machine, like the following image (Please be careful when handling the cover. Excessive force may break it):

![IMG_20240926_074132](https://github.com/user-attachments/assets/6bf5756a-f067-475d-871a-aac65bc21569)


4) Manually stretching the silicone cover tightly, from the edges and in both directions, will help a better fit on the resin vat. (Repeat several times)

![image](https://github.com/user-attachments/assets/8ee62d7b-7d2d-41ad-ad7d-cb174affc913)


5) The carriage includes a concave magnet and ball system, allowing for convenient clamping, easy platform leveling and tightening with a single hand knob screw. This tightening is achieved through the action of 2 fixed screws and the hand knob screw. When tightening the latter, the 3 screws must be symmetrical in contact with the 10mm diameter spherical part of the platform. It is a system as simple as it is effective, achieving a perfectly firm hold, although for its proper functioning a correct position of the two fixed screws is necessary.
In the latest revisions to the Gen 2 design, and in order to facilitate the positioning of the two fixed screws, we decided to include two CNC machined nuts. However, due to small inaccuracies in the length of the screws or the placement of the threaded insert, there may be some cases where the screws do not reach the desired depth, even by adjustment. In these cases it is recommended to proceed as follows:

a) Raise the carriage to its maximum height, to facilitate the operation. (As always, be careful not to exceed the maximum height.)

b) Unscrew the two fixed screws and remove the two machined nuts.

c) Re-screw the two screws without the machined nuts (store them just in case).

d) To achieve the proper depth of the two fixed screws, insert the platform into the concave magnet. Screw (preferably manually for greater sensitivity) the two screws until you feel that there begins to be a slight resistance.

e) Check that the adjustment was correct by tightening the platform with the hand knob screw. A correct position of the screws will ensure that when tightening the hand knob screw the platform will be perfectly fixed, and will not move or rotate during tightening. (See reference image in section 10.2 of the quick start guide).


**Some clarifications:**

1) All machines have been delivered with firmware v1.0 loaded.

2) Next to the power button, at the bottom of the main board, works the LDO voltage regulator, which can reach 90°C/200°F in normal operation.

3) In the menu, after selecting the micro SD card folder, you should be able to see a brief preview with the sections of the part/s to be printed. If not, check that the Slicer configuration is correct or that the micro SD card is in FAT32 format (and not damaged).

4) In all types of release films, we have experimentally observed that the peel forces are higher in recently installed films, so some precautions must be taken: select lower lifting speeds and if possible print smaller sections.
