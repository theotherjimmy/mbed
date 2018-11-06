#!/bin/bash

# Build secure images
mbed compile -m FUTURE_SEQUANA_M0_PSA -t GCC_ARM -c --profile debug -N psoc63_m0_psa
mbed test --compile -m FUTURE_SEQUANA_M0_PSA -t GCC_ARM -c --profile debug -n tests-spm-*

#Build Non-secure images
mbed test --compile -m FUTURE_SEQUANA_PSA -t GCC_ARM -n tests-mbed_hal-spm,tests-psa-*,tests-spm-* --profile debug -c -DNO_GREENTEA

cp BUILD/tests/FUTURE_SEQUANA_PSA/GCC_ARM/TESTS/psa/secure_add/secure_add.hex /media/$USER/KitProg2
cp BUILD/tests/FUTURE_SEQUANA_PSA/GCC_ARM/TESTS/psa/prot-internal-storage/prot-internal-storage.hex /media/$USER/KitProg2
cp BUILD/tests/FUTURE_SEQUANA_PSA/GCC_ARM/TESTS/mbed_hal/spm/spm.hex /media/$USER/KitProg2
cp BUILD/tests/FUTURE_SEQUANA_PSA/GCC_ARM/TESTS/spm/smoke/smoke.hex /media/$USER/KitProg2
cp BUILD/tests/FUTURE_SEQUANA_PSA/GCC_ARM/TESTS/spm/client_tests/client_tests.hex /media/$USER/KitProg2
cp BUILD/tests/FUTURE_SEQUANA_PSA/GCC_ARM/TESTS/spm/server/server.hex /media/$USER/KitProg2



# Or if you prefer to build all of mbed-os tests
# mbed test --compile -m FUTURE_SEQUANA_M0_PSA -t GCC_ARM -c --profile debug
# mbed test --compile -m FUTURE_SEQUANA_PSA -t GCC_ARM --profile debug -c -DNO_GREENTEA
