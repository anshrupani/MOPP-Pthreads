#include<bits/stdc++.h> 
#include <iostream>
#include <sstream>
#include <assert.h>
#include <stdlib.h>
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
int cpus;
int editDist(string str1 , string str2 , int m ,int n);
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
	int a, b, c, minimum;
    if (m == 0) {
    return n; 
    }
    if (n == 0) {
    return m; 
    }
    if (str1[m-1] == str2[n-1]) {
    return editDist(str1, str2, m-1, n-1); 
    }
    a = dp[m][n-1];
    b = editDist(str1, str2, m-1, n);  
    c = editDist(str1, str2, m-1, n-1);
    minimum = min (a, b, c);
    
    return 1 + minimum;
} 

void *doWork(void *thread_work_uncasted)
{
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
if (elementNumberEnd == -999)
{
            int checkrow = (minu((m+1), line)) - elementNumberStart - 1;
            
            int checkcol = start_col + elementNumberStart;
            
            int xx = editDist(str1, str2, checkrow, checkcol);
            dp[checkrow][checkcol] = xx;
      
}
else
{
for (int j=elementNumberStart; j<elementNumberEnd; j++) 
        {
        
            int checkrow = (minu((m+1), line)) - j - 1;
            int checkcol = start_col + j;
            
            int yy = editDist(str1, str2, checkrow, checkcol);
            dp[checkrow][checkcol] = yy;
        }
        
}
}

int editDistDP(string str1, string str2, int m, int n) 
{ 

    
    for (int line=1; line<(m + n + 2); line++) 
    { 
        int start_col =  max(0, line-(m+1));
        int count = min(line, ((n+1)-start_col), (m+1)); 
        int num_threads = 0;
        if (count <= cpus)
        {
        num_threads = count;
        pthread_t thread[num_threads];
        struct thread_work_t tw[num_threads];
        for (int i=0; i < num_threads; i++) 
        {
        tw[i].elementNumberS = i;
        tw[i].elementNumberE = -999;
        tw[i].lineN = line;
        tw[i].mn = m;
        tw[i].startcolumn = start_col;
        tw[i].str1 = str1;
        tw[i].str2 = str2;
        
        pthread_create(&thread[i], NULL, doWork, (void*)&tw[i]);
        }        

        for (int i=0; i<num_threads; i++) {
        pthread_join(thread[i], NULL);
        }

        }
        else
        {
        num_threads = cpus;
        pthread_t thread[num_threads];
        struct thread_work_t tw[num_threads];
        for (int i=0; i < num_threads; i++) 
        {
        if (i != (num_threads - 1))
        {
        tw[i].elementNumberS = i;
        tw[i].elementNumberE = -999;
        tw[i].lineN = line;
        tw[i].mn = m;
        tw[i].startcolumn = start_col;
        tw[i].str1 = str1;
        tw[i].str2 = str2;
        pthread_create(&thread[i], NULL, doWork, (void*)&tw[i]);
        }
        else
        {
        tw[i].elementNumberS = i;
        tw[i].elementNumberE = count;
        tw[i].lineN = line;
        tw[i].mn = m;
        tw[i].startcolumn = start_col;
        tw[i].str1 = str1;
        tw[i].str2 = str2;
        pthread_create(&thread[i], NULL, doWork, (void*)&tw[i]);
        }
        }
        for (int i=0; i<num_threads; i++) {
        pthread_join(thread[i], NULL);
        } 
        } 
    }
    
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

    for (int i = 0; i < s.length(); i++)
    {
    for (int j = 0; j < t.length(); j++)
    dp[i][j] = -1;
    }
    
    cpus = get_nprocs();
        // nprocs() might return wrong amount inside of a container.
        // Use MAX_CPUS instead, if available.
        if (getenv("MAX_CPUS")) {
        cpus = atoi(getenv("MAX_CPUS"));
        }
    // Sanity-check
        assert(cpus > 0 && cpus <= 64);
        //cpus = 4;

    std::cout
        << editDistDP(s, t, s.length(), t.length())
        << std::endl ;
    return 0;         
}

