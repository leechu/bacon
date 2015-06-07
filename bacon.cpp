#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <iostream>
#include <sstream>
#include <deque>
#include <time.h>
#include <stdlib.h>

// You can easily change this to play the game with an actor other
// than Kevin Bacon.
//
const char *g_baconString = "Bacon, Kevin";

#define NUM_AUTO_TEST 5
#define FILE_DELIMITER '/'
#define BACON_ARG_INPUT_FILE 1
#define BACON_ARG_MAX (BACON_ARG_INPUT_FILE+1)

using namespace std;

// Each edge links to another actor via a specific movie
//
struct Edge
{
  unordered_set<string>::iterator     movie;
  unordered_map<string,int>::iterator actor;
};

// This is our adjacency list
//
typedef deque<Edge> edgeList;

// The main control block for this application.
// All the important structures are anchored here.
//
struct BaconCB
{
  // Hash tables to allow us to quickly lookup a movie
  // or actor's name.  We only want to store the strings once
  // and access them via iterators to save on memory.
  //
  unordered_set<string>      m_moviesMap;
  unordered_map<string, int> m_actorsMap;

  // This is an array of all our nodes in
  // the graph
  //
  deque< edgeList >          m_graphVertex;
  int                        m_numActors;

  // Command line parameters
  //
  char *command;
  char *inputFile;
};

// Open the input file and build the hash tables.
//
static void parseFile( BaconCB *bCB)
{
  const char *inputFile = bCB->inputFile;
  std::ifstream file(inputFile);
  std::string token;
  int t0 = time(NULL);

  if ( !file.is_open() )
  {
    cout << "Error opening input file " << inputFile << endl;
    perror("");
  }
  else
  {
    cout << "Parsing file and building hash tables..." << endl;

    // Foreach line in the input file...
    //
    while (getline(file, token, '\n') )
    {
      unsigned int numRead = 0;
      std::istringstream line(token);
      unordered_set<string>::iterator movieIT;
      deque<unordered_map<string,int>::iterator> movieCast;

      // Break up each line by a delimiter, which is '/' by default.
      // The first tokenized string is the name of the movie.
      // The cast follows.
      //
      while ( getline(line, token, FILE_DELIMITER))
      {
        if ( !numRead )
        {
          pair<unordered_set<string>::iterator, bool> insertResult;

          insertResult = bCB->m_moviesMap.insert(token);

          // Remember the movie info
          //
          movieIT = insertResult.first;
        }
        else
        {
          // The insert() returns a pair value.  The first
          // is an iterator to the hash table location for this actor.
          // We can check the unique actor ID pointed to by the iterator
          // and if it is the same as what we just tried to insert,
          // then it means the actor was not present in the hash table
          // before our insert call.
          //
          pair<unordered_map<string,int>::iterator, bool> insertResult = 
            bCB->m_actorsMap.insert(make_pair(token, bCB->m_numActors));

          // Actor not found, so this is the first time we're seeing them.
          //
          if ( insertResult.first->second  == bCB->m_numActors )
          {
            // Adjust the unique ID that represents each actor.
            //
            bCB->m_numActors++;

            // Add a new vertex to our graph.
            //
            edgeList newList;
            bCB->m_graphVertex.push_back(newList);
          }

          // For each cast member in this movie, add it to an array.  We will
          // perform an O(n^2) loop over them later to create edges between
          // each actor.
          //
          movieCast.push_back(insertResult.first);
        }
        numRead++;
      }

      // Go through all the actors and create edges between them.  O(n^2)
      //
      struct Edge newEdge;
      newEdge.movie = movieIT;

      for ( int i = 0; i < movieCast.size() - 1; i++ )
      {
        for ( int j = i+1; j < movieCast.size(); j++ )
        {
          const int srcID = movieCast[i]->second;
          const int dstID = movieCast[j]->second;

          // Create a link from i to j
          // I tried using an unordered_map instead of a deque so that
          // there are no duplicate links, but that didn't improve
          // performance and the number of duplicate links wasn't that
          // significant.  In fact, the program performed slower when
          // building the hash tables and memory usage was higher.
          //
          //    11s           48s
          // 4.31GB  vs.   4.53GB
          //
          newEdge.actor = movieCast[j];
          bCB->m_graphVertex[srcID].push_back(newEdge);

          // Create a link back to i from j
          //
          newEdge.actor = movieCast[i];
          bCB->m_graphVertex[dstID].push_back(newEdge);
        }
      }
    }

    if ( file.bad() )
    {
      perror("Error while reading file");
    }

    // Calculate how long it took to do all of the above
    //
    int t1 = time(NULL);
    t1 -= t0;
    cout << "Done.  "<< t1 <<" seconds elapsed.  " 
         << bCB->m_graphVertex.size() << " vertices." << endl;
  }
  cout << endl;
}

