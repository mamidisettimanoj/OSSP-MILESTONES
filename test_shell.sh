#!/bin/bash
# Test script for ShellForge memory audit

sleep 2 &
sleep 1 &
jobs
sleep 1

fg 1 2>/dev/null || true
sleep 1

sleep 3 &
bg 1 2>/dev/null || true
sleep 1

jobs
history
pwd
cd /tmp
cd -
exit
