@echo off
setlocal
python "%~dp0adftproj" %*
exit /b %ERRORLEVEL%
