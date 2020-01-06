# Arduino PCA9634 library

## Installation

There are two ways to install this library:
1. Using the Arduino IDE Library Manager (Sketch > Include Library > Manage Libraries), search for "PCA9634 library". From the result list, select this library and click "Install".
2. Checkout this repository in your `~/Arduino/library/` folder. When using the Arduino IDE, you should be able to find this library under Sketch > Include Library > PCA9634 library.

## Usage

Every library method and parameters is documented. 

## Reserved I2C addresses

The I2C bus lets a master and a number of slaves to communicate. The master selects the slave with which wants to speak by using an address.

When designing a schematic with multiple ICs connected to the same I2C bus, you have to make sure not to create conflicts between those devices. If two devices have the same address, they will receive the same commands. This can be wanted or make the board useless.

The PCA9634 can use a number of addresses both to avoid conflicts and to create groups of receivers.

|Address name     |I2C address          | Active at power-on/reset? | Can it be enabled/disabled? | Can it be changed? |
|-----------------|---------------------|---------------------------|-----------------------------|--------------------|
|Hardware address |\<A7-A0 selectable\> | Yes                       | No                          | No                 |
|All call         |0x70                 | Yes                       | Yes                         | Yes                |
|Subaddress1      |0x71                 | No                        | Yes                         | Yes                |
|Subaddress2      |0x72                 | No                        | Yes                         | Yes                |
|Subaddress3      |0x74                 | No                        | Yes                         | Yes                |


The "All call" address is enabled at startup, can be changed with the method `setAllCallAddressActive(addr)` and disabled with `setAllCallAddressInactive()`.
The three subaddresses are instead disabled by default and you can enable them with `setSubaddressXActive(addr)`.

> !!! WARNING !!! The I2C address `0x03` is used for resetting the chip, so you have to avoid at all cost to have another slave with that address on the same bus. If this cannot be done, make sure that you never write the sequence `0xA5`, `0x5A` to that slave, as it would reset the PCA9634.
