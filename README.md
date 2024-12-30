search for files in the file system
query algorithm:
Easiest way. Just type the words (or the parts of the words) separating 
them with spaces.

Algorithm.

1. Spliting string to some strings. Separator is space. The AND 
operation is applied to these substrings.
2. If the substring contains the character '|', it is the OR 
operation. (the substring should not contain spaces)
3. If the first symbol is "!" , it is the NOT operation.

Example. <br>
search string: "comp .cpp" .  <br>
Search for filenames containing "comp" and ".cpp"  <br>
Possible output:  <br>
c:/build/Desktop-Debug/untitled_autogen/mocs_compilation.cpp  <br>
c:/build/Desktop-Debug/CMakeFiles/untitled.dir/untitled_autogen/mocs_com <br>
pilation.cpp.o.d <br>
c:/libAACenc/src/metadata_compressor.cpp <br>
 <br>
add "!.o.d", i.e. search string "comp .cpp !.o.d" . <br>
Search for filenames containing "comp" and ".cpp" and not containing ".o.d"  <br>
Output: <br>
c:/build/Desktop-Debug/untitled_autogen/mocs_compilation.cpp <br>
c:/libAACenc/src/metadata_compressor.cpp <br>
 <br>
search string "comp .cpp|.exe !.o.d" . <br> 
Search for filenames containing "comp" and (".cpp" or ".exe") and not containing ".o.d"
Output: <br>
c:/build/Desktop-Debug/untitled_autogen/mocs_compilation.cpp <br>
c:/libAACenc/src/metadata_compressor.cpp <br>
c:/libAACenc/metadata_compressor.exe <br>
