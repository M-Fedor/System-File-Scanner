$projectPath = "$HOME\git\File-Identification-System\"

$confFileFis = "$projectPath\conf\FileIdentificationSystem.conf"
$confFileMgr = "$projectPath\conf\SystemSnapshotManager.conf"

# Evaluate user parameters
function printHelp() {
    Write-Output "System Snapshot Manager v0.1`n"
    Write-Output "Usage: SystemSnapshotManager [options]"
    Write-Output "Options:"
    Write-Output "`t-h`t--help`t`tPrints this help."
    Write-Output "`t`t--turn-off`tSystem Snapshot Manager will NOT be started on system boot."
    Write-Output "`t`t--turn-on`tSystem Snapshot Manager will be started on every system boot.`n"
    Write-Output "In case of any problems, please contact `<matej.fedor.mf@gmail.com>.`n";
}

$option = $args[0]
switch ($option) {
    ""                  { break }
    ("-h" -or "--help") { printHelp 
                        exit }
    "--turn-off"        { Set-Content -Path $confFileMgr -Value 0 
                        exit }
    "--turn-on"         { Set-Content -Path $confFileMgr -Value 1 
                        exit }
    
    default             { printHelp 
                        exit }
}

# Create log file
$logFile = $projectPath
if (!(Test-Path "$logFile\data\")) {
    New-Item -Path "$logFile\data\" -ItemType Directory
}
$logFile = "$logFile\data\"
if (!(Test-Path "$logFile\SysUpdate.log")) {
    New-Item -Path "$logFile\SysUpdate.log" -ItemType File
}
$logFile = "$logFile\SysUpdate.log"


Add-Content -Path $logFile -Value "-----------------------------------------------------------------`n$(Get-Date)`n"
Add-Content -Path $logFile -Value "System Snapshot Manager v0.1"
Add-Content -Path $logFile -Value "Checking phase of snapshot creation from configuration file...`n"


# Obtain phase of creating system snapshot.
# Phases are stored in configuration file, so the snapshot process can be restored after necessary reboot.
# 0 - No action requested - use this value, if you don't wanna take any action on system boot.
# 1 - Update operating system.
# 2 - Update other packages.
# Anything else - Create snapshot of current state of file system and store it in database.
[int]$phase = Get-Content -Path $confFileMgr

if ($phase -eq 0) {
    Add-Content -Path $logFile -Value "Phase $phase`: No action requested"
    exit
}

if ($phase -eq 1) {
    Add-Content -Path $logFile -Value "Phase $phase`: Update operating system`n"
    & $projectPath\build\SysUpdate.exe --verbose --list *>> $logFile
    $exitCode = $LASTEXITCODE

    Add-Content -Path $logFile -Value "Exited with code $exitCode"
    if ($exitCode -eq 1) { 
        Add-Content -Path $logFile -Value "Exiting!"
        exit 
    }

    $phase++
    Set-Content -Path $confFileMgr -Value $phase
    if ($exitCode -eq 2) {
        Add-Content -Path $logFile -Value "Rebooting..."
        Restart-Computer -Force
    }
}

Add-Content -Path $logFile -Value "Phase $($phase - 1)`: DONE!"

if ($phase -eq 2) {
    Add-Content -Path $logFile -Value "`nPhase $phase`: Update other packages`n"

    # Run all custom scripts for other software package update
    Get-ChildItem -Path "$projectPath\scripts\custom" | ForEach-Object {
        Add-Content -Path $logFile -Value "Running script $projectPath\scripts\custom\$_...`n"
        & $projectPath\scripts\custom\$_.FullName *>> $logFile
        Add-Content -Path $logFile -Value "`n"
    }

    $phase++
    Set-Content -Path $confFileMgr -Value $phase
    Add-Content -Path $logFile -Value "Phase $($phase - 1)`: DONE!"
}

Add-Content -Path $logFile -Value "`nCreating and storing snapshot of current state of file system...`n"
Get-Content -Path $confFileFis | & $projectPath\build\File-Identification-System.exe --offline 2>&1> Out-Null
$exitCode = $LASTEXITCODE

if ($exitCode -eq 0) {
    Add-Content -Path $logFile -Value "Phase 3: DONE!`n`n"
    exit
}
Add-Content -Path $logFile -Value "Phase 3: FAILED!`n`n"
exit






