$productName = "Khiin PJH" 
# this should basically match against your previous
# installation path. Make sure that you don't mess with other components used 
# by any other MSI package

$components = Get-ChildItem -Path HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Installer\UserData\S-1-5-18\Components\
$count = 0

foreach ($c in $components) 
{
    foreach($p in $c.Property)
    {
        $propValue = (Get-ItemProperty "Registry::$($c.Name)" -Name "$($p)")."$($p)"
        if ($propValue -match $productName) 
        {
            Write-Output $propValue
            $count++
            Remove-Item "Registry::$($c.Name)" -Recurse
        }
    }
}

Write-Host "$($count) key(s) removed"