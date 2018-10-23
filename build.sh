#!/bin/bash
mbed compile -m FUTURE_SEQUANA_M0_PSA -t GCC_ARM -c --profile debug
cp BUILD/FUTURE_SEQUANA_M0_PSA/GCC_ARM/mbed-os.hex targets/TARGET_Cypress/TARGET_PSOC6/TARGET_CY8C63XX/TARGET_MCU_PSOC6_M4/device/psoc63_m0_psa_1.0.hex
mbed test --compile -m FUTURE_SEQUANA_PSA -t GCC_ARM -n psa-services-secure_add-tests-secure_add-dummy --profile debug -c
cp BUILD/tests/FUTURE_SEQUANA_PSA/GCC_ARM/psa/services/secure_add/TESTS/secure_add/dummy/dummy.hex /media/$USER/KitProg2
