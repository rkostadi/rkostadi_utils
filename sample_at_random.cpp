#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <utility>
#include <unordered_set>

// For getopt

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

using namespace std;


// GSL is used to simulate paired-end reads
// with insert size from a normal distribution
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>


/* The name of this program.  */
const char* program_name;

void print_usage (FILE* stream, int exit_code)
{
  fprintf (stream, "Usage: %s [options]\n", program_name);
  fprintf (stream,
           "  -a  --infile             FILE  Input file of ints (default: stdin)\n"	   
           "  -b  --array_size         UINT  Instead of an input file, simulate array_size integers, each between 0 and array_size-1\n"	   
	   "  -n  --number             UINT  The number of elements to sample\n"
	   "  -r  --random_seed        ULONG Unsigned long integer for the random seed\n"
	   "  -s  --sorted                   Toggle, if output needs to be sorted\n"
	   "  -t  --type                     Toggle for std::unordered_set implementation (default: vector-swap)\n"	   
	   "  -w  --with_replacement         Toggle, if random sampling with replacement\n"
	   "  -o  --outfile            FILE  Output bedgraph at positions in cpg_infile\n"
           "  -h  --help                     Display this usage information.\n");
  exit (exit_code);
}


/* Main, give the input file
 *
 */
int main(int argc, char** argv){


  int next_option;

  /* A string listing valid short options letters.  */
  const char* const short_options = "a:b:r:n:stwg:o:h";
  /* An array describing valid long options.  */
  const struct option long_options[] = {
    { "infile",   1, NULL, 'a' },
    { "array_size",   1, NULL, 'b' },
    { "random_seed",   1, NULL, 'r' },
    { "number", 1, NULL, 'n'},
    { "sorted", 0, NULL, 's'},
    { "type", 0, NULL, 't'},
    { "with_replacement", 0, NULL, 'w'},
    { "outfile",   1, NULL, 'o' },
    { "help",     0, NULL, 'h' },
    { NULL,       0, NULL, 0   }   /* Required at end of array.  */
  };

  /* Program name */
  program_name = argv[0];
  string infile;
  string outfile;
  int rseed=0;
  int N = 1;
  //bool needs_to_be_sorted = false;
  bool with_replacement = false;
  bool algo_type_flag = false;
  int array_size=-1;
  
  do {
    next_option = getopt_long (argc, argv, short_options,
                               long_options, NULL);
    switch (next_option){
    case 'a':
      infile = string(optarg);
      break;
    case 'b':
      array_size = atoi(optarg);
      break;
    case 'n':
      N = atoi(optarg);
      break;
    case 'r':
      rseed = atoi(optarg);
      break;
    case 's':
      //needs_to_be_sorted = true;
      break;
    case 't':
      algo_type_flag = true;
      break;
    case 'w':
      with_replacement = true;
      break;      
    case 'o':
      outfile = string(optarg);
      break;
    case 'h':   /* -h or --help */
      /* User has requested usage information.  Print it to standard
         output, and exit with exit code zero (normal termination).  */
      print_usage (stdout, 0);
      break;
    case '?':   /* The user specified an invalid option.  */
      /* Print usage information to standard error, and exit with exit
	 code one (indicating abonormal termination).  */
      print_usage (stderr, 1);
      break;
    case -1:    /* Done with options.  */      
      break;      
    default:    /* Something else: unexpected.  */
      print_usage (stdout, 1);
      abort ();
    }
  }
  while (next_option != -1);

  // Seed the random number generator
  unsigned long int random_seed = (unsigned long int)(rseed);
  const gsl_rng_type * T;
  gsl_rng * r;    
  gsl_rng_env_setup();
  // Choose type, taus is fastest
  // https://www.gnu.org/software/gsl/manual/html_node/Random-Number-Generator-Performance.html
  T = gsl_rng_taus;
  r = gsl_rng_alloc (T);
  // Seed the random number generation
  gsl_rng_set(r,random_seed);

  // The vector
  vector<int> v;

  if(array_size > 0){
    for(int i=0; i<array_size; i++){    
      unsigned long int element = gsl_rng_uniform_int(r, (unsigned long int)array_size);
      v.push_back(element);
    }
  } else {
    // prepare input
    std::streambuf * inbuf;
    std::ifstream ifs;
    if(!infile.empty()) {
      ifs.open(infile.c_str());
      inbuf = ifs.rdbuf();
    } else {
      inbuf = std::cin.rdbuf();
    }
    std::istream istr(inbuf); // input stream either from file or stdin
    
    // Load v
    while( !istr.eof() ){
      int i;
      istr >> i;
      v.push_back(i);
    }
  }
  
  // Output
  // prepare output
  std::streambuf * outbuf;
  std::ofstream ofs;
  if(!outfile.empty()) {
    ofs.open(outfile.c_str());
    outbuf = ofs.rdbuf();
  } else {
    outbuf = std::cout.rdbuf();
  }
  std::ostream ostr(outbuf); // output stream either to file or stdout
  

  // Sample  

  if(algo_type_flag){
    if(with_replacement){
    } else {
      // std::unordered_set
      std::unordered_set<int> uset;
      if((size_t)N>v.size()){
	cerr << "[" << program_name << "] " << " error: cant sample " << N << " elements without replacement from " << v.size() << " elements\n";  
	exit(EXIT_FAILURE);
	
      }
      while(uset.size()<(size_t)N){
	unsigned long int element = gsl_rng_uniform_int(r, (unsigned long int)v.size());
	uset.insert(v[(size_t)element]);
      }
      for(std::unordered_set<int>::iterator it=uset.begin(); it!=uset.end(); ++it){
	ostr << (*it) << "\n";
      }
    }
  } else {
    // sample N ints, with or without replacement
    if(with_replacement){
    } else {
      if((size_t)N>v.size()){
	cerr << "[" << program_name << "] " << " error: cant sample " << N << " elements without replacement from " << v.size() << " elements\n";  
	exit(EXIT_FAILURE);
      } else {
	unsigned long int p = (unsigned long int) v.size();
	for(int i=0; i<N; ++i){
	  // integers from [0,p-1] with  equal probability
	  unsigned long int element = gsl_rng_uniform_int(r, p);
	  //sampled.push_back(v[(size_t)element]);
	  // Swap chosen element with the "back" element
	  std::swap(v[(size_t)element],v[(size_t)p-1]);
	  // Decrease the "sampled" vector size
	  p--;
	}
	// Loop and output the randomly sampled ints
	for(size_t i=v.size()-1; i>=v.size()-(size_t)N; --i){
	  ostr << v[i] << "\n";
	}
      }
    }
  }

  return 0;
}


/**
 * Copyright (C) 2017  Rumen Kostadinov
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


