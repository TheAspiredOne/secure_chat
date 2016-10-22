Alden Tan
Avery Tan


A simple chat program implementing a client/server system between 2 connected Arduinos 
which allow users to type text on their command line which is encrypted using the HD public
key encryption scheme and sent to the other terminal via the Arduino client/server system.



Accessories:
1. 560 Ohm (Green Blue Brown) Resistor (x1)
2. Arduino Mega Board (AMG) (x2)
3. Wires

Wiring instructions:
Arduino1 GND <-> Arduino2 GND
Arduino1 Pin 14 (TX3) <-> Arduino2 Pin 15 (RX3)
Arduino2 Pin 14 (TX3) <-> Arduino1 Pin 15 (RX3)
Arduino1 Pin 13 <-> Resistor <-> Arduino1 GND

(Arduino1 = First Arduino)
(Arduino2 = Second Arduino)

Instrustions: 
Arduino1 will be your client in the handshaking protocol and Arduino 2 
will be the server. Simply upload the code into both arduinos then open
then launch serial monitor for Arduino1 followed by Arduino 2. 

