@echo off
setlocal enabledelayedexpansion

set PRJ_PATH=%~dp0
set PROJECT_PATH=%PRJ_PATH:~,-1%

cd %PROJECT_PATH%

@cmd.exe
