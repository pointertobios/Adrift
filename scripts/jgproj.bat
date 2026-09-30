@echo off
setlocal
python "%~dp0jgproj" %*
exit /b %ERRORLEVEL%
