# TDM Simulator

This is work-in-progress. It's not tested yet.

# Introduction

Quickly hacked TDM simulator.

 * Hard-coded TDM slot configuration.
 * Reliance on long TDM frame (in order to catch Frame Sync).

# Hardware

| PB Pin | BBW Pin | PRU Pin | Type  | Signal                 |
|------------------|---------|-------|------------------------|
| P2.24  | P8_12   | R30_14  | Output| TDM Data Output        |
| P2.33  | P8_11   | R30_15  | Output| TDM Data Output Enable |
| P1.36  | P9_31   | R31_0   | Input | TDM Bit Clock          |
| P1.33  | P9_29   | R31_1   | Input | TDM Frame Clock        |
| P2.32  | P9_30   | R31_2   | Input | TDM Data Input         |
| P2.30  | P9_28   | R31_3   | Input |                        |
| P1.31  |         | R31_4   | Input |                        |
| P2.34  | P9_27   | R31_5   | Input |                        |
| P2.28  |         | R31_6   | Input |                        |
| P1.29  | P9_25   | R31_7   | Input |                        |

# Software
PRU0 takes care of driving the I/O pins. PRU1 is retransmitting the PCM data to/from PRU0 down to the ARM host, using RPMSG.

# Running The Example

    # Build PRU firmware
    cd pru
    make
    # Start the new PRU firmware
    sudo ./start.sh
    # Build the host program
    cd ../host
    make
    # Record audio
    sudo ./record >out.raw <in.raw
    # Hit Ctrl+C to stop.
    sox -r 44100 -e signed -b 16 -c 1 -t raw out.raw out.wav


# References
 * [Inspiration for high-bandwidth data acquisition](https://github.com/ZeekHuge/BeagleScope)
