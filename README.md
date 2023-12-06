# High-speed Camera Recording Trigger

This software is developed to initiate a recording event using the high-speed camera Mars640-815UM from the MARS camera series. It received a trigger signal from a microcontroller connected via the serial port. The project focuses on capturing the movement of termites as they pass through a force-plate; when the termites activate the force-plate, the MCU reads the signal and sends a trigger signal to this software to start the recording.
## Dependencies

## Installation and usage

This software is developed to run on a Linux machine, Ubuntu 20.04 (however, this software should work on Ubuntu 18.04 and 16.04).

Firstly, clone the repository into your computer:
```git
git clone git@github.com:mintnguyenn/Soft-Trigger-Record.git
```

Then, direct to your folder directory and build the software:
```
cd Soft-Trigger-Record/
make
```

Finally, run the software:
```
./runSample.sh
```

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License
