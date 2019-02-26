#!/bin/bash

LOG_FILE=$( mktemp )

function pio-target() {
    local PROJECT=$1

    shift
    platformio run -d ${PROJECT} $@ 2>&1 > ${LOG_FILE}
    local EXIT_CODE=$?

    echo -e "\n\t\e[32m\e[1m${PROJECT}\e[0m" | tr ./ ' '
    if grep -q SUMMARY ${LOG_FILE}; then
        cat ${LOG_FILE} | sed -n -e '/SUMMARY/,$p'
    else
        cat ${LOG_FILE} | sed -n -e '/Memory Usage/,$p'
    fi

    return ${EXIT_CODE}
}

function fail() {
    cat ${LOG_FILE}
    exit 1
}

for PIO_PROJECT in ./*/platformio.ini; do
    PROJECT=${PIO_PROJECT%*/*}

    pio-target ${PROJECT} || fail
    test -d ${PROJECT}/resources && \
    {
        pio-target ${PROJECT} -t buildfs || fail;
    }

done

exit