#include <bits/stdc++.h>
#define double long double
using namespace std;

const int maxn = 5010;
double f[maxn][maxn];
double w[maxn], c[maxn]; 
const double eps = 1e-10;
int m;

/**
 * 
 * 
 * max sum_{l < i <= r} ci * [M - (1 / i) * (1 / i)]
 * 
 *     sum_{l < i <= r} ci * (1 / i) <= 1 - 1/l
 * 
 *  
 * 
 * 
 * 
 * 
 * 
 */


vector<int> sol, seq;
int i, j;
void branch_and_bounds(int pos, double C, double partial_cst, double& lb) {
	if (lb < partial_cst - floor(C*j) / j / j) {
        lb = partial_cst - floor(C*j) / j / j;
        sol = seq;
        int r = floor(C * j);
        for (int i=0; i<r; ++i) sol.push_back(0);
    }
	if (pos == m) return;
	if (C + eps < w[pos]) return branch_and_bounds(pos+1, C, partial_cst, lb);
	int s = -1;
    double sum = 0;
	double val = partial_cst;
    auto seq2 = seq;
	for (int i = pos; i < m; ++i) {
        seq2.push_back(i);
		sum += w[i];
		if (sum > C) {
			s = i;
			break;
		}
		val += c[i];
	}
    /** 
	if (s == -1) {
		if (lb < val) {
            lb = val;
            sol = seq2;
        }
		return;
	}
    **/
    seq2.pop_back();
	sum -= w[s];
	double cb = C - sum;
	double upper_bound = val + cb * c[s] / w[s];
    double lower_bound = val;
 
	//if (s + 1 != m) {
	//	double u0 = val + cb * c[s+1] / w[s+1];
	//	double u1 = u0; //val + c[s] - (w[s] - cb) * c[s-1] / w[s-1];
	//	upper_bound = max(u0, u1);
	//}
	
	//if (lower_bound > lb) {
    //    lb = lower_bound;
    //    sol = seq2;
    //}
	//if (abs(lower_bound - upper_bound) < eps) return;
	//if (upper_bound > lb) {
        seq.push_back(pos);
		branch_and_bounds(pos + 1, C - w[pos], partial_cst + c[pos], lb);
        seq.pop_back();
	//}
    //while (pos + 1 < m && abs(w[pos] - w[pos+1]) < eps) pos++; 
	branch_and_bounds(pos + 1, C, partial_cst, lb);
}


double calc1(int i, int j) {
    ::i = i;
    ::j = j;
    double C = (double)(i - 1) / i;
    m = 0;
    for (int t=i; t<=j; ++t) {
        int r = (i - 1) * t / i;
        for (int k=0; k<r; ++k) {
            w[m + k] = 1. / t;
            c[m + k] = -1. / t / t;
        }
        m += r;
    }
    reverse(w, w+m);
    reverse(c, c+m);
    /** 
    vector<pair<double, double> > ss;
    for (int i=0; i<m; ++i) {
        ss.emplace_back(c[i], w[i]);
    }
    sort(ss.begin(), ss.end(), [&](pair<double, double> p, pair<double, double> q) {
        return p.first * q.second > p.second * q.first;
    });
    for (int i=0; i<m; ++i) {
        c[i] = ss[i].first;
        w[i] = ss[i].second;
    }
    **/

    double lb = -1e9;
    seq.clear();
    seq.push_back(m-1);    
    branch_and_bounds(0, C, 0., lb); 
    seq.pop_back();
    //cout << i << ", " << j << ": ";
    bool ok = 0;
    for (auto v : sol) {
        int u = round(1. / w[v]);
        if (u != i && u != j) {
            ok = 1;
            break;
        }
        //cout << u << ' ';
    }
    if (i == 2 && j == 8) {
        cout << m << endl;
        cout << i << ", " << j << ": ";
        for (auto v : sol) {
            int u = round(1. / w[v]);
            cout << u << ' ';
        }
        cout << '\n';
    }

    //cout << '\n';
    return 1./i/i - lb; 
}


int main() {
    cout.precision(10);
    cout << fixed;
    int n;
    cin>>n; //using bins 1/2, 1/3, ..., 1/n
    assert(n >= 2);
    for (int i=2; i<=n; ++i)
        for (int j=i; j<=n; ++j) {
            f[i][j] = calc1(i, j);
            //cout << f[i][j] << " \n"[j==n];
        }    
    cout << f[2][8] << endl; 
    return 0;
}