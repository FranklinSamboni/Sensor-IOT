// basic file operations
#include <iostream>
#include <fstream>
using namespace std;

int main () {
  ofstream myfile;
  myfile.open ("example.txt");
  myfile << "adasdasdasdasadle.\n";
  myfile.close();
  return 0;
}
