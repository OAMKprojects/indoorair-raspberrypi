# RaspberryPi part from IndoorAir-project

install cmake and g++ packages (if not found)

    sudo apt install cmake g++

install sqlite3

    sudo apt install libsqlite3-dev

create new build directory

    mkdir build

    cd build

configure cmake files without admin app

    cmake ../
    
configure camke with admin app

    cmake -DADMIN=ON ../

build project

    make

running program takes serial port name as argumet. Typically its named ttyACM0 when nucleo is plugged in,
but you can check it in under /dev/-directory what file shows up when plugging nucleo on serial port.

run program (read values from serial port and save in database)

    sudo ./indoorair-server ttyACM0
    
run program with admin app extensin (require compiled with option ADMIN=ON)

    sudo ./indoorair-server ttyACM0 admin

show database entries in terminal

    ./indoorair-server debug
