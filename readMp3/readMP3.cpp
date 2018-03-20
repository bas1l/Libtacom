#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main()
{
	string line;
	//ifstream myfile ("m.mp3");
        ifstream myfile ("../Libtacom/readMp3/m.mp3", ios::in | ios::binary);

	if (myfile.is_open()) {
            while (! myfile.eof() ) {
              getline (myfile,line);
              cout << line << endl;
            }
            
            myfile.close();
            cout << "Yay!\n";
	}
	else {
            cout << "Erorr!\n";
        }
        
	return 0;
}