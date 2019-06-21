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
int mini, minim;
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
for (int j=elementNumberStart; j<elementNumberEnd; j++) 
        {
        
            int checkrow = (minu((m+1), line)) - j - 1;
            int checkcol = start_col + j;
            int local_val= 0;
            if (checkrow==0)
            local_val = checkcol;
            
            else if (checkcol==0)
            local_val = checkrow;
            
            else if (str1[checkrow-1] == str2[checkcol-1]) 
            local_val = dp[checkrow-1][checkcol-1];
            
            else
            local_val = 1 + min(dp[checkrow][checkcol-1],  // Insert 
                               dp[checkrow-1][checkcol],  // Remove 
                               dp[checkrow-1][checkcol-1]); // Replace
            dp[checkrow][checkcol] = local_val;
            //int yy = editDist(str1, str2, checkrow, checkcol);
            //dp[checkrow][checkcol] = yy;
        }
}

int editDistDP(string str1, string str2, int m, int n) 
{ 

    
    for (int line=1; line<(m + n + 2); line++) 
    { 
        
        int start_col =  max(0, line-(m+1));
        int count = min(line, ((n+1)-start_col), (m+1)); 
        //printf("line is %d %d\n", line, count);
        int num_threads = cpus, eStart = 0, eEnd = 0, work_pt = 0, tempS = 0, nt = 0, rem = 0;
        
        if (count >= minim) {
        //printf("count more %d %d\n", count, num_threads);
        if (num_threads <= count) {
            if ((count / num_threads) >= mini) {
                nt = num_threads;
                work_pt = count / num_threads;
                rem = count % num_threads;
            } else if ((count / num_threads) < mini) {
                nt = count / mini;
                work_pt = mini;
                rem = count % mini;
            }
        //printf("%d %d %d %d\n", count, nt, work_pt, rem);
        }
        else if (num_threads > count) {
        nt = count / mini;
        work_pt = mini;
        rem = count % mini;
        
        }
        struct thread_work_t tw[nt];
        pthread_t thread[nt];
        //printf("%d %d %d\n", nt, work_pt, rem);
        for (int i = 0; i < nt; i++) {
        
        if (i != (nt - 1)) {
        eStart = tempS;
        eEnd = eStart + work_pt;
        }
        else {
        eStart = eEnd;
        eEnd = eStart + work_pt + rem;
        }
        tw[i].elementNumberS = eStart;
        tw[i].elementNumberE = eEnd;
        tw[i].lineN = line;
        tw[i].mn = m;
        tw[i].startcolumn = start_col;
        tw[i].str1 = str1;
        tw[i].str2 = str2;
        tempS = eEnd;
        pthread_create(&thread[i], NULL, doWork, (void*)&tw[i]);        
        }
        for (int i=0; i < nt; i++) {
        pthread_join(thread[i], NULL);
        }
        }       
        else {
        //printf("count less\n");
        struct thread_work_t twm;
        twm.elementNumberS = 0;
        twm.elementNumberE = count;
        twm.lineN = line;
        twm.mn = m;
        twm.startcolumn = start_col;
        twm.str1 = str1;
        twm.str2 = str2;        
        doWork((void*)&twm);
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
        //cpus = 2;
        minim = minu(s.length(), t.length());
        mini = minim;
        
    std::cout
        << editDistDP(s, t, s.length(), t.length())
        << std::endl ;
    return 0;         
}

