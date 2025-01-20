# MemoryManager

To buuild do:
  
  cd MemoryManager
  
  make
  
  cd ..
 
  c++ -std=c++17 -o clt CommandLineTest.cpp -L ./MemoryManager -lMemoryManager
 
  valgrind --leak-check=yes ./clt      or       ./clt


explanation of code:
https://youtu.be/2Zp3JVMw57M?si=3UEPRCRZuhdtJyEr
