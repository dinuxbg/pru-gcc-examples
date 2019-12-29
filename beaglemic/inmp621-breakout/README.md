This is a KiCAD project for a simple breakout board for the INMP621 PDM mems microphone.

This board deliberately does not follow the spm0423hd4h-breakout pinout. The spm0423hd4h-breakout was designed to be connected via long wires to a main board, so it had:
 * Two microphones per breakout board, to reduce overall wiring.
 * GND wire between all other signals, to reduce inter-signal EMI when using long flat cables.

Soldering flat cables proved too cumbersome, so I abandoned that design. While soft cable wiring is still possible, the new INMP621 breakout board is envisioned to be directly connected to a main board using pin headers.. For compatibility's sake, I used same pinout as [Adafruit's PDM board](https://www.adafruit.com/product/3492).

References:
 * Microphone footprint, 3D model and schematics symbol are taken from https://www.snapeda.com/parts/INMP621/InvenSense/view-part/?ref=digikey . Footprint has minor modification to "break" the ground track and ease the data wires routing.
