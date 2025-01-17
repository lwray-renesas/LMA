## Running the simulator
1. Use CMake to generate your build environment.
2. In your debug environment the executable must be called with the following command line arguments:
    ns:    Number of samples to simulate - integer e.g., ns=39060
    fs:    Sampling frequency to simulate - double e.g., fs=3609.25
    fline: Line frequency to simulate - double e.g., fline=50.0
    vrms:  RMS Voltage to simulate - double e.g., vrms=230.0
    irms:  RMS Current to simulate - double e.g., irms=10.0
    ps:    Phase shift of current signal relative to voltage signal to simulate - double e.g., ps=10.0
    calib: 0 = disabled calibration on startup, !0 = enable calibration on startup

    For example to invoke the executable:
    LMA-sim-windows ns=39060 fs=3906.25 fline=50.0 vrms=230.0 irms=10.0 ps=0.0 calib=0

    So the debug environment must replicate this.

    An example launch.json is:
    ```
    {
    "version": "0.2.0",
    "configurations": [
      {
        "name": "Launch LMA-sim-windows.exe",
        "type": "cppvsdbg",
        "request": "launch",
        "program": "${workspaceFolder}/build/bin/Debug/LMA-sim-windows.exe",
        "args": [
          "ns=39063",
          "fs=3906.25",
          "fline=50.0",
          "vrms=230.0",
          "irms=10.0",
          "ps=0.0",
          "calib=0"
        ],
        "cwd": "${workspaceFolder}",
        "stopAtEntry": false,
        "environment": [],
        "console": "integratedTerminal"
      }
    ]
  }
  ```
