# GreenHouseDevice
Device firmware based on the particle.io for the greenhouse gardening robot

## How to build the firmware
####Prerequisite: 
Set up the official formware git clone in subfolder "SparkOfficialFirmware"
Follow the instructions to install the tools to create a local build as described here: https://github.com/spark/firmware/tree/latest

###Build and Flash:
1. set photon into DFU mode by holding down the mode/setup button on the device and then tapping on the RESET button once. 
	Release the MODE/setup button after you start to see the RGB LED flashing in yellow. It's easy to get this one wrong: Make sure you don't let go of the MODE/SETUP button until you see flashing yellow, about 3 seconds after you release the RESET button. 
	A flash of white then flashing green can happen when you get this wrong. You want flashing yellow.
	
2. execute the BuildSparkFirmware.bat batch

