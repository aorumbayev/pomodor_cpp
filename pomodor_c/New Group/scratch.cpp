#include <list>
#include <iostream>

/* Class to be used as a member of the delegation host.
 Keeps a list of event clients, which you can iterate over.
 As a C++11 convenience, function call operator() is overloaded to call
 all clients with the given argument list.
 
 This is templated over the base type for the clients. The base type
 should define virtual function(s) for handling the events.
 Templating is necessary because we can't anticipate the number or types
 of these virtual functions. */

template< typename delegate >
struct delegator {
    typedef std::list< delegate * > list; // host interface
    list delegates;
    
    typedef typename list::iterator delegate_id; // client interface
    
    delegate_id add( delegate *d )
    { return delegates.insert( delegates.end(), d ); }
    
    void remove( delegate_id d )
    { delegates.erase( d ); }
    
#if __cplusplus >= 201103L // C++11-only convenient host interface
    template< typename ... args >
    void operator() ( args && ... a ) {
        for ( auto d : delegates ) ( *d )( std::forward< args >( a ) ... );
    }
#endif
};

/* Abstract base class for all delegate bases. Registers and unregisters
 from the delegator, but doesn't define any event handler. */
template< typename derived >
struct delegate {
    typedef ::delegator< derived > delegator;
    
    delegator &source;
    typename delegator::delegate_id id;
    
    delegate( delegator &in_source )
    : source( in_source ),
    id( source.add( static_cast< derived * >( this ) ) ) {}
    
    virtual ~delegate() { source.remove( id ); }
};

/* Example delegate base. Defines an event handler which clients must implement.
 Other types of events might declare other bases. */
struct my_delegate_base : delegate< my_delegate_base > {
    typedef delegate< my_delegate_base > base;
    typedef base::delegator delegator;
    my_delegate_base( delegator &d ) : base( d ) {}
    
    virtual void operator() ( int ) = 0;
};

/* Example client class defines how to handle an event. */
struct my_delegate_impl : my_delegate_base {
    my_delegate_impl( delegator &d ) : my_delegate_base( d ) {}
    
    virtual void operator() ( int i ) {
        std::cout << i << '\n';
    }
};
struct my_delegate_impl2 : my_delegate_base {
    my_delegate_impl2( delegator &d ) : my_delegate_base( d ) {}
    
    virtual void operator() ( int i ) {
        std::cout << - i << '\n';
    }
};

int main() {
    typedef delegator< my_delegate_base > my_delegator;
    my_delegator distro;
    my_delegate_impl a( distro ), b( distro );
    my_delegate_impl2 c( distro );
    
    for ( my_delegator::delegate_id d = distro.delegates.begin();
         d != distro.delegates.end(); ++ d ) {
        ( **d )( 5 );
    }
}

