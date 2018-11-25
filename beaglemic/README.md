# BeagleMic 16-channel PDM Audio Capture

# Introduction
Ever wanted to record audio from 16 PDM microphones simultanously? Now you can with a BeagleMic running on a [PocketBeagle](https://beagleboard.org/pocket).

Yes, you could opt for the much simpler I2S microphones. But then you won't have fun writing assembly to capture and process sixteen digital signals at more than 2MHz sample rate.

Current firmware supports:

| Feature                    | Support           |
|----------------------------|-------------------|
| PDM Bit Clock              | 2,273 MHz         |
| PCM Output Bits Per Sample | 16 bps            |
| PCM Output Sample Rate     | 35511 Samples/sec |

# Hardware
The schematic is simple. PDM microphones' digital outputs are connected directly to PocketBeagle's PRU input pins. The PRU also drives the bit clock.

| PocketBeagle Pin | PRU Pin | Type  | Signal               |
|------------------|---------|-------|----------------------|
| P2.24            | R30_14  | Output| PDM Bit Clock        |
| P1.36            | R31_0   | Input | MIC0 and MIC1 Data   |
| P1.33            | R31_1   | Input | MIC2 and MIC3 Data   |
| P2.32            | R31_2   | Input | MIC4 and MIC5 Data   |
| P2.30            | R31_3   | Input | MIC6 and MIC7 Data   |
| P1.31            | R31_4   | Input | MIC8 and MIC9 Data   |
| P2.34            | R31_5   | Input | MIC10 and MIC11 Data |
| P2.28            | R31_6   | Input | MIC12 and MIC13 Data |
| P1.29            | R31_7   | Input | MIC14 and MIC15 Data |

For each microphone pair, one microphone is configured to output data on the rising clock edge, and the other is configured to output data on the falling edge. This way we need only 8 input GPIOs to capture data from all 16 microphones.

Microphone breakout board and a PocketBeagle Cape are provided in KiCad format. The cape PCB is only for convenience **and has not been tested yet**. You can easily use a breadboard and wires instead.

Unfortunately the breakout board is essential for a home DIY user like me, since all PDM microphones I could find are BGA. There are numerous youtube guides how to solder BGAs at home using a skillet or a toaster oven.

# Software
PRU0 takes care of driving the PDM bit clock and capturing the microphone bit data. It then runs a CIC filter to convert PDM to PCM, and feeds PCM data to PRU1.

PRU1 is retransmitting PCM data from PRU0 down to the ARM host, using RPMSG.

The host program simply gets data from the RPMSG device file and dumps it on its ```stdout``` for further processing.

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
    sudo ./record >out.raw
    # Hit Ctrl+C to stop.
    sox -r 35511 -e signed -b 16 -c 16 -t raw out.raw out.wav


# Further Work
A few ideas to improve the design:

 * If input is limited toto 8 microphones, then process and output 24bit PCM data.
 * Move comb filters to PRU1, and try to add more integrators in PRU0.
 * Clean-up the cape PCB.

# References
 * [CIC Filter Introduction](https://dspguru.com/dsp/tutorials/cic-filter-introduction/)
 * [Another CIC Filter Article](http://www.tsdconseil.fr/log/scriptscilab/cic/cic-en.pdf)
 * [Series of PDM articles and software implementations](https://curiouser.cheshireeng.com/category/projects/pdm-microphone-toys/)
 * [Another CIC Filter Article](https://www.embedded.com/design/configurable-systems/4006446/Understanding-cascaded-integrator-comb-filters)
 * [Datasheet for the PDM microphones I've used](http://media.digikey.com/PDF/Data Sheets/Knowles Acoustics PDFs/SPM0423HD4H-WB.pdf)
 * [Inspiration for high-bandwidth data acquisition](https://github.com/ZeekHuge/BeagleScope)
