#include "test_env.h"

DigitalOut myled(LED1);
DigitalOut led2(LED2);

volatile int checks = 0;
void in_handler() {
    checks++;
    led2 = !led2;
}

#if defined(TARGET_KL25Z)
#define PIN_OUT     PTC6
#define PIN_IN      PTA5

#elif defined(TARGET_KL05Z)
#define PIN_OUT     PTB11
#define PIN_IN      PTB1

#elif defined(TARGET_EFM32LG_STK3600) \
    ||defined(TARGET_EFM32GG_STK3700) \
    ||defined(TARGET_EFM32TG_STK3300) \
    ||defined(TARGET_EFM32_G8XX_STK) \
    ||defined(TARGET_EFM32WG_STK3800)
#define PIN_OUT     PB12
#define PIN_IN      PD5

#elif defined(TARGET_EFM32ZG_STK3200)
#define PIN_OUT     PA1
#define PIN_IN      PD5

#elif defined(TARGET_LPC812)
#define PIN_OUT     D10
#define PIN_IN      D11

#elif defined(TARGET_LPC1114)
#define PIN_OUT     dp1
#define PIN_IN      dp2

#elif defined(TARGET_LPC1549)
// TARGET_FF_ARDUINO cannot be used, because D0 is used as USBRX (USB serial
// port pin), D1 is used as USBTX
#define PIN_OUT     D2
#define PIN_IN      D7

#elif defined(TARGET_LPC4088)
#define PIN_IN      (p11)
#define PIN_OUT     (p12)

#elif defined(TARGET_NUCLEO_F103RB) || \
    defined(TARGET_NUCLEO_L152RE) || \
    defined(TARGET_NUCLEO_F302R8) || \
    defined(TARGET_NUCLEO_F030R8) || \
    defined(TARGET_NUCLEO_F401RE) || \
    defined(TARGET_NUCLEO_F411RE) || \
    defined(TARGET_NUCLEO_F072RB) || \
    defined(TARGET_NUCLEO_F334R8) || \
    defined(TARGET_NUCLEO_L053R8)
#define PIN_IN      PB_8
#define PIN_OUT     PC_6

#elif defined(TARGET_DISCO_F407VG)
#define PIN_OUT    PC_12
#define PIN_IN     PD_0

#elif defined(TARGET_FF_ARDUINO)
#define PIN_OUT    D0
#define PIN_IN     D7

#else
#define PIN_IN      (p5)
#define PIN_OUT     (p25)

#endif

DigitalOut out(PIN_OUT);
InterruptIn in(PIN_IN);

void flipper() {
    for (int i = 0; i < 5; i++) {
        out = 1; myled = 1; wait(0.2);

        out = 0; myled = 0; wait(0.2);
    }
}

int main() {
    out = 0; myled = 0;
    //Test falling edges first
    in.rise(NULL);
    in.fall(in_handler);
    flipper();

    if(checks != 5) {
        printf("falling edges test failed: %d\n",checks);
        notify_completion(false);
    }

    //Now test rising edges
    in.rise(in_handler);
    in.fall(NULL);
    flipper();

    if (checks != 10) {
        printf("raising edges test failed: %d\n",checks);
        notify_completion(false);
    }

    //Now test switch off edge detection
    in.rise(NULL);
    in.fall(NULL);
    flipper();

    if (checks != 10) {
        printf("edge detection switch off test failed: %d\n",checks);
        notify_completion(false);
    }

    //Finally test both
    in.rise(in_handler);
    in.fall(in_handler);
    flipper();

    if (checks != 20) {
        printf("Simultaneous rising and falling edges failed: %d\n",checks);
        notify_completion(false);
    }

    notify_completion(true);
    return 0;
}
