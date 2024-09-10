# RunLintGCA.pl

# This Perl Script runs lint a single set of code in the GCA. This file must be placed in
# the folder ../tools/dev/lint relative to the source code to be analyzed.

# The following code with include all folders in "..\..\..\src" as part of the include directories when list runs.
# A temporary file called interfaceList.lnt is created and then removed by this batch file. Any previously existing file with
# that name will be destroyed.

# echo // interfaceList.lnt > interfaceList.lnt
# Get the list of directories
opendir (SRC_DIR, "../../../src") or die "Error opening directory: $!";
# Get list of diretories only
@interfaceFiles = grep !/\./, readdir SRC_DIR;
closedir SRC_DIR;

# Add information to list of diretories as needed by lint.
# The form of each line of text in the file should be
#	-i"..\..\..\src\INTERFACE_XXX"
# where INTERFACE_XXX is on the folders in the src folder.
@interfaceFiles = map { "-i\"..\\..\\..\\src\\$_\"" } @interfaceFiles;

# Print out the included folders so user can see them
print "\nThe included folders are \n";
foreach $item (@interfaceFiles)
{
	print "$item\n";
}

open (INTER_FILE, ">interfaceList.lnt");
foreach $item (@interfaceFiles)
{
	printf INTER_FILE "$item\n";
}
close (INTER_FILE);

# Determine which processor is used and create a define
# Note that if two folders are included with conflicting processors,
# this code will set the last one.
foreach $item (@interfaceFiles)
{
	if ($item =~ /PIC24HJ256GP610/)
	{
		$processorDefine = "-d__PIC24HJ256GP610__";
		$lintOptionsFiles = '"C:\\GCA\\TOOLS\\lint\\GCA_Options.lnt" "interfaceList.lnt" -i"..\\..\\..\\src"';
	}
	elsif ($item =~ /DSPIC33FJ128MC804/)
	{
		$processorDefine = "-d__dsPIC33FJ128MC804__";
		$lintOptionsFiles = '"C:\\GCA\\TOOLS\\lint\\GCA_Options.lnt" "interfaceList.lnt" -i"..\\..\\..\\src"';
	}
	elsif ($item =~ /PIC32MX795F512L/)
	{
		$processorDefine = "-d__PIC32MX795F512L__";
		$lintOptionsFiles = '"C:\\GCA\\TOOLS\\lint\\GCA_Options_PIC32.lnt" "interfaceList.lnt" -i"..\\..\\..\\src"';
	}
}
printf ("\nThe processor define is %s\n\n", $processorDefine);

# Run lint now
print "Running lint...\n\n";
# Use single quotes in variables here so that " show in the strings
$lintProgram = '\\\\sv1\\lint\\lint-nt';
$lintAlwaysOptions = '+v +fem -u';
$lintSourceFiles = '"..\\..\\..\\src\\*.c"';
$lintOutputFile = 'lintOutput.txt';

# Call the lint program
$lintCommand = "$lintProgram $lintAlwaysOptions $processorDefine $lintOptionsFiles $lintSourceFiles > $lintOutputFile";
printf ("$lintCommand\n");
system ("$lintCommand");

# Remove interfaceList.lnt
unlink ("interfaceList.lnt");
