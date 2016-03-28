// The Art of C++ / PostgreSQL
// Copyright (c) 2016 Daniel Frey

#ifndef TAOCPP_INCLUDE_POSTGRES_CONNECTION_HPP
#define TAOCPP_INCLUDE_POSTGRES_CONNECTION_HPP

#include <memory>
#include <unordered_set>
#include <string>
#include <libpq-fe.h>
#include <tao/postgres/transaction.hpp>
#include <tao/postgres/result.hpp>

namespace tao
{
  namespace postgres
  {
    class table_writer;

    class connection final
      : public std::enable_shared_from_this< connection >
    {
    private:
      friend class postgres::transaction;
      friend class table_writer;

      const std::unique_ptr< ::PGconn, void(*)( ::PGconn* ) > pgconn_;
      postgres::transaction* current_transaction_;
      std::unordered_set< std::string > prepared_statements_;

      std::string error_message() const;
      void check_prepared_name( const std::string& name ) const;
      bool is_prepared( const char* name ) const;

      result execute_params( const char* statement, const int n_params, const char* const param_values[] );

    public:
      static std::shared_ptr< connection > create( const std::string& connect_info );

    private:
      // pass-key idiom
      class private_key
      {
        friend std::shared_ptr< connection > connection::create( const std::string& connect_info );
      };

    public:
      // the ctor is public for technical reasons, the user is *not*
      // supposed to call it directly. use connection::create() instead.
      connection( const private_key&, const std::string& connect_info );

      connection( const connection& ) = delete;
      void operator=( const connection& ) = delete;

      bool is_open() const;

      void prepare( const std::string& name, const std::string& statement );
      void deallocate( const std::string& name );

      std::shared_ptr< postgres::transaction > direct();
      std::shared_ptr< postgres::transaction > transaction( const transaction::isolation_level isolation_level = transaction::isolation_level::DEFAULT );

      template< typename... Ts >
      result execute( Ts&&... ts )
      {
        return direct()->execute( std::forward< Ts >( ts )... );
      }

      ::PGconn* underlying_raw_ptr()
      {
        return pgconn_.get();
      }

      const ::PGconn* underlying_raw_ptr() const
      {
        return pgconn_.get();
      }
    };
  }
}

#endif // TAOCPP_INCLUDE_POSTGRES_CONNECTION_HPP
