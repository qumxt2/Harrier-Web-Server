@echo off

REM analyzeGCA.bat
REM Copyright Graco Inc. 2007-2010
REM
REM This batch file is used to analyze the Graco Control Architecture source code using
REM Understand 2.0 Pro (analysis software from SciTools). Understand must be installed on
REM the PC for this batch file to run properly.
REM The command line version of Understand must be located at the value of variable _understandExe
REM It can be used for any of the common, bootloader, component, or application software.
REM
REM The batch file is called with the following usage.
REM analyzeGCA
REM
REM This batch file uses Log Parser 2.2 from Microsoft to create chars from the .csv
REM files created by Understand. Log Parser is freeware and may downloaded from
REM Microsoft.com. Also, the automatic chart creation done by Log Parser requires
REM that Microsoft Office 2000 or newer and Microsoft Office Chart Web Component
REM are installed. The basic analysis will still run without Log Parser installed.
REM Log Parser 2.2 and owc11.exe are stored at $/GCA2/TOOLS/Log Parser 2_2/
REM
REM The batch file also uses perl scripts. The batch file assumes that perl is installed
REM at C:\PERL\bin\perl.exe. This file is installed during the standard setup for Teamcenter.
REM These perl scripts run the basic analysis and summarie the metrics.

echo I am analyzeGCA
echo.
echo Running analysis of code using Understand.
echo Wait for database to be created ...
echo.

REM set up variables
set _databaseFile="project.udb"
set _perlExe="C:\PERL\bin\perl.exe"
set _startDir=%CD%
if exist "C:\Program files (x86)" (
	set _understandExe="C:\Program Files (x86)\STI\bin\pc-win32\und.exe"
) else (
	set _understandExe="C:\Program Files\STI\bin\pc-win32\und.exe"
)

REM Setup Understand database
REM Create an include file list
echo. > includeFolders.txt
FOR /D %%H in (..\..\..\src\*) DO Echo %%H >> includeFolders.txt
FOR /D %%G in (..\..\..\*) DO FOR /D %%H in (%%G\src\*) DO Echo %%H >> includeFolders.txt

REM Create database
%_understandExe% -db %_databaseFile% -create -languages C++ -define __attribute__() -include @includeFolders.txt
REM Define DEBUG_PRINT macros to be nothing so that complexity is not counted
%_understandExe% -db %_databaseFile% -define NO_DEBUG_PRINT  DEBUG_PRINT_STRING() DEBUG_PRINT_SIGNED_DECIMAL() DEBUG_PRINT_UNSIGNED_DECIMAL() DEBUG_PRINT_UINT8() DEBUG_PRINT_UINT16() DEBUG_PRINT_UINT32()
REM Add files
%_understandExe% -db %_databaseFile% -addFiles ..\..\..\src\*.c ..\..\..\src\*.h
FOR /D %%G in (..\..\..\LIB*) DO %_understandExe% -db %_databaseFile% -addFiles %%G\src\*.c %%G\src\*.h
FOR /D %%G in (..\..\..\SRC*) DO %_understandExe% -db %_databaseFile% -addFiles %%G\*.c %%G\*.h

REM Set up the output file names
set _metricsFile="Metrics.csv"
set _AnalysisFile="Analysis.txt"
set _HistogramChart="Histogram.jpg"
set _ComplexityChart="Complexity.jpg"

REM Setup names of SQL and Perl script (PL) analysis files
set _HistogramOfFunctionComplexity="C:\GCA2\TOOLS\understand\HistogramOfFunctionComplexity.sql"
set _ComplexityChartQuery="C:\GCA2\TOOLS\understand\ComplexityChartQuery.sql"
set _CodeAnalysisPerl="C:\GCA2\TOOLS\understand\CodeAnalysis.pl"
set _metricsFormatPerl="C:\GCA2\TOOLS\understand\formatMetrics.pl"

REM Analyze the files and create a metrics report 
echo Analyzing the database
echo. 
%_understandExe% -db %_databaseFile% -analyzeAll 

REM Create the metrics file. The first command sets the desired metrics.
REM The second command sets that the short file name of each function should be included.
REM The third creates the actual file.
REM The fourth calls a Perl script that converts the formatting of the metrics.csv file.
%_understandExe% -db %_databaseFile% -metrics_sel Cyclomatic CountLineCode CountDeclFunction SumCyclomatic
%_understandExe% -db %_databaseFile% -metrics_DeclFilename short 
%_understandExe% -db %_databaseFile% -metrics %_metricsFile%
%_metricsFormatPerl%

REM Location of Log Parser executable.
if exist "C:\Program files (x86)" (
	set _logParserCmd="C:\Program Files (x86)\Log Parser 2.2\LogParser"
) else (
	set _logParserCmd="C:\Program Files\Log Parser 2.2\LogParser"
)

REM Analyze software to examine lines of code, number of functions and sum of complexity
echo.
echo Analysis of GCA2 software
echo Analysis of GCA2 software > %_AnalysisFile%
echo %CD% >> %_AnalysisFile%
echo. >> %_AnalysisFile%

REM Output lines of code, number of functions, and sum of the complexity.
echo ******************************** >> %_AnalysisFile%
%_perlExe% %_CodeAnalysisPerl% < %_metricsFile% >> %_AnalysisFile%
echo. >> %_AnalysisFile%

REM ************************************************************
REM Use Log Parser to make JPEG charts
REM ************************************************************

REM Create a JPEG chart of the histogram of function complexity
%_logParserCmd% file:%_HistogramOfFunctionComplexity%?inputFile=%_metricsFile%+outputFile=%_HistogramChart% -o:CHART -chartType:columnClustered -stats:OFF -categories -i:CSV

REM Create a JPEG chart of all functions complexity
%_logParserCmd% file:%_ComplexityChartQuery%?inputFile=%_metricsFile%+outputFile=%_ComplexityChart% -o:CHART -chartType:columnClustered -stats:OFF -groupSize:800x600 -i:CSV
 