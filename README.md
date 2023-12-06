# High-speed Camera Recording Trigger

This software is developed to initiate a recording event using the high-speed camera Mars640-815UM from the MARS camera series. It received a trigger signal from a microcontroller connected via the serial port. The project focuses on capturing the movement of termites as they pass through a force-plate; when the termites activate the force-plate, the MCU reads the signal and sends a trigger signal to this software to start the recording.

## Prerequisites

1. The ContrasTech Software Development Kit for Linux (x86) (a.k.a iCentral) - this includes the driver required to run the camera in Linux and can be found at [Contrastech](https://www.contrastech.com/en/service/005001002.html).

2. LibSerial library. To install:
```bash
sudo apt update
sudo apt install libserial-dev
```

## Installation and usage

This software is developed to run on a Linux machine, specifically Ubuntu 20.04. It should also work on Ubuntu 18.04 and 16.04.

1. Clone the repository into your computer:
```bash
git clone git@github.com:mintnguyenn/Soft-Trigger-Record.git
```

2. Navigate to the directory and build the code
```bash
cd Soft-Trigger-Record/
make
```

3. Run the software:
```bash
./runSample.sh
```

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License
