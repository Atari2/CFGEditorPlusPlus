call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake.exe -S . -B build -GNinja "-DCMAKE_BUILD_TYPE:String=Release" "-DQT_QMAKE_EXECUTABLE:STRING=C:/Qt/6.4.0/msvc2019_64/bin/qmake.exe" "-DCMAKE_PREFIX_PATH:STRING=C:/Qt/6.4.0/msvc2019_64" "-DCMAKE_C_COMPILER:STRING=cl.exe" "-DCMAKE_CXX_COMPILER:STRING=cl.exe"
cd build
cmake.exe --build . --target all
cd ..
exit