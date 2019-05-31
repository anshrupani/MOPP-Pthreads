// A Dynamic Programming based C++ program to find minimum 
// number operations to convert str1 to str2 
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
    //std::cout <<str1<<" "<<str2<<"\n";
   // int **dp = thread_work->dp;
    
// j becomes element number
if (elementNumberEnd == -999)
{

            //printf("Entering if in doWork");
            int checkrow = (minu((m+1), line)) - elementNumberStart - 1;
            
            int checkcol = start_col + elementNumberStart;
            dp[checkrow][checkcol] = checkcol;
            //printf("checkrow and checkcol are %d and %d\n", checkrow, checkcol);
                  
            if (checkrow==0){
            //printf("Going to accessed dp");
            dp[checkrow][checkcol] = checkcol;
            //printf("Accessed dp");
            }
            else if (checkcol==0){
            //printf("Going to accessed dp");
            dp[checkrow][checkcol] = checkrow;
            //printf("Accessed dp");
            }
            
            else if (str1[checkrow-1] == str2[checkcol-1]){ 
            //printf("Going to accessed dp");
            dp[checkrow][checkcol] = dp[checkrow-1][checkcol-1];
            //printf("Accessed dp");
            }
            
            else {
            //printf("Going to accessed dp");
            dp[checkrow][checkcol] = 1 + min(dp[checkrow][checkcol-1],  // Insert 
                               dp[checkrow-1][checkcol],  // Remove 
                               dp[checkrow-1][checkcol-1]); // Replace
            //printf("Accessed dp");
            }
            //printf("Value from doWork %d\n", dp[checkrow][checkcol]);
            //printf("%5d ", dp[minu(m+1, line)-elementNumberStart-1][start_col+elementNumberStart]);
            //pthread_exit(NULL);
}
else
{
//printf("Entering else in doWork");
for (int j=elementNumberStart; j<elementNumberEnd; j++) 
        {
        
            int checkrow = (minu((m+1), line)) - j - 1;
            int checkcol = start_col + j;
                   
            if (checkrow==0)
            dp[checkrow][checkcol] = checkcol;
            
            else if (checkcol==0)
            dp[checkrow][checkcol] = checkrow;
            
            else if (str1[checkrow-1] == str2[checkcol-1]) 
            dp[checkrow][checkcol] = dp[checkrow-1][checkcol-1];
            
            else
            dp[checkrow][checkcol] = 1 + min(dp[checkrow][checkcol-1],  // Insert 
                               dp[checkrow-1][checkcol],  // Remove 
                               dp[checkrow-1][checkcol-1]); // Replace
            //printf("Value from doWork %d\n", dp[checkrow][checkcol]);
            //printf("%5d ", dp[minu(m+1, line)-j-1][start_col+j]); 
            //pthread_exit(NULL);
        }
}
//pthread_exit(NULL);
}

