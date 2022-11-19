cmake.exe -S . -B build -GNinja "-DCMAKE_BUILD_TYPE:String=Release" "-DQT_QMAKE_EXECUTABLE:STRING=C:/Qt/6.4.0/msvc2019_64/bin/qmake.exe" "-DCMAKE_PREFIX_PATH:STRING=C:/Qt/6.4.0/msvc2019_64"
cd build
cmake.exe --build . --target all
cd ..
exit