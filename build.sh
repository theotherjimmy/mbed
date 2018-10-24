#!/bin/bash

BASE_DIR=$(basename $(pwd))

mbed compile -m FUTURE_SEQUANA_M0_PSA -t GCC_ARM -c --profile debug
cp BUILD/FUTURE_SEQUANA_M0_PSA/GCC_ARM/$BASE_DIR.hex targets/TARGET_Cypress/TARGET_PSOC6/TARGET_CY8C63XX/TARGET_MCU_PSOC6_M4/device/psoc63_m0_psa_1.0.hex
mbed test --compile -m FUTURE_SEQUANA_PSA -t GCC_ARM -n psa-services-secure_add-tests-secure_add-dummy --profile debug -c -DNO_GREENTEA
cp BUILD/tests/FUTURE_SEQUANA_PSA/GCC_ARM/psa/services/secure_add/TESTS/secure_add/dummy/dummy.hex /media/$USER/KitProg2

mbed compile -m FUTURE_SEQUANA_M0_PSA -t GCC_ARM -c --profile debug --source . --source TESTS/spm/smoke/TARGET_PSA_TFM
cp BUILD/FUTURE_SEQUANA_M0_PSA/GCC_ARM/$BASE_DIR.hex targets/TARGET_Cypress/TARGET_PSOC6/TARGET_CY8C63XX/TARGET_MCU_PSOC6_M4/device/psoc63_m0_psa_1.0.hex
mbed test --compile -m FUTURE_SEQUANA_PSA -t GCC_ARM -n tests-spm-smoke --profile debug -c -DNO_GREENTEA
cp BUILD/tests/FUTURE_SEQUANA_PSA/GCC_ARM/TESTS/spm/smoke/smoke.hex /media/$USER/KitProg2

mbed compile -m FUTURE_SEQUANA_M0_PSA -t GCC_ARM -c --profile debug --source . --source TESTS/spm/client_tests/TARGET_PSA_TFM
cp BUILD/FUTURE_SEQUANA_M0_PSA/GCC_ARM/$BASE_DIR.hex targets/TARGET_Cypress/TARGET_PSOC6/TARGET_CY8C63XX/TARGET_MCU_PSOC6_M4/device/psoc63_m0_psa_1.0.hex
mbed test --compile -m FUTURE_SEQUANA_PSA -t GCC_ARM -n tests-spm-client_tests --profile debug -c -DNO_GREENTEA
cp BUILD/tests/FUTURE_SEQUANA_PSA/GCC_ARM/TESTS/spm/client_tests/client_tests.hex /media/$USER/KitProg2

mbed compile -m FUTURE_SEQUANA_M0_PSA -t GCC_ARM -c --profile debug --source . --source TESTS/spm/server/TARGET_PSA_TFM
cp BUILD/FUTURE_SEQUANA_M0_PSA/GCC_ARM/$BASE_DIR.hex targets/TARGET_Cypress/TARGET_PSOC6/TARGET_CY8C63XX/TARGET_MCU_PSOC6_M4/device/psoc63_m0_psa_1.0.hex
mbed test --compile -m FUTURE_SEQUANA_PSA -t GCC_ARM -n tests-spm-server --profile debug -c -DNO_GREENTEA
cp BUILD/tests/FUTURE_SEQUANA_PSA/GCC_ARM/TESTS/spm/server/server.hex /media/$USER/KitProg2