#ifdef DEBUG
static void dumpMovies(BaconCB *bCB)
{
  unordered_set<string>::iterator it = bCB->m_moviesMap.begin();

  while ( it != bCB->m_moviesMap.end() )
  {
    cout << *it << endl;
    it++;
  }
  cout << "Number of vertices: " << bCB->m_actorsMap.size() << endl;
}

static void printAdjList( BaconCB *bCB,
                          const unordered_map<string,int>::iterator &it)
{
  const int id = it->second;
  edgeList::iterator edgeIT = bCB->m_graphVertex[id].begin();
  while ( edgeIT != bCB->m_graphVertex[id].end() )
  {
    cout << it->first << " was in \"" << *edgeIT->movie << "\" with " << edgeIT->actor->first << endl;

    edgeIT++;
  }
}

static void printStats(BaconCB *bCB ,
                       const unordered_map<string,int>::iterator &it )
{
  cout << it->first << " (# " << it->second << ") has " << bCB->m_graphVertex[it->second].size() << " edges." << endl;
}

static void printStats(BaconCB *bCB ,
                       const char *src )
{
  unordered_map<string,int>::iterator it = bCB->m_actorsMap.find(src);

  if ( it != bCB->m_actorsMap.end() )
  {
    printStats(bCB, it);
  }
  else
  {
    cout << "Unknown actor"<<endl;
  }
}
#endif

// This is our main function that will calculate the Bacon number
// and print out the relationships.
//
static void baconator( BaconCB *bCB ,
                       const char *src )
{
  unordered_map<string,int>::iterator bit = bCB->m_actorsMap.find(g_baconString);

  // Kevin Bacon should always be in our database
  //
  if ( bit != bCB->m_actorsMap.end() )
  {
    unordered_map<string,int>::iterator it = bCB->m_actorsMap.find(src);

    // Search for the actor for whom we want to calculate a Bacon number.
    //
    if ( it != bCB->m_actorsMap.end() )
    {
      // If the adjacency list is empty, then this actor was the sole person in a movie.
      //
      if ( bCB->m_graphVertex[it->second].empty() )
      {
        cout << it->first << " is not connected to anybody.  The actor must have been in a movie by themselves." << endl;
      }
      else
      {
        bool        foundPath = false;
        const int   BaconID = bit->second;
        const int   numVertices = bCB->m_actorsMap.size();
        deque<bool> visited(numVertices, false);

        Edge dummy;
        dummy.movie = bCB->m_moviesMap.end();
        dummy.actor = bCB->m_actorsMap.end();

        // This stores the edge that was last used to link to
        // another actor.  We can print out the path from our search 
        // actor to Kevin Bacon by backtracking starting from
        // the last actor that was used to get to Kevin Bacon.
        edgeList prev(numVertices, dummy);;

        deque<unordered_map<string,int>::iterator > work;

        // Enqueue our starting point
        //
        work.push_back(it);

        visited[it->second] = true;

        while ( !work.empty() )
        {
          unordered_map<string,int>::iterator curIT = work.front();
          const int currActor = curIT->second;
          work.pop_front();

          // Keep track of which actor to store in the
          // path we took to each edge.
          //
          dummy.actor = curIT;

          // If we just pulled Kevin Bacon off the work queue, then
          // our search is completed.
          //
          if ( currActor == BaconID )
          {
            foundPath = true;
            break;
          }

          // For all the edges in this actor's adjacency list, queue them onto
          // the work queue.  This is the breadth first search portion of
          // the code.
          //
          edgeList::iterator edgeIT = bCB->m_graphVertex[currActor].begin();
          while ( edgeIT != bCB->m_graphVertex[currActor].end() )
          {
            const int edgeID = edgeIT->actor->second;

            // Only enqueue unvisited vertices.
            //
            if ( !visited[edgeID] )
            {
              visited[edgeID] = true;
              unordered_map<string,int>::iterator nextIT = bCB->m_actorsMap.find(edgeIT->actor->first);
              work.push_back(nextIT);

              // Keep track of how we got to each actor by tracking the
              // edge that we traveled to get to the actor.  When all is said
              // and done, we will backtrack this path starting with Kevin
              // Bacon and that will tell us the shortest path from the
              // starting actor to Kevin Bacon.
              //
              dummy.movie = edgeIT->movie;

              prev[edgeID] = dummy;
            }

            edgeIT++;
          }
        }

        // Backtrack through the path array until we find the actor
        // we began our search with.
        //
        if ( foundPath )
        {
          edgeList shortestPath;
          const int maxDepth = 255;
          int i = 0;
          int currID = BaconID;

          while ( i < maxDepth &&
                  currID != it->second )
          {
            shortestPath.push_front( prev[currID] );
            currID = prev[currID].actor->second;
            i++;
          }

          cout << it->first << " has a Bacon number of " << shortestPath.size() <<endl;

          // Go through the path in reverse order now, and that will print out
          // the shortest path to Kevin Bacon.
          //
          for ( int j = 0;
                j < shortestPath.size() && i < maxDepth;
                j++ )
          {
            cout << j << ") "<< shortestPath[j].actor->first << " was in " << *shortestPath[j].movie << " with " ;
            if ( j < shortestPath.size() -1)
            {
              cout << shortestPath[j+1].actor->first;
              cout << endl;
            }
          }

          if ( shortestPath.size() > 0 )
          {
            cout << g_baconString <<endl;
          }
        }
        else
        {
          cout << it->first << " doesn't have a path to " << g_baconString << endl;
        }
      }
    }
    else
    {
      cout << src << " was not in my database" << endl;
    }
  }
  else
  {
    cout << "Couldn't find " << g_baconString << endl;
  }
  cout <<endl;
}

