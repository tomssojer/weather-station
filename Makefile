PROGRAM=transmitter
PROGRAM_SRC_FILES=./receiver.cpp
EXTRA_COMPONENTS = extras/RF24 extras/bmp280 extras/i2c extras/dhcpserver

include ~/esp-open-rtos/common.mk
