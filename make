#!/bin/bash

EXIT_CODE=0
LOG_FILE=/tmp/platfomio.log
LOG_FILE_FS=/tmp/platfomio-fs.log

for PROJECT in ./*/platformio.ini; do
    cd ${PROJECT%*/*}
    echo -e "\n\t\e[32m\e[1m${PROJECT%*/*}\e[0m" | tr ./ ' '

    platformio run 2>&1 > ${LOG_FILE}
    EXIT_CODE=$?

	test -d resources && \
	{ 
		pio run --target buildfs 2>&1 > ${LOG_FILE_FS};
		EXIT_CODE=$(( ${EXIT_CODE} + $? ));
	}

    cd ../

    for LOG in ${LOG_FILE} ${LOG_FILE_FS}; do
    	test -f ${LOG} || continue

		if [[ ! ${EXIT_CODE} -eq 0 ]]; then
	        cat ${LOG}
	        break
	    else
	        if grep -q SUMMARY ${LOG}; then
	            cat ${LOG} | sed -n -e '/SUMMARY/,$p'
	        else
	            cat ${LOG} | sed -n -e '/Memory Usage/,$p'
	        fi
	    fi
    done
done

exit ${EXIT_CODE}