// A driver function to do some random testing.
//
static void automatedTest( BaconCB *bCB,
                           int      numToTest )
{
  // We don't have random access into the hash tables.
  // Instead, just randomly pick a vertex and then a random
  // actor from the adjacency list.
  //
  srand(time(NULL));
  for ( int i = 0; i < numToTest; i++ )
  {
    int randomVertex = 0;
    int numEdges = 0;

    // Loop until we find an actor with something in his/her
    // adjacency list.
    //
    do
    {
      randomVertex = rand() % bCB->m_graphVertex.size();
      numEdges = bCB->m_graphVertex[randomVertex].size();
    }
    while ( numEdges == 0 );

    int randomEdge = rand() % numEdges;

    unordered_map<string,int>::iterator tmpActor = bCB->m_graphVertex[randomVertex][randomEdge].actor;
    baconator(bCB, tmpActor->first.c_str() );
  }
}

static void interactiveMode( BaconCB *bCB )
{
  const char *msg = "Interactive mode.  Enter an actor's name to search.  CTRL-D to end.\n> ";

  string searchString;

  cout << msg ;
  while ( getline(cin, searchString) )
  {
    baconator(bCB, searchString.c_str());
    cout << msg ;
  }
}

static void printUsage(BaconCB *bCB)
{
  cout << "Usage: " << bCB->command << " input_file" << endl;
  cout << "  Each line of the input file shall consist of a movie name followed by the list of " <<endl;
  cout << "  actors.  Each field in the input file is delimited by the " << FILE_DELIMITER << " character " << endl;
  cout << "Example:" << endl;
  cout << "Avengers: Age of Ultron (2015)/Downey Jr., Robert/Johansson, Scarlett/Ruffalo, Mark" << endl;
}

static void parseArgs(BaconCB *bCB, int argc, char *argv[])
{
  bCB->inputFile = argv[BACON_ARG_INPUT_FILE];
}

int main( int argc, char *argv[] )
{
  BaconCB bCB;
  bCB.m_numActors = 0;
  bCB.inputFile = NULL;
  bCB.command =  argv[0];

  if ( argc > 1 &&
       argc <= BACON_ARG_MAX )
  {
    parseArgs(&bCB, argc, argv);

    parseFile(&bCB);

    // We can only allow searches if there are vertices in the graph!
    //
    if ( !bCB.m_graphVertex.empty() )
    {
      // By default, run 5 random searches before entering interactive mode.
      //
      automatedTest(&bCB, NUM_AUTO_TEST);

      interactiveMode( &bCB );
    }
  }
  else
  {
    printUsage(&bCB);
  }

  return 0;
}