int editDistDP(string str1, string str2, int m, int n) 
{ 

//    int dp[m+1][n+1];
    //int** dp = new int*[n+1];
    //for(int k = 0; k < n+1; ++k)
    //dp[k] = new int[m+1];
    // There will be ROW+COL-1 lines in the output 
    for (int line=1; line<(m + n + 2); line++) 
    { 
        /* Get column index of the first element in this line of output. 
           The index is 0 for first ROW lines and line - ROW for remaining 
           lines  */
        int start_col =  max(0, line-(m+1)); 
  
        /* Get count of elements in this line. The count of elements is 
           equal to minimum of line number, COL-start_col and ROW */
        int count = min(line, ((n+1)-start_col), (m+1)); 
        //printf("Value of count %d\n", count);
            int cpus = get_nprocs();
        // nprocs() might return wrong amount inside of a container.
        // Use MAX_CPUS instead, if available.
        if (getenv("MAX_CPUS")) {
        cpus = atoi(getenv("MAX_CPUS"));
        }
    // Sanity-check
        assert(cpus > 0 && cpus <= 64);
        //fprintf(stderr, "Running on %d CPUs\n", cpus);
        int num_threads = 0;
        if (count <= cpus)
        {
        num_threads = count;
        pthread_t thread[num_threads];
        struct thread_work_t tw[num_threads];
        //printf("num_threads = %d\n", num_threads);
        for (int i=0; i < num_threads; i++) 
        {
        //printf("if for loop 1 element number %d\n", i);
        tw[i].elementNumberS = i;
        tw[i].elementNumberE = -999;
        tw[i].lineN = line;
        tw[i].mn = m;
        tw[i].startcolumn = start_col;
        tw[i].str1 = str1;
        tw[i].str2 = str2;
        //tw[i].dp;
        //fprintf(stderr, "Starting thread %d from %d to %d\n", i, tw[i].elementNumberS, tw[i].lineN);
        //printf("Data in struct: %d, %d, %d, %d, %d \n", tw[i].elementNumberS, tw[i].elementNumberE, tw[i].lineN, tw[i].mn, tw[i].startcolumn);
        //elementNumberStart = i, elementNumberEnd = -999
        
        pthread_create(&thread[i], NULL, doWork, (void*)&tw[i]);
        //printf("Came back in if loop 1");
       // pthread_exit(NULL);
        }        

        for (int i=0; i<num_threads; i++) {
        // wait for all threads
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
        //printf("if for loop 2 element number %d\n", i);
        if (i != (num_threads - 1)) // elementNumberStart = i, elementNumberEnd = -999
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
        pthread_create(&thread[i], NULL, doWork, (void*)&tw[i]);
        //pthread_exit(NULL);
        }
        else //tell pthread that this last one needs to work for all remaining elements, elementNumberStart = i, elementNumberEnd = count-i
        {
        tw[i].elementNumberS = i;
        tw[i].elementNumberE = count;
        tw[i].lineN = line;
        tw[i].mn = m;
        tw[i].startcolumn = start_col;
        tw[i].str1 = str1;
        tw[i].str2 = str2;
        //fprintf(stderr, "Starting thread %d from %d to %d\n", i, tw[i].elementNumberS, tw[i].lineN);
        //printf("Data in struct: %d, %d, %d, %d, %d \n", tw[i].elementNumberS, tw[i].elementNumberE, tw[i].lineN, tw[i].mn, tw[i].startcolumn);
        pthread_create(&thread[i], NULL, doWork, (void*)&tw[i]);
        //pthread_exit(NULL);
        }
        }
        for (int i=0; i<num_threads; i++) {
        // wait for all threads
        pthread_join(thread[i], NULL);
        } 
        }
  
        /* Print elements of this line */
         
        /* Ptint elements of next diagonal on next line */
        //printf("\n"); 
    }
    
    return dp[m][n]; 
} 

/*  
int editDistDP(string str1, string str2, int m, int n) 
{ 
    // Create a table to store results of subproblems 
    int dp[m+1][n+1]; 
  
    // Fill d[][] in bottom up manner 
    for (int i=0; i<=m; i++) 
    { 
        for (int j=0; j<=n; j++) 
        { 
            // If first string is empty, only option is to 
            // insert all characters of second string 
            if (i==0) 
                dp[i][j] = j;  // Min. operations = j 
  
            // If second string is empty, only option is to 
            // remove all characters of second string 
            else if (j==0) 
                dp[i][j] = i; // Min. operations = i 
  
            // If last characters are same, ignore last char 
            // and recur for remaining string 
            else if (str1[i-1] == str2[j-1]) 
                dp[i][j] = dp[i-1][j-1]; 
  
            // If the last character is different, consider all 
            // possibilities and find the minimum 
            else
                dp[i][j] = 1 + min(dp[i][j-1],  // Insert 
                                   dp[i-1][j],  // Remove 
                                   dp[i-1][j-1]); // Replace 
        } 
    } 
  
    return dp[m][n]; 
} 
 */ 
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

