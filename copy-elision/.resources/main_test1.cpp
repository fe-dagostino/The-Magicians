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

std::vector<int> get_vector_nrvo( int items ) noexcept
{
  std::vector<int> out;

  for ( auto i = 0; i < items; ++i )
  {
    out.push_back(i);
  }

  return out;
}

constexpr void get_vector_by_reference_clear( int items, std::vector<int>& out ) noexcept
{
  // Clear out vector before to do any operation.
  // We don't know if the vector is empty or not, so we just make sure to 
  // do our operation on an empty vector.
  out.clear();

  for ( auto i = 0; i < items; ++i )
  {
    out.push_back(i);
  }
}


void compare_copy_elision()
{
  const size_t      _nRepeat       = 1000000;

  /////////////////////////////////////////////////////
  ///  CASE 1   
  ///
  ///  Calling get_vector_nrvo() with 1 Million time at each iteration
  ///  and increasing the number of items by 1000.
  ///
  ///  In this case the std::vector will created directly with the 
  ///  value returned by the get_vector_nrvo() and the optimization
  ///  will avoid to create extra copy of the vector.
  /// 
  std::cout << "NRVO" << std::endl;
  {
    for ( int _items = 1000; _items <= 10000; _items += 1000 )
    {
      /// Read the clock
      auto time_start = chrono::steady_clock::now();

      {
        for ( size_t _counter = 0; _counter < _nRepeat; ++_counter )
        {
          std::vector<int> out = get_vector_nrvo( _items );
        }
      }

      /// Read the clock
      auto time_end = chrono::steady_clock::now();

      std::cout << _items << "," <<  get_duration_ms(time_start,time_end) << std::endl;
    }
  }

  /////////////////////////////////////////////////////
  ///  CASE 2   
  ///
  ///  Calling get_vector_nrvo() with 1 Million time at each iteration
  ///  and increasing the number of items by 1000.
  ///
  ///  In this case return value will be optimized from the compiler 
  ///  using the move semantic so operator+(&&) will be automatically used.
  /// 
  std::cout << std::endl << "move-semantic" << std::endl;
  {
    for ( int _items = 1000; _items <= 10000; _items += 1000 )
    {
      /// Read the clock
      auto time_start = chrono::steady_clock::now();

      {
        std::vector<int> out;
        for ( size_t _counter = 0; _counter < _nRepeat; ++_counter )
        {
          // Push only one item 
          out = get_vector_nrvo( _items );
        }
      }

      /// Read the clock
      auto time_end = chrono::steady_clock::now();

      std::cout << _items << "," <<  get_duration_ms(time_start,time_end) << std::endl;
    }
  }

  /////////////////////////////////////////////////////
  ///  CASE 3   
  ///
  ///  Calling get_vector_by_reference_clear() with 1 Million time at each 
  ///  iteration and increasing the number of items by 1000.
  ///
  ///  In this case the "std::vector<int> out" instances is outside the scope 
  ///  of the for() loop, so it will be created and deleted only once for each
  ///  increment of _items and the get_vector_by_reference_clear() will be in 
  ///  charge to call clear() on the vector.
  /// 
  std::cout << std::endl << "reference + clear" << std::endl;
  {
    for ( int _items = 1000; _items <= 10000; _items += 1000 )
    {
      /// Read the clock
      auto time_start = chrono::steady_clock::now();

      {
        std::vector<int> out;    
        for ( size_t _counter = 0; _counter < _nRepeat; ++_counter )
        {
          get_vector_by_reference_clear( _items,  out );
        }
      }

      /// Read the clock
      auto time_end = chrono::steady_clock::now();

      std::cout << _items << "," <<  get_duration_ms(time_start,time_end) << std::endl;
    }
  }
}

int main()
{
  compare_copy_elision();

  return 0;
}

