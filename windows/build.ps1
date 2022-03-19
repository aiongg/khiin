Write-Output "Building x86-Release"
MSBuild.exe -m .\KhiinWindows.sln -p:configuration=Release -p:platform=x86 -t:Clean > .\out\build\msbuild_x86.log
MSBuild.exe -m .\KhiinWindows.sln -p:configuration=Release -p:platform=x86 > .\out\build\msbuild_x86.log
Write-Output "Building x64-Release"
MSBuild.exe -m .\KhiinWindows.sln -p:configuration=Release -p:platform=x64 -t:Clean > .\out\build\msbuild_x64.log
MSBuild.exe -m .\KhiinWindows.sln -p:configuration=Release -p:platform=x64 > .\out\build\msbuild_x64.log
