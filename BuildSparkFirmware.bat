echo Copying all custom code over to official firmware

xcopy /s SparkGreenhouseFirmware SparkOfficialFirmware

echo Building and Flashing Firmware: Make sure to put device in DFU Mode

cd SparkOfficialFirmware\modules
make clean all PLATFORM=photon -s program-dfu

pause