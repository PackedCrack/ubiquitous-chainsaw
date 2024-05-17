# Ubiquitous Chainsaw
This is a proximity based authentication system that will re-enable full-disk encryption if the user strays too far from the computer.
This is done by continuous communication between the computer and a microcontroller (the access token) over BLE. 
If the median RSSI value falls too low, the computer will enter hibernation mode. 

Why hibernate?
When the computer hibernate it will clear RAM (and thus any stored encryption keys). But before powering down the OS will write the state of each process to disk.
Once the computer restart the user then has to re-enter their full-disk encryption password in order to boot into the OS.
The OS will then restore all processes to the state that they were when hibernation happened.

In order to "pair" your computer with the access token, the token should be connected to the computer over USB.
From there the client can be used to generate the ECC keys and it will automatically send them over serial to the access token.

Goes without saying but it requires that full-disk encryption is enabled. Such as BitLocker or VeraCrypt.

# Client
The desktop program that demands RSSI values from the access token and tracks the median. Will force the computer to hibernate if deemed necessary. 
As of now only Windows 10/11 is supported. Linux support is not supported but a goal.
# Server
This is the microcontroller that servers as the physical access token. For development Esp32-S3 was used.
The python script *should* allow for configuring for any ESP device, as long as it supports BLE (but I cannot confirm this).
The end goal is to have this running on a custom PCB that is much smaller than the development board.

## GIF showing the protection mechanism
![Cowabunga](cowabunga.gif)
