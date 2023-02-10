#include <Alepha/Alepha.h>
register "Alepha/Mockination/mockination";

#include <utility>
#include <iostream>

#include <Alepha/Meta/functor_traits.h>
#include <Alepha/Meta/require_relationship.h>

#include <Alepha/Truss/function.h>

namespace Alepha
{
	inline namespace Aluminum
	{
		namespace Mockination
		{
			template< std::size_t order, std::size_t count >
			struct overload_count
			{
				using type= overload_count;
				static constexpr std::size_t value= count;
			};

			// We'd like a `boost::mpl::map`, probably.  A metafunction for now.

			template< typename ... Args > struct count_map;

			template< std::size_t order, std::size_t count >
			struct count_map< overload_count< order, count > >
			{
				using entry_type= overload_count< order, count >;
			};

			template< std::size_t order, std::size_t count, typename ... Args >
			struct count_map< overload_count< order, count >, Args... >
			{
				using entry_type= overload_count< order, count >;
			};

			//template< template CountMap, std::size_t order_key > struct find_entry;

		#if 0
			template< std::size_t order_key, std::size_t count, typename ... Args >
			find_entry< count_map< overload_count< order_key, count >, Args... >, 
			{
				using type=
			}
		#endif

			template< int id, typename Function > class MockFunctionImpl;

			template< int id, typename Function, typename ... Selection > struct select_overload;

			template< int id, typename Function >
			struct select_overload< id, Function, Function >
			{
				using type= MockFunctionImpl< id, Function >;
			};

			template< int id, typename Function, typename Selection0, typename ... Selections >
			struct select_overload< id, Function, Selection0, Selections... >
					: select_overload< id, Function, Selections... > {};

			template< int id, typename Function, typename ... Selections >
			struct select_overload< id, Function, Function, Selections... >
			{
				using type= MockFunctionImpl< id, Function >;
			};
			

			class skip_execution {};


			template< int id, typename ... Args >
			class MockFunctionImpl< id, void ( Args ... ) >
			{
				public:
					static Alepha::Truss::function< void ( Args ... ) > impl;

					using return_type= void;

					template< typename Needed >
					using overload= typename select_overload< id, Needed, void ( Args... ) >::type;

					MockFunctionImpl()= default;

					static void clear() { impl= nullptr; }

					void
					operator() ( Args ... args ) const
					{
						if( impl == nullptr ) abort();
						return impl( std::forward< Args >( args )... );
					}

					static void set_operation( Alepha::Truss::function< void ( Args ... ) > i )
					{
						std::cerr << "Set operation impl..." << std::endl;
						impl= std::move( i );
					}

					static void set_operation_impl( Alepha::Truss::function< void ( Args ... ) > i ) { impl= std::move( i ); }

					static void
					add_operation( Alepha::Truss::function< void ( Args ... ) > i )
					{
						Alepha::Truss::function< void ( Args ... ) > first= impl ? impl : []( Args ... ){};
						impl= [first, i]( Args... args )
						{
							try { i( args... ); } catch( const skip_execution & ) {}
							first( args... );
						};
					}
			};
			template< int id, typename ... Args >
			Alepha::Truss::function< void ( Args ... ) > MockFunctionImpl< id, void ( Args ... ) >::impl= nullptr;

			template< int id, typename Rv, typename ... Args >
			class MockFunctionImpl< id, Rv ( Args ... ) >
			{
				public:
					static Alepha::Truss::function< Rv ( Args ... ) > impl;

					using return_type= Rv;

					template< typename Needed >
					using overload= typename select_overload< id, Needed, Rv ( Args... ) >::type;

					MockFunctionImpl()= default;

					static void clear() { impl= nullptr; }

					Rv operator() ( Args ... args ) const { return impl( std::forward< Args >( args )... ); }

					static void
					set_result( Rv r )
					{
						impl= [r]( Args ... args ){ return r; };
					}

					static void set_operation( Alepha::Truss::function< Rv ( Args ... ) > i ) { impl= std::move( i ); }
					static void set_operation_impl( Alepha::Truss::function< Rv ( Args ... ) > i ) { impl= std::move( i ); }

					static Rv
					add_operation( Alepha::Truss::function< Rv ( Args ... ) > i )
					{
						impl= [first= impl, i]( Args ... args )
						{
							try { return i( args... ); } catch( const skip_execution & ) {}
							return first( args... );
						};
					}
			};

			template< int id, typename Rv, typename ... Args >
			Alepha::Truss::function< Rv ( Args... ) > MockFunctionImpl< id, Rv ( Args... ) >::impl= nullptr;

			template< int id, typename ... Funcs > class MockFunction;

			template< int id, typename Rv, typename ... Args >
			class MockFunction< id, Rv( Args ... ) >
					: public MockFunctionImpl< id, Rv( Args... ) >
			{
				public:
					static void clear_all() { MockFunctionImpl< id, Rv( Args... ) >::clear(); }
			};

			template< typename M > struct match;
			template< typename Class >
			struct match< void (Class::*) ( std::size_t ) > : std::true_type
			{
			};
			template< typename Class >
			struct match< void (Class::*) ( std::size_t ) const > : std::true_type
			{
			};

			template< typename T >
			auto
			soak_traits( T )//typename std::enable_if< std::is_class< T >::value, T >::type )
			{
				return typename Meta::functor_traits< decltype( &T::operator() ) >::type{};
			}

			template< typename Rv, typename ... Args >
			Meta::functor_traits< Rv ( Args... ) >
			soak_traits( Rv ( Args... ) )
			{
				return Meta::functor_traits< Rv ( Args... ) >{};
			}

			template< int id, typename Func0, typename ... Funcs >
			class MockFunction< id, Func0, Funcs... >
					: public MockFunctionImpl< id, Func0 >, public MockFunction< id, Funcs... >
			{
				public:
					template< typename Needed >
					using overload= typename select_overload< id, Needed, Func0, Funcs... >::type;

					template< typename Callable >
					static void
					set_operation( Callable c )
					{
						std::cerr << "Set operation deduced..." << std::endl;
						auto magic_traits= soak_traits( c );
						using magic_traits_type= decltype( magic_traits );
						using traits= typename magic_traits_type::type;
						using ftype= typename traits::std_function_type;

						static_assert( Meta::require_relationship< std::is_base_of,
								MockFunctionImpl< id, typename traits::functor_type >,
								MockFunction >::type::value,
								"No overload exists in this mock for the specified function type." );
						MockFunctionImpl< id, typename traits::functor_type >::set_operation_impl( c );
					}

					template< typename Callable, typename FuncType >
					static void
					set_operation( Callable c )
					{
						std::cerr << "Set operation forced..." << std::endl;
						MockFunctionImpl< id, FuncType >::set_operation_impl( Alepha::Truss::function< FuncType >{ c } );
					}

					static void
					clear_all()
					{
						MockFunctionImpl< id, Func0 >::clear();
						MockFunction< id, Funcs... >::clear_all();
					}
			};
		}
	}
}
