#!/usr/bin/fish

# Use this with `watchexec -w . -f cpp -- "fish run-bmtests.fish"` to
# auto retest when you update or add a test

mbed test --compile -t gcc_arm -m nucleo_f411re -n "*bare-metal*" --app-config bm-app.json --silent
mbed test --run -t gcc_arm -m nucleo_f411re -n "*bare-metal*"
