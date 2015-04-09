#!/bin/bash

JOBNUMBER=0
for OUTPUT in $(find *.dcm)
do
	./main $OUTPUT &
	JOBNUMBER=$[JOBNUMBER +1]
	if (( $JOBNUMBER % 10 == 0)); then wait; fi
done
