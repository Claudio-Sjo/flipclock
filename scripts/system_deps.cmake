# This file contains the system specific stuff, i.e. paths or maybe commands, env vars etc.
# It is at the moment not needed - no system dependencies (clang must be in the PATH).

# MCUxpresso tools path
if( CMAKE_HOST_SYSTEM_NAME MATCHES Windows )
    # Tools path for Windows
    # set(MCUX_PREFIX c:/nxp/mcuxpressoide_11.3.0_5222/ide/tools)
elseif( CMAKE_HOST_SYSTEM_NAME MATCHES Darwin )
    # Tools path for Mac seems Ok
    # set(MCUX_PREFIX /Applications/MCUXpressoIDE_11.3.0_5222/ide/tools)
elseif( CMAKE_HOST_SYSTEM_NAME MATCHES Linux )
    # Tools path for Linux(?)
    # set(MCUX_PREFIX /usr/local/MCUXpressoIDE_11.3.0_5222/ide/tools)
else()
    message( FATAL_ERROR "Unknown operating system")
endif()
