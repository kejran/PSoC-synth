# PSoC-synth

A simple implementation of DSS-based musical synthesizer with minimal cost (less than $10) hardware.

## Devices
The application was designed for the *PSoC® 4 CY8CKIT-049 42xx* cheap evaluation kit, which uses a 48MHz ARM Cortex-M0 microprocessor. Code includes cypress-specific interrupt and peripheral handling. However, it should be adaptable to all 32-bit ARM microcontrollers.

## Peripherals
Application requires a filtered DAC with at least  8-bit depth. Implementation uses embedded IDAC with a ~1K resistor to the ground, followed by a simple lowpass-filter (~80Ohm resistor + 0.1uF capacitor), and finally the internal OpAmp as Unity Buffer. Output should be connected to a speaker or audio jack.

## Features
The currently implemented features are:
- Sine wave generation with DSS
- voice polyphony
- MIDI over UART
- Smooth output clipping to [-1, 1] range

Planned features include:
- ADSR envelope for notes
- More waveforms
- MIDI channel handling 
- MIDI CC handling for parameter modification
- Tweakable low/high-pass filter
- Unison mode
- Pitch bending & tremolo
- LFO

##

<img src='https://dl.dropboxusercontent.com/u/93510029/synth.jpg'>