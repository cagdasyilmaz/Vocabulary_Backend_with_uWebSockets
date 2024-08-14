# Vocabulary_Backend

In this project backend server application is running on Linux OS. Backend server was implemented with the help of uWebSockets framework. Perfomance indications of the UWebSockets framework outperform the Restbed framework which was implemented before. Before using this framework uSockets library and a necessary header file mut be installed the OS which runs the application. 

 The libusockets.h and uSockets.a will be in the subdirectory uSockets of the repository that can be cloned via uWebSockets. libusockets.h is in uWebSockets/uSockets/src and the uSockets.a is in uWebSockets/uSockets.

We need to do the following copy opearitons:

1. Copy the libusockets.h to /usr/local/include
2. Copy the uSockets.a to /usr/local/lib

Please also check the other necessary installation to avoid getting CMake error at compilation.
