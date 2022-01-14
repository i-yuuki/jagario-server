#include "winsock-error.h"

#include <WinSock2.h>

std::string getLastWinSockErrorMessage(){
	return getWinSockErrorMessage(WSAGetLastError());
}

std::string getWinSockErrorMessage(int code){
	#define CASE(c, msg) case c: return msg " (" #c ")";
	switch(code){
	CASE(WSANOTINITIALISED, "A successful WSAStartup must occur before using this function.")
	CASE(WSAENETDOWN,	"The Windows Sockets implementation has detected that the network subsystem has failed.")
	CASE(WSAENOTCONN,	"The socket is not connected.")
	CASE(WSAEINTR,	"The (blocking) call was canceled using WSACancelBlockingCall.")
	CASE(WSAEINPROGRESS,	"A blocking Windows Sockets operation is in progress.")
	CASE(WSAENOTSOCK,	"The descriptor is not a socket.")
	CASE(WSAEOPNOTSUPP,	"MSG_OOB was specified, but the socket is not of type SOCK_STREAM.")
	CASE(WSAESHUTDOWN,	"The socket has been shut down; it is not possible to recv on a socket after shutdown has been invoked with how set to 0 or 2.")
	CASE(WSAEWOULDBLOCK,	"The socket is marked as nonblocking and the receive operation would block.")
	CASE(WSAEMSGSIZE,	"The datagram was too large to fit into the specified buffer and was truncated.")
	CASE(WSAEINVAL,	"The socket has not been bound with bind.")
	CASE(WSAECONNABORTED,	"The virtual circuit was aborted due to timeout or other failure.")
	CASE(WSAECONNRESET,	"The virtual circuit was reset by the remote side.")
	CASE(WSAEAFNOSUPPORT,	"The specified address family is not supported.")
	CASE(WSAEMFILE,	"No more file descriptors are available.")
	CASE(WSAENOBUFS,	"No buffer space is available. The socket cannot be created.")
	CASE(WSAEPROTONOSUPPORT,	"The specified protocol is not supported.")
	CASE(WSAEPROTOTYPE,	"The specified protocol is the wrong type for this socket.")
	CASE(WSAESOCKTNOSUPPORT,	"The specified socket type is not supported in this address family.")
	CASE(WSAEADDRINUSE, 	"The specified address is already in use. (See the SO_REUSEADDR socket option under setsockopt.)")
	CASE(WSAEFAULT,	"The namelen argument is too small (less than the size of a struct sockaddr).")
	CASE(WSAEADDRNOTAVAIL,	"The specified address is not available from the local computer.")
	CASE(WSAECONNREFUSED,	"The attempt to connect was forcefully rejected.")
	CASE(WSAEDESTADDRREQ,	"A destination address is required.")
	CASE(WSAEISCONN,	"The socket is already connected.")
	CASE(WSAENETUNREACH,	"The network can't be reached from this host at this time.")
	CASE(WSAETIMEDOUT,	"Attempt to connect timed out without establishing a connection")
	CASE(WSAEACCES,	"The requested address is a broadcast address, but the appropriate flag was not set.")
	CASE(WSAENETRESET,	"The connection must be reset because the Windows Sockets implementation dropped it.")
	default: return "Unknown WinSock error code";
	}
	#undef CASE
}
