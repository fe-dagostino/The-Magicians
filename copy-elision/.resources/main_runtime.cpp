#include <iostream>
#include <map>
#include <vector>
#include <iomanip>    // needed by setprecision
#include <chrono>
#include <algorithm>

using namespace std;

typedef chrono::steady_clock::time_point tp;

constexpr double get_duration_ms( tp ts, tp te )
{
  std::chrono::duration<double> duration =  (te - ts);
  return (duration.count() * 1000 ); // return milliseconds
}

double print_duration( tp ts, tp te )
{
  std::chrono::duration<double> duration =  (te - ts);
  cout << setprecision(9) << "Duration = " << duration.count() << "s ≈ " 
  << (duration.count() * 1000      ) << "ms ≈ "  
  << (duration.count() * 1000000   ) << "µs ≈ "
  << (duration.count() * 1000000000) << "ns   " << endl;

  return (duration.count() * 1000 ); // return milliseconds
}

std::vector<int> get_vector_ce( int items ) noexcept
{
  std::vector<int> out;

  for ( auto i = 0; i < items; ++i )
  {
    out.push_back(i);
  }

  return out;
}

constexpr void get_vector_ref( int items, std::vector<int>& out ) noexcept
{
  for ( auto i = 0; i < items; ++i )
  {
    out.push_back(i);
  }
}

void compare_copy_elision()
{
  const size_t      _nRepeat       = 1000000;

  std::cout << "copy-elision" << std::endl;
  for ( int _items = 10; _items <= 100; _items += 10 )
  {
    auto ts_ce = chrono::steady_clock::now();
    {
      for ( size_t _counter = 0; _counter < _nRepeat*_items; ++_counter )
      {
        std::vector<int> out = get_vector_ce( /*_items*/1 );
      }
    }
    auto te_ce = chrono::steady_clock::now();
    std::cout << _nRepeat*_items << "," <<  get_duration_ms(ts_ce,te_ce) << std::endl;
  }

  std::cout << std::endl << "move-semantic" << std::endl;
  for ( int _items = 10; _items <= 100; _items += 10 )
  {
    auto ts_ce = chrono::steady_clock::now();
    {
      std::vector<int> out;
      for ( size_t _counter = 0; _counter < _nRepeat*_items; ++_counter )
      {
        out = get_vector_ce( /*_items*/1 );
      }
    }
    auto te_ce = chrono::steady_clock::now();
    std::cout << _nRepeat*_items << "," <<  get_duration_ms(ts_ce,te_ce) << std::endl;
  }

  std::cout << std::endl << "reference + create" << std::endl;
  {
    for ( int _items = 10; _items <= 100; _items += 10 )
    {
      auto ts_ref = chrono::steady_clock::now();
      {
        for ( size_t _counter = 0; _counter < _nRepeat*_items; ++_counter )
        {
          std::vector<int> out; 
          get_vector_ref( /*_items*/1,  out );
        }
      }
      auto te_ref = chrono::steady_clock::now();
      std::cout << _nRepeat*_items << "," <<  get_duration_ms(ts_ref,te_ref) << std::endl;
    }
  }

  std::cout << std::endl << "reference + clear" << std::endl;
  {
    for ( int _items = 10; _items <= 100; _items += 10 )
    {
      auto ts_ref = chrono::steady_clock::now();
      {
        std::vector<int> out;    
        for ( size_t _counter = 0; _counter < _nRepeat*_items; ++_counter )
        {
          out.clear(); 
          get_vector_ref( /*_items*/1,  out );
        }
      }
      auto te_ref = chrono::steady_clock::now();
      std::cout << _nRepeat*_items << "," <<  get_duration_ms(ts_ref,te_ref) << std::endl;
    }
  }
}

int main(/* int argc, char* argv[]*/ )
{
  compare_copy_elision();

  return 0;
}

