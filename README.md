# Handle sound file (wav file)

### Step 1: Open terminal and compile file signal.cpp:
gcc -g signal.cpp -o signal -Llibsndfile-master -lsndfile -lstdc++ -lws2_32

### Step 2: Run file signal.exe to write wav file data to file data.txt

### Step 3: Compile and run file plot.cpp to plot file data.txt
