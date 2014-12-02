CC=clang++
CFLAGS=-std=c++11 -Wall 
INC=-I/usr/local/include/opencv -I/usr/local/include -I/usr/local/include/dcmtk/**
LDFLAGS=-L/usr/local/lib -L/usr/local/lib/dcmtk -L/usr/lib -lz -lpthread -ldcmjpeg -lijg8 -lijg12 -lijg16 -ldcmimage -ldcmimgle -ldcmdsig -ldcmsr -ldcmtls -ldcmnet -ldcmdata -li2d -loflog -lofstd -lopencv_calib3d -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videoio -lopencv_videostab -lgsl -lgslcblas 
DFLAGS=-DHAVE_CONFIG_H
SRCS=breast.cpp breastgradient_segmented.cpp mammography.cpp breast_dist.cpp dailyCalibration.cpp phantomCalibration.cpp scanner.cpp breastgradient.cpp main.cpp various.cpp
OBJS= main.o various.o scanner.o phantomCalibration.o mammography.o dailyCalibration.o breast.o
PROG= main
$(PROG): $(OBJS) 
	$(CC) $(CFLAGS) $(DFLAGS) -o $(PROG) $(OBJS) $(LDFLAGS)
.cpp.o:
	$(CC) $(CFLAGS) $(DFLAGS) -c $*.cpp 
clean:
	rm $(OBJS) $(PROG)
depend:
	makedepend -- $(CFLAGS) -- $(SRCS)
# DO NOT DELETE

breast.o: breast.h main_header.h mammography.h various.h phantomCalibration.h
breast.o: scanner.h /usr/include/gsl/gsl_fit.h /usr/include/stdlib.h
breast.o: /usr/include/features.h /usr/include/stdc-predef.h
breast.o: /usr/include/alloca.h /usr/include/gsl/gsl_math.h
breast.o: /usr/include/math.h /usr/include/gsl/gsl_sys.h
breast.o: /usr/include/gsl/gsl_inline.h /usr/include/gsl/gsl_machine.h
breast.o: /usr/include/limits.h /usr/include/gsl/gsl_precision.h
breast.o: /usr/include/gsl/gsl_types.h /usr/include/gsl/gsl_nan.h
breast.o: /usr/include/gsl/gsl_pow_int.h /usr/include/gsl/gsl_minmax.h
breast.o: /usr/include/gsl/gsl_poly.h /usr/include/gsl/gsl_complex.h
breast.o: dailyCalibration.h
breastgradient_segmented.o: /usr/include/stdlib.h /usr/include/features.h
breastgradient_segmented.o: /usr/include/stdc-predef.h /usr/include/alloca.h
mammography.o: mammography.h various.h
dailyCalibration.o: dailyCalibration.h main_header.h mammography.h various.h
dailyCalibration.o: phantomCalibration.h scanner.h /usr/include/gsl/gsl_fit.h
dailyCalibration.o: /usr/include/stdlib.h /usr/include/features.h
dailyCalibration.o: /usr/include/stdc-predef.h /usr/include/alloca.h
dailyCalibration.o: /usr/include/gsl/gsl_math.h /usr/include/math.h
dailyCalibration.o: /usr/include/gsl/gsl_sys.h /usr/include/gsl/gsl_inline.h
dailyCalibration.o: /usr/include/gsl/gsl_machine.h /usr/include/limits.h
dailyCalibration.o: /usr/include/gsl/gsl_precision.h
dailyCalibration.o: /usr/include/gsl/gsl_types.h /usr/include/gsl/gsl_nan.h
dailyCalibration.o: /usr/include/gsl/gsl_pow_int.h
dailyCalibration.o: /usr/include/gsl/gsl_minmax.h /usr/include/gsl/gsl_poly.h
dailyCalibration.o: /usr/include/gsl/gsl_complex.h
phantomCalibration.o: phantomCalibration.h scanner.h
phantomCalibration.o: /usr/include/gsl/gsl_fit.h /usr/include/stdlib.h
phantomCalibration.o: /usr/include/features.h /usr/include/stdc-predef.h
phantomCalibration.o: /usr/include/alloca.h /usr/include/gsl/gsl_math.h
phantomCalibration.o: /usr/include/math.h /usr/include/gsl/gsl_sys.h
phantomCalibration.o: /usr/include/gsl/gsl_inline.h
phantomCalibration.o: /usr/include/gsl/gsl_machine.h /usr/include/limits.h
phantomCalibration.o: /usr/include/gsl/gsl_precision.h
phantomCalibration.o: /usr/include/gsl/gsl_types.h /usr/include/gsl/gsl_nan.h
phantomCalibration.o: /usr/include/gsl/gsl_pow_int.h
phantomCalibration.o: /usr/include/gsl/gsl_minmax.h
phantomCalibration.o: /usr/include/gsl/gsl_poly.h
phantomCalibration.o: /usr/include/gsl/gsl_complex.h dailyCalibration.h
phantomCalibration.o: main_header.h mammography.h various.h
scanner.o: scanner.h /usr/include/gsl/gsl_fit.h /usr/include/stdlib.h
scanner.o: /usr/include/features.h /usr/include/stdc-predef.h
scanner.o: /usr/include/alloca.h /usr/include/gsl/gsl_math.h
scanner.o: /usr/include/math.h /usr/include/gsl/gsl_sys.h
scanner.o: /usr/include/gsl/gsl_inline.h /usr/include/gsl/gsl_machine.h
scanner.o: /usr/include/limits.h /usr/include/gsl/gsl_precision.h
scanner.o: /usr/include/gsl/gsl_types.h /usr/include/gsl/gsl_nan.h
scanner.o: /usr/include/gsl/gsl_pow_int.h /usr/include/gsl/gsl_minmax.h
scanner.o: /usr/include/gsl/gsl_poly.h /usr/include/gsl/gsl_complex.h
scanner.o: phantomCalibration.h various.h dailyCalibration.h main_header.h
scanner.o: mammography.h
breastgradient.o: /usr/include/stdlib.h /usr/include/features.h
breastgradient.o: /usr/include/stdc-predef.h /usr/include/alloca.h
main.o: main_header.h mammography.h various.h phantomCalibration.h scanner.h
main.o: /usr/include/gsl/gsl_fit.h /usr/include/stdlib.h
main.o: /usr/include/features.h /usr/include/stdc-predef.h
main.o: /usr/include/alloca.h /usr/include/gsl/gsl_math.h /usr/include/math.h
main.o: /usr/include/gsl/gsl_sys.h /usr/include/gsl/gsl_inline.h
main.o: /usr/include/gsl/gsl_machine.h /usr/include/limits.h
main.o: /usr/include/gsl/gsl_precision.h /usr/include/gsl/gsl_types.h
main.o: /usr/include/gsl/gsl_nan.h /usr/include/gsl/gsl_pow_int.h
main.o: /usr/include/gsl/gsl_minmax.h /usr/include/gsl/gsl_poly.h
main.o: /usr/include/gsl/gsl_complex.h dailyCalibration.h breast.h
various.o: main_header.h mammography.h various.h phantomCalibration.h
various.o: scanner.h /usr/include/gsl/gsl_fit.h /usr/include/stdlib.h
various.o: /usr/include/features.h /usr/include/stdc-predef.h
various.o: /usr/include/alloca.h /usr/include/gsl/gsl_math.h
various.o: /usr/include/math.h /usr/include/gsl/gsl_sys.h
various.o: /usr/include/gsl/gsl_inline.h /usr/include/gsl/gsl_machine.h
various.o: /usr/include/limits.h /usr/include/gsl/gsl_precision.h
various.o: /usr/include/gsl/gsl_types.h /usr/include/gsl/gsl_nan.h
various.o: /usr/include/gsl/gsl_pow_int.h /usr/include/gsl/gsl_minmax.h
various.o: /usr/include/gsl/gsl_poly.h /usr/include/gsl/gsl_complex.h
various.o: dailyCalibration.h
