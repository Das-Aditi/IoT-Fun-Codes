@echo off
title Driver Monitor USB Bridge
echo Starting Driver Monitor USB Stream...
cd /d "%~dp0"
python bridge.py
pause