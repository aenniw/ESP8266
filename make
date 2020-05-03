#!/bin/bash

for PROJECT in ./*/platformio.ini; do
    platformio run --project-dir ${PROJECT%/platformio.ini} || \
        { echo "Project: ${PROJECT} compilation failed"; exit 1; }
    
    if [[ -d ${PROJECT%/platformio.ini}/resources ]]; then
        platformio run -t "buildfs" --project-dir ${PROJECT%/platformio.ini} || \
            { echo "Project: ${PROJECT} ffs failed"; exit 2; }
    fi
done