# MyShellProgram

Project Description:
For my project I wanted to create my own shell in order to test my ability at implementing a useable interface.
The main goal was to implement my own version of the “|” pipes operator.
The “|” pipe operator takes the output of one command and sends it to another command as its input.
Syntax: “cat test.txt | head -4”
Connect and pipe 2 commands together.

Implemented Features:
Execute basic Linux commands.
Execute Pipes on two commands, along with pipes on multiple commands.
History Buffer that allows the user to view recent history and execute commands in history.
Time flag to see commands completion time.
Help command to see built-in commands.
Type “exit” to close shell.

Compiled with a makefile using “make”
Run using “./shell”
