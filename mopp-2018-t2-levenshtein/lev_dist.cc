// A Dynamic Programming based C++ program to find minimum 
// number operations to convert str1 to str2 
#include<bits/stdc++.h> 
#include <iostream>
#include <sstream>
#include <assert.h>
#include <stdlib.h>
#include "SafeQueue.h"
#include "ThreadPool.h"
#include <stdio.h>
#include <pthread.h>
#include <sys/sysinfo.h>

using namespace std; 

    
struct thread_work_t{
    int elementNumberS;
    int elementNumberE;
    int lineN;
    int mn;
    int startcolumn;
    std::string str1 ;
    std::string str2 ;
    
};

int **dp;

// Utility function to find the minimum of three numbers 
int minu(int a, int b) 
{ 
    return (a < b)? a: b;     
} 

int max(int a, int b) 
{ 
    return (a > b)? a: b; 
} 

int min(int x, int y, int z) 
{ 
    return minu(minu(x, y), z); 
}


int editDist(string str1 , string str2 , int m ,int n) 
{ 
    // If first string is empty, the only option is to 
    // insert all characters of second string into first 
    if (m == 0) return n; 
  
    // If second string is empty, the only option is to 
    // remove all characters of first string 
    if (n == 0) return m; 
  
    // If last characters of two strings are same, nothing 
    // much to do. Ignore last characters and get count for 
    // remaining strings. 
    if (str1[m-1] == str2[n-1]) 
        return editDist(str1, str2, m-1, n-1); 
  
    // If last characters are not same, consider all three 
    // operations on last character of first string, recursively 
    // compute minimum cost for all three operations and take 
    // minimum of three values. 
    return 1 + min ( editDist(str1,  str2, m, n-1),    // Insert 
                     editDist(str1,  str2, m-1, n),   // Remove 
                     editDist(str1,  str2, m-1, n-1) // Replace 
                   ); 
} 

void *doWork(void *thread_work_uncasted)
{
    //printf("Entering doWork");
    struct thread_work_t *thread_work = (struct thread_work_t*)thread_work_uncasted;
    int elementNumberStart = thread_work->elementNumberS;
    int elementNumberEnd = thread_work->elementNumberE;
    int line = thread_work->lineN;
    int m = thread_work->mn;
    int start_col = thread_work->startcolumn;
    std::string str1;
    std::string str2;
    str1 = thread_work->str1;
    
    str2 = thread_work->str2;

            int checkrow = (minu((m+1), line)) - elementNumberStart - 1;
            
            int checkcol = start_col + elementNumberStart;
            
            dp[checkrow][checkcol] = editDist(str1, str2, checkrow, checkrow);

}

int editDistDP(string str1, string str2, int m, int n) 
{ 

        int cpus = get_nprocs();
        // nprocs() might return wrong amount inside of a container.
        // Use MAX_CPUS instead, if available.
        if (getenv("MAX_CPUS")) {
        cpus = atoi(getenv("MAX_CPUS"));
        }
    // Sanity-check
        assert(cpus > 0 && cpus <= 64);
        //fprintf(stderr, "Running on %d CPUs\n", cpus);
        ThreadPool pool(cpus);
        pool.init();
    for (int line=1; line<(m + n + 2); line++) 
    { 

        int start_col =  max(0, line-(m+1)); 

        int count = min(line, ((n+1)-start_col), (m+1)); 
        //printf("Value of count %d\n", count);
        
        pthread_t thread[count];
        struct thread_work_t tw[count];
        for (int i=0; i < count; i++) 
        {
        tw[i].elementNumberS = i;
        tw[i].elementNumberE = -999;
        tw[i].lineN = line;
        tw[i].mn = m;
        tw[i].startcolumn = start_col;
        tw[i].str1 = str1;
        tw[i].str2 = str2;
        //fprintf(stderr, "Starting thread %d from %d to %d\n", i, tw[i].elementNumberS, tw[i].lineN);
        //printf("Data in struct: %d, %d, %d, %d, %d \n", tw[i].elementNumberS, tw[i].elementNumberE, tw[i].lineN, tw[i].mn, tw[i].startcolumn);
        auto future = pool.submit(doWork, (void*)&tw[i]);
        future.get();
        //pthread_exit(NULL);
        }
  
        
    }
    
    pool.shutdown();
    
    return dp[m][n]; 
} 

int main (int argc, char const* argv [])
{
    std::string s ;
    std::string t ;
    std::getline (std::cin, s) ;
    std::getline (std::cin, t) ;
    dp = new int*[s.length()+1];
    for(int i = 0; i < (s.length()+1); ++i)
    dp[i] = new int[t.length()];
    std::cout
        << editDistDP(s, t, s.length(), t.length())
        << std::endl ;
    return 0;         
}

