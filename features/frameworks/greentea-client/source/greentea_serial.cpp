#include "greentea-client/greentea_serial.h"
#include "mbed_config.h"

SingletonPtr<GreenteaSerial> greentea_serial;

GreenteaSerial::GreenteaSerial() : mbed::RawSerial(USBTX, USBRX, MBED_CONF_PLATFORM_STDIO_BAUD_RATE) {};
