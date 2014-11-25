CC=clang++
CFLAGS=-std=c++11 -Wall 
INC=-I/usr/local/include/opencv -I/usr/local/include -I/usr/local/include/dcmtk/**
LDFLAGS=-L/usr/local/lib -L/usr/local/lib/dcmtk -L/usr/lib -lz -lpthread -ldcmjpeg -lijg8 -lijg12 -lijg16 -ldcmimage -ldcmimgle -ldcmdsig -ldcmsr -ldcmtls -ldcmnet -ldcmdata -li2d -loflog -lofstd -lopencv_calib3d -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videoio -lopencv_videostab -lgsl -lgslcblas 
DFLAGS=-DHAVE_CONFIG_H
SRCS=breast.cpp breastgradient_segmented.cpp mammography.cpp breast_dist.cpp dailyCalibration.cpp PhantomCalibration.cpp scanner.cpp breastgradient.cpp main.cpp various.cpp
OBJS= main.o various.o scanner.o PhantomCalibration.o mammography.o dailyCalibration.o breast.o
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

breast.o: /usr/include/math.h /usr/include/features.h
breast.o: /usr/include/stdc-predef.h main_header.h
breastgradient_segmented.o: /usr/include/stdlib.h /usr/include/features.h
breastgradient_segmented.o: /usr/include/stdc-predef.h /usr/include/alloca.h
mammography.o: main_header.h
dailyCalibration.o: main_header.h /usr/include/math.h /usr/include/features.h
dailyCalibration.o: /usr/include/stdc-predef.h
PhantomCalibration.o: main_header.h
scanner.o: /usr/include/math.h /usr/include/features.h
scanner.o: /usr/include/stdc-predef.h main_header.h
scanner.o: /usr/include/gsl/gsl_fit.h /usr/include/stdlib.h
scanner.o: /usr/include/alloca.h /usr/include/gsl/gsl_math.h
scanner.o: /usr/include/gsl/gsl_sys.h /usr/include/gsl/gsl_inline.h
scanner.o: /usr/include/gsl/gsl_machine.h /usr/include/limits.h
scanner.o: /usr/include/gsl/gsl_precision.h /usr/include/gsl/gsl_types.h
scanner.o: /usr/include/gsl/gsl_nan.h /usr/include/gsl/gsl_pow_int.h
scanner.o: /usr/include/gsl/gsl_minmax.h /usr/include/gsl/gsl_poly.h
scanner.o: /usr/include/gsl/gsl_complex.h
breastgradient.o: /usr/include/stdlib.h /usr/include/features.h
breastgradient.o: /usr/include/stdc-predef.h /usr/include/alloca.h
main.o: main_header.h
various.o: main_header.h
