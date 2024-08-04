@echo off
setlocal enabledelayedexpansion

REM Check current Git global email configuration
for /f "delims=" %%i in ('git config --global user.email') do set "current_email=%%i"

echo Current Git global email: %current_email%

REM Set new email and name
set "new_email=tim.trussner@gmail.com"
set "new_name=Tim Trussner"

REM Update Git global email configuration
git config --global user.email "%new_email%"
git config --global user.name "%new_name%"
echo Git global email updated to: %new_email%
echo Git global name updated to: %new_name%

REM Rewrite commit history
echo Rewriting commit history...
git filter-branch --env-filter "
set NEW_NAME=%new_name%
set NEW_EMAIL=%new_email%

if [ !"" == "%GIT_COMMITTER_EMAIL%" ] (
    export GIT_COMMITTER_NAME=%NEW_NAME%
    export GIT_COMMITTER_EMAIL=%NEW_EMAIL%
)
if [ !"" == "%GIT_AUTHOR_EMAIL%" ] (
    export GIT_AUTHOR_NAME=%NEW_NAME%
    export GIT_AUTHOR_EMAIL=%NEW_EMAIL%
)
" --tag-name-filter cat -- --branches --tags

echo Commit history rewritten. Please push the changes with --force option.

endlocal
pause
