#!/bin/bash

for OUTPUT in $(find *.dcm)
do
	./main $OUTPUT &
done
