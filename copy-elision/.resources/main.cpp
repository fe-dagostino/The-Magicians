#include <iostream>
#include <map>
#include <vector>
#include <iomanip>    // needed by setprecision
#include <chrono>
#include <algorithm>

using namespace std;

typedef chrono::steady_clock::time_point tp;

double print_duration( tp ts, tp te )
{
  std::chrono::duration<double> duration =  (te - ts);
  cout << setprecision(9) << "Duration = " << duration.count() << "s ≈ " 
  << (duration.count() * 1000      ) << "ms ≈ "  
  << (duration.count() * 1000000   ) << "µs ≈ "
  << (duration.count() * 1000000000) << "ns   " << endl;

  return (duration.count() * 1000 ); // return milliseconds
}

std::vector<int> map2vec_ce( const std::map<int,int>& map ) 
{
  std::vector<int> out;

  for ( auto iter = map.begin(); iter != map.end(); ++iter )
  {
    if (( iter->second % 2 ) == 0)
      out.push_back(iter->second);
  }

  std::sort( out.begin(), out.end() );

  return out;
}

void map2vec_ref( const std::map<int,int>& map, std::vector<int>& out ) 
{
  out.clear();

  for ( auto iter = map.begin(); iter != map.end(); ++iter )
  {
    if (( iter->second % 2 ) == 0)
      out.push_back(iter->second);
  }

  std::sort( out.begin(), out.end() );
}

void compare_copy_elision()
{
  const size_t      _nRepeat       = 10000000;

  cout << "**************************************************************" << endl;
  cout << "          copy_elision    VS    reference + clear()           " << endl;
  cout << "**************************************************************" << endl;
  cout << "                                                              " << endl;
  cout << " ITERATIONS: " << _nRepeat << endl << endl;

  const std::map<int,int> _map = {
    {  1,  0}, {  2,  1}, {  3,  2}, {  4,  3}, {  5,  4},
    {  6,  5}, {  7,  6}, {  8,  7}, {  9,  8}, { 10,  9},
    { 11, 10}, { 12, 11}, { 13, 12}, { 14, 13}, { 15, 14},
    { 16, 15}, { 17, 16}, { 18, 17}, { 19, 18}, { 20, 19}
  };

  auto ts_ce = chrono::steady_clock::now();
  {
    for ( size_t _counter = 0; _counter < _nRepeat; ++_counter )
    {
      std::vector<int> out = map2vec_ce( _map );
    }
  }
  auto te_ce = chrono::steady_clock::now();

  auto ts_ref = chrono::steady_clock::now();
  {
    std::vector<int> out;    
    for ( size_t _counter = 0; _counter < _nRepeat; ++_counter )
    {
      map2vec_ref( _map,  out );
    }
  }
  auto te_ref = chrono::steady_clock::now();

  cout << "Benchmark with map2vec_ce" << endl;
  double dCopyElision = print_duration(ts_ce,te_ce);
  cout << endl;

  cout << "Benchmark with map2vec_ref" << endl;
  double dReference = print_duration(ts_ref,te_ref);
  cout << endl;


  double dPercentage = ((1.0-(std::min( dReference, dCopyElision ) / std::max( dReference, dCopyElision ))) * 100);
  if (dCopyElision > dReference)
  {
    cout << " map2vec_ce() is slower than map2vec_ref() by " << dPercentage << "%" << endl;
    cout << " USE map2vec_ref() IMPLEMENTATION FOR BEST PERFORMANCES" << endl;
  }
  else
  {
    cout << " map2vec_ref() is slower than map2vec_ce() by " << dPercentage << "%" << endl;
    cout << " USE map2vec_ce() IMPLEMENTATION FOR BEST PERFORMANCES" << endl;
  }
}

int main(/* int argc, char* argv[]*/ )
{
  compare_copy_elision();

  return 0;
}

