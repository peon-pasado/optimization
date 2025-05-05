#include <bits/stdc++.h>
#define double long double
using namespace std;

const int step = 100'000;//eps = 1 / step
const int maxn = 505;
const int L = 2;
int sz[maxn], inv[maxn];
int n;

//1 -> step
//1/k ~~ ceil(step / k) 
int n_bins_ceil(int k) { // 1 / k
    return (step + k - 1) / k;
}

int n_bins_floor(int k) { // 1 / k
    return step / k;
}

long long area(int height) {
    return 1ll * height * height;
}

double cua(double x) {
    return x * x;
}

//upper bound
double dp_1D[maxn][maxn];
void preprocess() {
    for (int i=0; i<n; ++i) {
        for (int j=i; j<n; ++j) {
            //1 / inv[i] + t * 1 / inv[j] <= 1
            //inv[j] + t * inv[i] <= inv[i] * inv[j]
            //t <= (inv[i] - 1) / inv[i] *  inv[j]
            long long t = (inv[i] - 1) * 1ll* inv[j] / inv[i];
            dp_1D[i][j] = cua((double)step / inv[i]) + t * cua((double)step / inv[j]);
        }
    }
}

double dp_2D[maxn][step + 1];
double dpr_2D[maxn][step + 1];
long long dp() {
    for (int i=0; i<n; ++i)
        for (int j=0; j<=step; ++j) {
            dp_2D[i][j] = 1e15;
            dpr_2D[i][j] = 1e15;
        }
    
    dp_2D[0][sz[0]] = 0; 
    dpr_2D[0][sz[0]] = 0;
    for (int s=sz[0]+1; s<=step; ++s) {
        for (int j=0; j<n; ++j) {
            for (int i=0; i<=j; ++i) {
                auto temp = dp_2D[i][s - sz[j]] + dp_1D[i][j];
                if (dp_2D[j][s] > temp) {
                    dp_2D[j][s] = temp;
                    dpr_2D[j][s] = dpr_2D[i][s - sz[j]] + (double) step / inv[i];
                }
            } 
        }
    }
    double ans = 1e18;
    int idx_final = -1;
    double lans = 0;
    for (int i=0; i<n; ++i) {
        for (int j=i; j<n; ++j) {
            for (int k=step-sz[j]+1; k<=step; ++k) {
                double temp = dp_2D[i][k] + dp_1D[i][j] + (double)step * step / inv[j] / inv[j];
                double ltemp = dpr_2D[i][k] + (double) step / inv[i] + (double) step / inv[j];
                if (ltemp <= step) {
                    //ltemp + i * step / inv[idx_final] <= step
                    //i = floor((step - ltemp) * inv[idx_final] / step) + 1
                    long long ex = floor((step - ltemp) / step * inv[idx_final]) + 1;
                    ltemp += (double)i * step / inv[idx_final];
                    temp += (double)step * step / inv[idx_final];  
                }
                if (ans > temp) {
                    ans = temp;
                    lans = ltemp;
                    idx_final = j;
                }   
            }
        }
    }
    cout << "last idx: " << idx_final << endl;
    return ans;
}


double solve_upper() {
    for (int i=0; i<n; ++i) {
        sz[i] = n_bins_ceil(i + 2);
        inv[i] = i + 2;
    } 
    preprocess();
    return 1.d * (dp() - 0.25 * area(step)) / area(step);
}

int main() {
    cout.precision(8);
    cout << fixed;
    cin>>n;
    cout << solve_upper() << endl;
    return 0;
}