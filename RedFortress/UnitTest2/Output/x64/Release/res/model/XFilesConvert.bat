@echo off
rem ------------------------------------------------------------
rem  XtoBinaryX.bat  ?  MeshConvert.exe で .x をバイナリ化
rem  使い方 : .x ファイルをこの BAT にドラッグ＆ドロップ
rem ------------------------------------------------------------
if "%~1"=="" (
    echo 変換したい .x ファイルをドラッグ＆ドロップしてください。
    pause
    goto :eof
)

rem カレントを MeshConvert.exe のあるフォルダーに変更
pushd "%~dp0"

for %%F in (%*) do call :CONVERT "%%~fF"

popd
goto :eof


:CONVERT
rem ---------- 1ファイル分 ----------
setlocal
set "SRC=%~1"

rem 拡張子チェック
if /I not "%~x1"==".x" (
    echo [%SRC%] は .x ではないためスキップ。
    goto :EOF
)

echo.
echo === [%SRC%] をバイナリ .x に変換 ===

rem 一時ファイル名（同じフォルダーに *_bin.x を作成）
set "DST=%~dpn1_bin.x"

rem /x  : .x バイナリ形式で出力
rem /o  : 出力ファイル名
rem /y  : 既存ファイルを上書き
MeshConvert.exe "%SRC%" /x /o "%DST%" /y >nul
if errorlevel 1 (
    echo 変換失敗...
    goto :EOF
)

rem 元ファイルを置き換え
move /Y "%DST%" "%SRC%" >nul
echo 変換完了。

endlocal
goto :EOF
