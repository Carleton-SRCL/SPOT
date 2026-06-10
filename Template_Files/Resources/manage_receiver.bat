@echo off
if "%1"=="start" goto start
if "%1"=="stop" goto stop

echo Usage:
echo     control_script.bat start   - To start the script
echo     control_script.bat stop    - To stop the script
exit /b

:start
echo Starting the script...

REM Start the Python script in the background
start /min pythonw stream_receive_all.py

echo Script started.
exit /b

:stop
echo Stopping the script...

REM Look for the PID file and kill the process
if exist stream_receiver.lock (
    set /p PID=<stream_receiver.lock
    taskkill /pid %PID% /f >nul 2>&1
    del stream_receiver.lock
    echo Script stopped.
) else (
    echo Script not running.
)

exit /b
