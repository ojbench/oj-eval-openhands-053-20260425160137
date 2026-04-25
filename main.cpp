#include "src.hpp"
#include <bits/stdc++.h>
using namespace std;
using namespace Grammar;

int main(){
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  string regex;
  if(!(cin>>regex)) return 0;
  RegexChecker checker(regex);
  // Input format not specified; assume then a number T and T strings or until EOF
  vector<string> xs;
  string s;
  if (cin>>s) {
    if (all_of(s.begin(), s.end(), ::isdigit)) {
      int T = stoi(s);
      for(int i=0;i<T;i++){ string t; if(!(cin>>t)) break; xs.push_back(t);}  
    } else {
      xs.push_back(s);
      while (cin>>s) xs.push_back(s);
    }
  }
  for (auto &t: xs) {
    cout << (checker.Check(t)?"YES":"NO") << '\n';
  }
  return 0;
}
