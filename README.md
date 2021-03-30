# RaspberryPi part from IndoorAir-project

install cmake and g++ packages (if not found)

    sudo apt install cmake g++

install sqlite3

    sudo apt install libsqlite3-dev

create new build directory

    mkdir build

    cd build

configure cmake files

    cmake ../

build project

    make

running program takes serial port name as argumet. Typically its named ttyACM0 when nucleo is plugged in,
but you can check it in under /dev/-directory what file shows up when plugging nucleo on serial port.

run program (read values from serial port and save in database)

    sudo ./indoorair-server ttyACM0

show database entries in terminal

    ./indoorair-server debug
