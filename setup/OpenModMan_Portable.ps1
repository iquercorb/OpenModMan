# global variables

$AAP_MAJ =          "1";
$AAP_MIN =          "3";
$AAP_REV =          "3";

# copy required files

Write-Host "Copying x64 binary...";

Copy-Item -Force "$PSScriptRoot\..\bin\64-bit\Release\OpenModMan.exe" -Destination "$PSScriptRoot\OpenModMan_x64\";

Write-Host "Copying x64 dlls...";

Copy-Item -Force "$PSScriptRoot\..\dll\64-bit\libcurl-x64.dll" -Destination "$PSScriptRoot\OpenModMan_x64\";
Copy-Item -Force "$PSScriptRoot\..\dll\64-bit\liblzma.dll" -Destination "$PSScriptRoot\OpenModMan_x64\";
Copy-Item -Force "$PSScriptRoot\..\dll\64-bit\libzstd.dll" -Destination "$PSScriptRoot\OpenModMan_x64\";
Copy-Item -Force "$PSScriptRoot\..\dll\64-bit\zlib1.dll" -Destination "$PSScriptRoot\OpenModMan_x64\";

Write-Host "Copying x86 binary...";

Copy-Item -Force "$PSScriptRoot\..\bin\32-bit\Release\OpenModMan.exe" -Destination "$PSScriptRoot\OpenModMan_x86\";

Write-Host "Copying x86 dlls...";

Copy-Item -Force "$PSScriptRoot\..\dll\32-bit\libcurl.dll" -Destination "$PSScriptRoot\OpenModMan_x86\";
Copy-Item -Force "$PSScriptRoot\..\dll\32-bit\liblzma.dll" -Destination "$PSScriptRoot\OpenModMan_x86\";
Copy-Item -Force "$PSScriptRoot\..\dll\32-bit\libzstd.dll" -Destination "$PSScriptRoot\OpenModMan_x86\";
Copy-Item -Force "$PSScriptRoot\..\dll\32-bit\zlib1.dll" -Destination "$PSScriptRoot\OpenModMan_x86\";

Write-Host "Copying CREDITS and LICENSE files...";

Copy-Item -Force "$PSScriptRoot\..\CREDITS" -Destination "$PSScriptRoot\OpenModMan_x86\";
Copy-Item -Force "$PSScriptRoot\..\CREDITS" -Destination "$PSScriptRoot\OpenModMan_x64\";

Copy-Item -Force "$PSScriptRoot\..\LICENSE" -Destination "$PSScriptRoot\OpenModMan_x86\";
Copy-Item -Force "$PSScriptRoot\..\LICENSE" -Destination "$PSScriptRoot\OpenModMan_x64\";

Write-Host "Making Zip archive for x86...";

$compress_x86 = @{
  Path = "$PSScriptRoot\OpenModMan_x86"
  CompressionLevel = "Optimal"
  DestinationPath = "$PSScriptRoot\OpenModMan_" + $AAP_MAJ + "-" + $AAP_MIN + "-" + $AAP_REV + "-x86_Portable.zip"
}
Compress-Archive -Force @compress_x86

Write-Host "Making Zip archive for x64..."

$compress_x64 = @{
  Path = "$PSScriptRoot\OpenModMan_x64"
  CompressionLevel = "Optimal"
  DestinationPath = "$PSScriptRoot\OpenModMan_" + $AAP_MAJ + "-" + $AAP_MIN + "-" + $AAP_REV + "-x64_Portable.zip"
}
Compress-Archive -Force @compress_x64
