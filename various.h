#include <string>
#include <sstream>

using namespace std;

string intToString(int input){
    stringstream ss;
    ss << input;
    string output = ss.str();
    return output;
}
