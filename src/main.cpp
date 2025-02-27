// main.cpp
// Currently a testing ground
#include <iostream>
#include "dice.h"


using namespace std;

int main () {
    
    cout << "5 D100: " << d_100(5) << endl;
    cout << "5 D20: " << d_20(5) << endl;
    cout << "5 D12: " << d_12(5) << endl;
    cout << "5 D10: " << d_10(5) << endl;
    cout << "5 D8: " << d_8(5) << endl;
    cout << "5 D6: " << d_6(5) << endl;
    cout << "5 D4: " << d_4(5) << endl;


    
    return 0;
}