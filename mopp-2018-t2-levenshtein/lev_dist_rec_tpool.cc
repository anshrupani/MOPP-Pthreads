// A Naive recursive C++ program to find minimum number 
// operations to convert str1 to str2 
#include<bits/stdc++.h>
#include <vector>
#include <future>
#include <pthread.h>
#include "SafeQueue.h"
#include "ThreadPool.h" 
//#include "threadpool.h"
using namespace std; 

//vector<future<int>> future1;
//vector<future<int>> future2;
//vector<future<int>> future3;

struct thread_work_t{
    std::string str1 ;
    std::string str2 ;
    int m;
    int n;
};

struct thread_work_t tw, tw1, tw2, tw3, tw_a, tw_b, tw_c;

ThreadPool pool(100);
// Utility function to find minimum of three numbers 
int min(int x, int y, int z) 
{ 
    return min(min(x, y), z); 
} 

int a, b, c, minimum;
//int **dp;

int editDistance(void *thread_work_uncasted) 
{ 

    struct thread_work_t *thread_work = (struct thread_work_t*)thread_work_uncasted;
    int m = thread_work->m;
    int n = thread_work->n;
    std::string str1;
    std::string str2;
    str1 = thread_work->str1;
    str2 = thread_work->str2;
    
    
    //create structs
        tw_a.m = m;
        tw_a.n = n-1;
        tw_a.str1 = str1;
        tw_a.str2 = str2;   
        
        tw_b.m = m-1;
        tw_b.n = n;
        tw_b.str1 = str1;
        tw_b.str2 = str2;
        
        tw_c.m = m-1;
        tw_c.n = n-1;
        tw_c.str1 = str1;
        tw_c.str2 = str2;
    
    // If first string is empty, the only option is to 
    // insert all characters of second string into first 
    if (m == 0) {
        return n;
    }

    // If second string is empty, the only option is to 
    // remove all characters of first string 
    if (n == 0) {
        return m; 
    }

    // If last characters of two strings are same, nothing 
    // much to do. Ignore last characters and get count for 
    // remaining strings. 
    if (str1[m-1] == str2[n-1]) {
        //return editDistance(str1, str2, m-1, n-1); 
        return editDistance(tw_c); 
    }


    return 1 + min ( editDistance(tw_a),    // Insert 
            editDistance(tw_b),   // Remove 
            editDistance(tw_c) // Replace 
            ); 
            
    /*return 1 + min ( editDistance(str1,  str2, m, n-1),    // Insert 
            editDistance(str1,  str2, m-1, n),   // Remove 
            editDistance(str1,  str2, m-1, n-1) // Replace 
            );*/
}

int editDist(string str1 , string str2 , int m ,int n, int**dp) 
{ 
    // If first string is empty, the only option is to 
    // insert all characters of second string into first 
    if (m == 0) {
        dp[m][n] = n;
        return dp[m][n]; 
    }

    // If second string is empty, the only option is to 
    // remove all characters of first string 
    if (n == 0) {
        dp[m][n] = m;
        return dp[m][n]; 
    }

    // If last characters of two strings are same, nothing 
    // much to do. Ignore last characters and get count for 
    // remaining strings. 
    if (str1[m-1] == str2[n-1]) {
        tw.m = m-1;
        tw.n = n-1;
        tw.str1 = str1;
        tw.str2 = str2;
        auto future = pool.submit(editDistance, (void*)&tw));
        dp[m][n] = future.get();
        //dp[m][n] = editDistance(str1, str2, m-1, n-1);
        return dp[m][n]; 
    }

    // If last characters are not same, consider all three 
    // operations on last character of first string, recursively 
    // compute minimum cost for all three operations and take 
    // minimum of three values. 

    if (dp[m][n-1] != -1)
        a = dp[m][n-1];
    else {
        tw1.m = m;
        tw1.n = n-1;
        tw1.str1 = str1;
        tw1.str2 = str2;
        auto future1 = pool.submit(editDistance, (void*)&tw1));
        a = future1.get();
        //dp[m][n] = future1.get();
        //a = editDistance(str1, str2, m, n-1);
        }

    if(dp[m-1][n] != -1)
        b = dp[m-1][n];
    else {
        tw2.m = m-1;
        tw2.n = n;
        tw2.str1 = str1;
        tw2.str2 = str2;
        auto future2 = pool.submit(editDistance, (void*)&tw2));
        b = future2.get();
        }
    //else
     //   b = editDistance(str1, str2, m-1, n);

    if (dp[m-1][n-1] != -1)
        c = dp[m-1][n-1];
    else {
        tw3.m = m-1;
        tw3.n = n-1;
        tw3.str1 = str1;
        tw3.str2 = str2;
        auto future3 = pool.submit(editDistance, (void*)&tw1));
        c = future3.get();
        //dp[m][n] = future1.get();
        //a = editDistance(str1, str2, m, n-1);
        }
    //else
       //c = editDistance(str1, str2, m-1, n-1);

    minimum = min (a, b, c);

    return 1 + minimum;

    /*return 1 + min ( editDist(str1,  str2, m, n-1),    // Insert 
      editDist(str1,  str2, m-1, n),   // Remove 
      editDist(str1,  str2, m-1, n-1) // Replace 
      ); */
} 

// Driver program 
int main() 
{ 
    // your code goes here 
    int **dp;
    string str1 = "chicken"; 
    string str2 = "kitchen"; 
    dp= new int*[100];

    for (int i = 0; i < 100; i++)
        dp[i]=new int[100];

    for (int i = 0; i < str1.length(); i++)
    {
        for (int j = 0; j < str2.length(); j++)
            dp[i][j] = -1;
    }
    pool.init();
    cout << editDist( str1 , str2 , str1.length(), str2.length(),dp); 
    pool.shutdown();
    return 0; 
} 
