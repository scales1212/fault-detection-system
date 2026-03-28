## Fault Detection System
This project monitors home equipment at 50Hz using UDP to ensure the systems are running correctly. This project comes in 3 parts: a C++ program that has 1-many UDP interfaces with other systems. It runs at 50Hz and, at each update, sends a command request for response from each device. At each update, it checks the queues and verifies non of the fault variables indicate a failure. If they do, it logs a failure in a log file. The second program is also in C++ and runs the models of the other devices in the loop. There can be 1-many of these other devices, but they all communicate through UDP and only send responses when prompted by the main monitoring system. Finally, there is a Python program that operates a GUI for the user to test the overarching system, with a window that shows the live log file and has a way of looking at each device and can click different variables to over write to a bad value (all variables are boolean that can trigger a fault) for a single send, before the fault bounces back to a good state. All 3 will run in different processes on the same system. 

## Device List

temperature control unit command message structure:
- UID
- command
- value

temperature control unit response message structure:
- UID
- command
- current_temp
- set_temp
- overheating
- out_of_bounds_temp

garage door command message structure:
- UID
- command
- value

garage door response message structure:
- UID
- command
- garage_open
- light_on
- over_voltage

## Message Definition
This project uses JSON structures to define messages, then will contain an autogen script that'll run before any build happens to re-autogenerate C++/Python message structures. This allows for JSON message definition as truth and the only thing needed to update. Messages are stored in a common directory for maintainability. 