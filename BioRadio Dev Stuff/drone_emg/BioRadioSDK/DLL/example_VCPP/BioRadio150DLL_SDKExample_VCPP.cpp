/*---------------------------------------------------------------------------

This example was compiled and tested using Microsoft Visual Studio 2008.

Please Note: The example assumes you are using a CleveMed USB Receiver communication dongle.
If you are using a legacy Computer Unit, please change the call to CreateBioRadio
to pass a value of true instead of false.

------------------------------------------------------------------------------*/
#include <iostream>
#include <cstdlib>
#include <conio.h>
#include <string.h>
#include <direct.h>
#include <Windows.h>

#include "BioRadio150DLL.h"

using namespace std;

int main(int argc, char* argv[])
{
        /* set up path to example config file */
        char *configFileName = "\\ExampleConfig.ini";
        char* fullConfigFilePath = new char[100];
        _getcwd(fullConfigFilePath, 100);
        strcat(fullConfigFilePath, configFileName);

        const int FAST_INPUTS_BUFFER_SIZE = 20000;
        const int SLOW_INPUTS_BUFFER_SIZE = 10000;
        const int ACQUISITION_INTERVAL = 50; // (milliseconds)

        /* FIND DEVICES */
        const int MAX_DEVICES = 10;
        TUpdateStatusEvent UpdateStatusFunc = NULL;
        char *portName;
        int deviceCount = 0;
        TDeviceInfo DeviceList[MAX_DEVICES];
        cout << "Searching for BioRadio 150 Computer Units..." << endl;
        FindDevices(DeviceList, &deviceCount, MAX_DEVICES, UpdateStatusFunc);
        cout << "Found " << deviceCount << " BioRadio 150 Computer Units:";
        if (deviceCount > 0) {
                for (int i=0; i<deviceCount; i++) {
                        cout << ((i!=0) ? ", " : " ") << DeviceList[i].PortName;
                }
                portName = DeviceList[0].PortName;
                cout << endl << "Using " << portName << "." << endl;
        } else {
                cout << "Exiting." << endl;
                system("pause");
                return -1;
        }

        unsigned long int devHandle;

        /* Create the BioRadio handle.  Pass a value of true if you are using a legacy Computer Unit to connect to
               your BioRadio.  Otherwise, pass false */
        devHandle = CreateBioRadio(false);
        cout << endl << "Create BioRadio Object... (" << devHandle << ")" << endl;

        SetBadDataValues(devHandle, -65535, 65535);

        if (devHandle>0) {
                /* START COMMUNICATION */
                int successFlag = StartCommunication(devHandle, portName);
                cout << "Start Comm... (" << successFlag << ")" << endl << endl;
                if (successFlag > 0) {
                        /* START ACQUISITION */
                        successFlag = StartAcq(devHandle, 1);
                        cout << "Start Acq... (" << successFlag << ")" << endl;
                        if (successFlag > 0) {
                                /* PROGRAM CONFIGURATION */
                                successFlag = ProgramConfig(devHandle, 1, fullConfigFilePath);
                                cout << "Program Config... (" << successFlag << ")\n" << endl;
                                if (successFlag > 0) {
                                        int numFastMainInputs, numFastAuxInputs, numSlowInputs;
                                        int numTotalInputs = GetNumEnabledInputs(devHandle,
                                                             &numFastMainInputs, &numFastAuxInputs, &numSlowInputs);
                                        cout << "Inputs: " << numFastMainInputs << " fast main, " << numFastAuxInputs <<
                                             " fast aux, " << numSlowInputs << " slow, " << numTotalInputs << " total" << endl;
                                        cout << "Sample Rate = " << GetSampleRate(devHandle) << endl;

                                        double FastInputsData[FAST_INPUTS_BUFFER_SIZE];
                                        WORD SlowInputsData[SLOW_INPUTS_BUFFER_SIZE];
                                        int FastInputsNumRead, SlowInputsNumRead, readReturn;
                                        int statusCount = 0;

                                        while (!kbhit()) {
                                                /* REQUEST DATA EVERY 50MS UNTIL KEY PRESSED */
                                                cout << endl << "Tfr Buffer... (" << TransferBuffer(devHandle) << ")" << endl;
                                                readReturn = ReadScaledFastAndSlowData(devHandle,
                                                                                       FastInputsData, FAST_INPUTS_BUFFER_SIZE, &FastInputsNumRead,
                                                                                       SlowInputsData, SLOW_INPUTS_BUFFER_SIZE, &SlowInputsNumRead);
                                                cout << "Read Data... (" << readReturn << ")" << endl;
                                                cout << "Fast Inputs Num Read = " << FastInputsNumRead << endl;
                                                cout << "Slow Inputs Num Read = " << SlowInputsNumRead << endl;
                                                if (statusCount++ >= 20) {
                                                        statusCount = 0;
                                                        cout << endl;
                                                        cout << "  BatteryStatus = " << GetBatteryStatus(devHandle) << endl;
                                                        cout << "  RSSI: " << GetUpRSSI(devHandle) << " up / " << GetDownRSSI(devHandle) << " down" << endl;
                                                        cout << "  BufSz: " << GetLinkBufferSize(devHandle) << endl;
                                                        cout << "  BEC: " << GetBitErrCount(devHandle) << endl;
                                                        cout << "  BER: " << GetBitErrRate(devHandle) << endl;
                                                        cout << "  Pkts: " << GetGoodPackets(devHandle) << " good " <<
                                                             GetBadPackets(devHandle) << " bad " <<
                                                             GetDroppedPackets(devHandle) << " dropped" << endl;
                                                }
                                                Sleep(ACQUISITION_INTERVAL);
                                        }
                                }
                                /* STOP ACQUISITION */
                                cout << endl << "Stop Acq... (" << StopAcq(devHandle) << ")" <<  endl;
                        }
                        /* STOP COMMUNICATION */
                        cout << endl << "Stop Comm... (" << StopCommunication(devHandle) << ")" << endl;
                }
                successFlag = DestroyBioRadio(devHandle);
                cout << "Destroy BioRadio Object... (" << successFlag << ")\n" << endl;
        }

        system("pause");
        return 1;
}
//---------------------------------------------------------------------------
