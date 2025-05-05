#include <bits/stdc++.h>
#include "testlib.h"
using namespace std;

int main(int argc, char* argv[]) {
    registerGen(argc, argv, 1);
    int n = opt<int>("n");
    vector<int> p(n), h(n), w(n);
    int T = 0;
    for (int i=0; i<n; ++i) {
        p[i] = rnd.next(1, 10);
        h[i] = rnd.next(1, 10);
        w[i] = rnd.next(1, 10);
        T += p[i];
    }
    int rdd_id = opt<int>("rdd");
    int lf_id = opt<int>("lf");
    float rdd[] = {0.2, 0.4, 0.6, 0.8, 1.0};
    float lf[] = {0.2, 0.4};
    float RDD = rdd[rdd_id];
    float _LF = lf[lf_id];
    int L = ceil((1 - _LF - 0.5 * RDD) * T);
    int R = floor((1 - _LF + 0.5 * RDD) * T);
    assert(L <= R);
    vector<int> d(n);
    for (int i=0; i<n; ++i) {
        d[i] = rnd.next(L, R);
    }
    cout << n << '\n';
    for (int i=0; i<n; ++i) {
        cout<<p[i]<<' '<<h[i]<<' '<<w[i]<<' '<<d[i]<<'\n';    
    }
    return 0;
}