# bacon
A program to play Six Degrees of Kevin Bacon.  The program loads an input
file, builds a graph, and then performs a BFS (breadth first search) to
find the shortest path to Kevin Bacon.

Usage: ./bacon input_file
  Each line of the input file shall consist of a movie name followed by the list of 
  actors.  Each field in the input file is delimited by the / character 
  Example:
Avengers: Age of Ultron (2015)/Downey Jr., Robert/Johansson, Scarlett/Ruffalo, Mark

Sample input files can be found at http://introcs.cs.princeton.edu/java/45graph/
