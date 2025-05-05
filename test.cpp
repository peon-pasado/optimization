#include <iostream>
#include <cmath>
using namespace std;

double add_kahan(double a, double b) {
    double y = a + b;
    double t = y - a;   
    double z = b - t;
    return y + z;
}

int main() {
    double f[100];
    f[0] = 1;
    f[1] = (sqrtl(5) - 1) / 2;
    for (int i = 2; i < 100; i++) {
        f[i] = add_kahan(f[i-2], -f[i-1]);
    }
    cout.precision(6);
    cout << fixed;
    for (int i = 0; i < 100; i++) {
        cout << "f[" << i << "] = " << f[i] << ' ' << pow(f[1], i) << endl;
    }
    return 0;
}
