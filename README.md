# MemoryManager

To buuild do:
  cd MemoryManager
  make
  cd ..
  c++ -std=c++17 -o clt CommandLineTest.cpp -L ./MemoryManager -lMemoryManager
  valgrind --leak-check=yes ./clt      or       ./clt
