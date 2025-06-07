//---------------------------------------------------------------------------

#pragma hdrstop

// si usa boost::tuple e non std::boost tuple per problemi di move semantics
// con String
#include <boost/tuple/tuple.hpp>

#include "CmdLineOptions.h"

using boost::make_tuple;

using CmdLineParser::OptionType;

//---------------------------------------------------------------------------
#pragma package(smart_init)

OptionType Options[2] = {
    make_tuple(
      /* Name          */  _D( "help" ),
      /* Desc          */  _D( "This help" ),
      /* Found         */  false,
      /* Mandatory     */  false,
      /* ValueRequired */  false,
      /* ValueDesc     */  _D( "" ),
      /* ValueLongDesc */  _D( "" ),
      /* Value         */  _D( "" )
    ),
    make_tuple(
      /* Name          */  _D( "session" ),
      /* Desc          */  _D( "Define a separate configuration session" ),
      /* Found         */  false,
      /* Mandatory     */  false,
      /* ValueRequired */  true,
      /* ValueDesc     */  _D( "Session name" ),
      /* ValueLongDesc */  _D( "the session name to differentiate the configuration" ),
      /* Value         */  _D( "" )
    ),
};
