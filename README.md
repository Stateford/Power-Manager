# Power Manager

An application that watches for specified executables and changes power options to High Perfomance when they are launched.

## NOTE:
This is still a work in progress. The Service currently has permissions issues.


#### Architecture
This repo contains two dynamically linked libraries, a service, a C# GUI for updating the configs and controlling the service, and a C++\CLR runtime interop for the C# UI.