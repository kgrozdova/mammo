// COMPILE USING "clang++ `Magick++-config --cxxflags --cppflags` breastcheck.cpp `Magick++-config --ldflags --libs` -o breastcheck"

#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <Magick++.h>
#include <ctime>
#include <cmath>
#include <string>

//#define OL_LINE_OUTPUT
#define OL_RESIZE_FACTOR 1
#define OL_EXTENSION_LENGTH 4
#define OL_CHRONO

int main(int argc, char **argv){
	#ifdef OL_CHRONO
	auto startTime = time(NULL);
	#endif
	if(!argv[1]){ // If no command line parameter given, exit the programme.
		std::cerr << "ERROR: No image given. Usage: ./breastcheck [FILENAME]" << std::endl;
		exit(1);
	}
	std::string strFilename = argv[1];
	Magick::Image origMammo(strFilename);			// Try to open the image specified.
	origMammo.resize(Magick::Geometry(origMammo.rows()/OL_RESIZE_FACTOR, origMammo.columns()/OL_RESIZE_FACTOR));
	Magick::Image lineMammo = origMammo;		// Make a copy of it in memory.
	lineMammo.cannyEdge();						// Extract the lines from that image using Canny's algorithm.
	int iLastGap = -1;
	int iLeftness = 0;
	std::vector<std::vector<int>> vecBreastTopBottom;
	std::vector<int> vecBreastEdge;
	vecBreastEdge.resize(lineMammo.rows());
	
	for(int i = 0; i < lineMammo.columns(); i++){	
	// Find the gap between the first and last set of white pixels in each column
	
		int iThisGap;									
		int iFirstJ = -1;
		int iLastJ = -1;
		Magick::Quantum quOld = 0;					
		for(int j = 0; j < lineMammo.rows(); j++){
			Magick::Quantum quNew = lineMammo.pixelColor(i,j).redQuantum(); // red,blue,green all have same value so only check one.
			if(quOld != quNew){
				if(iFirstJ == -1){
					iFirstJ = j;
				} else {
					iLastJ = j;
				}
			}
		}
		vecBreastTopBottom.push_back({iFirstJ,iLastJ});
		// If this gap is bigger than the previous gap, the breast is probably coming from the left.
		if(iLastJ != -1){
			iThisGap = iLastJ - iFirstJ;
			if (iLastGap == -1) iLastGap = iThisGap;
			if (iThisGap < iLastGap){
				iLeftness++;
			}
			else if (iThisGap > iLastGap){
				iLeftness--;
			}
			iLastGap = iThisGap;
		}
	}
	
	std::cout << argv[1] << ": the breast is coming from the ";	// Include filename in output 
	if (iLeftness > 0) std::cout << "left." << std::endl;		// to allow for batch processing.
	else std::cout << "right." << std::endl;
	
	
	#ifdef OL_LINE_OUTPUT
		lineMammo.write("lineMammo.jpg"); // Draw the image after the Canny line transform, for debugging.
	#endif
	
	// Need to account for left facing breast in vecBreastTopBottom, empty fields, and places where there is only one line
	Magick::Image myMammo(Magick::Geometry(lineMammo.columns(),lineMammo.rows()),"white");
	if(iLeftness < 0) lineMammo.flop();
	for(int j = 0; j < myMammo.rows(); j++){
		int iMax = 0;
		bool bNotDone = true;
		while( (iMax < myMammo.columns()) && (bNotDone)){
			bNotDone = (lineMammo.pixelColor(iMax,j).redQuantum() == 0);
			iMax++;
		}
		vecBreastEdge[j] = iMax;
		if(iMax != myMammo.columns()){
			for(int i = 0; i < iMax; i++){
				if ((iMax - i) < std::min((j - vecBreastTopBottom[i][0]),(vecBreastTopBottom[i][1]) - j)){
					myMammo.pixelColor(i,j,Magick::ColorGray(std::pow((double(i) / double(iMax)),4)));
				} else if ((j - vecBreastTopBottom[i][0]) < (vecBreastTopBottom[i][1]) - j){
					myMammo.pixelColor(i,j,Magick::ColorGray(std::pow((double(vecBreastTopBottom[i][0]) / double(j)),4)));
				} else {
					myMammo.pixelColor(i,j,Magick::ColorGray(std::pow((double(j) / double(vecBreastTopBottom[i][1])),4)));
				}
			}
		}
	}
	if(iLeftness < 0) lineMammo.flop();
	if(iLeftness < 0) myMammo.flop();
	myMammo.resize(Magick::Geometry(myMammo.rows()*OL_RESIZE_FACTOR, myMammo.columns()*OL_RESIZE_FACTOR));
	
	
	strFilename.erase(strFilename.length()-OL_EXTENSION_LENGTH,strFilename.length());
	myMammo.write(strFilename+"_depthmap.jpg");
	#ifdef OL_CHRONO
	std::cout << std::endl << "Time taken: " << time(NULL) - startTime << std::endl;
	#endif
	exit(0);
}

