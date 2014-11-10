#include <gnt_socket.h>

#include <stdint.h>
#include <string.h>
#include <assert.h>

#if defined (_WIN32) || defined (_WIN64)
#pragma comment(lib, "Ws2_32.lib")
#include <Ws2tcpip.h>
#pragma warning(disable:4018)

int inet_aton(const char* cp, struct in_addr* inp)
{
	unsigned long ulAddr;

	ulAddr	=	inet_addr(cp);

	if (INADDR_NONE == ulAddr)
	{
		inp->S_un.S_addr	=	ulAddr;
		return 0;
	}

	inp->S_un.S_addr	=	ulAddr;

	return 1;
}

#if _MSC_VER < 1300
const char *inet_ntop(int af, const void *src,
						  char *dst, socklen_t cnt)
{
	return ""; //暂不支持
}
#elif _MSC_VER < 1500
const char *inet_ntop(int af, const void *src,
						  char *dst, socklen_t cnt)
{
	char serv[NI_MAXSERV];
	int iRet = 0;


	if ((NULL == src) ||(NULL == dst) ||(0 >= cnt))
	{
		return NULL;
	}


	*dst = '\0';
	getnameinfo( (struct sockaddr *)src, sizeof( struct sockaddr_in ),
		dst, cnt, serv, sizeof( serv ), NI_NUMERICHOST  );
	return dst;
}
#endif

#if _MSC_VER >= 1300
const char *gnt_inet_ntop(int af, const void *src,
                          char *dst, int cnt)
{
    struct in_addr *pstAddr;
    int iWriteLen;

    if ((NULL == src) ||(NULL == dst) ||(0 >= cnt) || (AF_INET != af))
    {
        return NULL;
    }

    pstAddr = (struct in_addr *)src;

    iWriteLen = snprintf(dst, cnt, "%u.%u.%u.%u",
        pstAddr->s_net,pstAddr->s_host,
        pstAddr->s_lh,pstAddr->s_impno);
    if ((0 > iWriteLen) || (iWriteLen >= cnt))
    {
        return NULL;
    }
    return dst;
}
#endif

/*
 * Internet network address interpretation routine.
 * The library routines call this routine to interpret
 * network numbers.
 */
in_addr_t inet_network(const char *cp)
{
	register in_addr_t val, base, n, i;
	register char c;
	in_addr_t parts[4], *pp = parts;
	int digit;

again:
	val = 0; base = 10; digit = 0;
	if (*cp == '0')
		digit = 1, base = 8, cp++;
	if (*cp == 'x' || *cp == 'X')
		base = 16, cp++;
	while ((c = *cp) != 0) {
		if (isdigit(c)) {
			if (base == 8 && (c == '8' || c == '9'))
				return (INADDR_NONE);
			val = (val * base) + (c - '0');
			cp++;
			digit = 1;
			continue;
		}
		if (base == 16 && isxdigit(c)) {
			val = (val << 4) + (tolower (c) + 10 - 'a');
			cp++;
			digit = 1;
			continue;
		}
		break;
	}
	if (!digit)
		return (INADDR_NONE);
	if (pp >= parts + 4 || val > 0xff)
		return (INADDR_NONE);
	if (*cp == '.') {
		*pp++ = val, cp++;
		goto again;
	}
	if (*cp && !isspace(*cp))
		return (INADDR_NONE);
	if (pp >= parts + 4 || val > 0xff)
		return (INADDR_NONE);
	*pp++ = val;
	n = (in_addr_t)(pp - parts);
	for (val = 0, i = 0; i < n; i++) {
		val <<= 8;
		val |= parts[i] & 0xff;
	}
	return (val);
}


/*
 * Formulate an Internet address from network + host.  Used in
 * building addresses stored in the ifnet structure.
 */
struct in_addr inet_makeaddr(in_addr_t net, in_addr_t host)
{
	struct in_addr in;

	if (net < 128)
		in.s_addr = (net << IN_CLASSA_NSHIFT) | (host & IN_CLASSA_HOST);
	else if (net < 65536)
		in.s_addr = (net << IN_CLASSB_NSHIFT) | (host & IN_CLASSB_HOST);
	else if (net < 16777216L)
		in.s_addr = (net << IN_CLASSC_NSHIFT) | (host & IN_CLASSC_HOST);
	else
		in.s_addr = net | host;

	in.s_addr = htonl(in.s_addr);

	return in;
}



/*
 * Return the network number from an internet
 * address; handles class a/b/c network #'s.
 */
in_addr_t inet_netof(struct in_addr in)
{
	register unsigned long i = ntohl(in.s_addr);

	if (IN_CLASSA(i))
		return (((i)&IN_CLASSA_NET) >> IN_CLASSA_NSHIFT);
	else if (IN_CLASSB(i))
		return (((i)&IN_CLASSB_NET) >> IN_CLASSB_NSHIFT);
	else
		return (((i)&IN_CLASSC_NET) >> IN_CLASSC_NSHIFT);
}


/*
 * Return the local network address portion of an
 * internet address; handles class a/b/c network
 * number formats.
 */
in_addr_t inet_lnaof(struct in_addr in)
{
	register uint32_t i = ntohl(in.s_addr);

	if (IN_CLASSA(i))
		return ((i)&IN_CLASSA_HOST);
	else if (IN_CLASSB(i))
		return ((i)&IN_CLASSB_HOST);
	else
		return ((i)&IN_CLASSC_HOST);
}


#else
#if 0
const char *gnt_inet_ntop(int af, const void *src,
                          char *dst, int cnt)
{
    struct in_addr *pstAddr;
    int iWriteLen;
	char *buff;
    if ((NULL == src) ||(NULL == dst) ||(0 >= cnt) || (AF_INET != af))
    {
        return NULL;
    }



    pstAddr = (struct in_addr *)src;
	buff = inet_ntoa(*pstAddr);

	if(buff != NULL)
	{
		strncpy(dst,buff,cnt);
	}

    return dst;
}
#endif

#endif

int gnt_socket_init(int a_iMajor, int a_iMinor)
{

#if defined (_WIN32) || defined (_WIN64)

	WSADATA stWsa;
	int iRet;
	WORD wVer;

	wVer	=	MAKEWORD(a_iMajor, a_iMinor);

	iRet	=	WSAStartup(wVer, &stWsa);

	return iRet;

#else
	void a_iMajor;
	void a_iMinor;
return 0;
#endif
}

int gnt_socket_fini(void)
{
#if defined (_WIN32) || defined (_WIN64)
	WSACleanup();
	return 0;
#else
	return 0;
#endif
}

GSOCKET gnt_socket_open(int a_iAf, int a_iType, int a_iProto)
{
	return socket(a_iAf, a_iType, a_iProto);
}

GSOCKET gnt_socket_open_tcp(void)
{
	return gnt_socket_open(PF_INET, SOCK_STREAM, 0);
}

GSOCKET gnt_socket_open_udp(void)
{
	return gnt_socket_open(PF_INET, SOCK_DGRAM, 0);
}

GSOCKET gnt_socket_open_unix(int a_iType)
{
	return gnt_socket_open(PF_UNIX, a_iType, 0);
}

int gnt_socket_reuse_addr(GSOCKET a_iSock, int a_iIsReuseAddr)
{
	return setsockopt(a_iSock, SOL_SOCKET, SO_REUSEADDR, (char*)&a_iIsReuseAddr, sizeof(a_iIsReuseAddr));
}

int gnt_socket_set_nonblock(GSOCKET a_iSock, int a_iIsNonblocking)
{
#if defined (_WIN32) || defined (_WIN64)
	unsigned long lParam;

	lParam	=	a_iIsNonblocking;

	return ioctlsocket(a_iSock, FIONBIO, &lParam);

#else
	int iFlags;

	iFlags = fcntl(a_iSock, F_GETFL, 0);

	if( a_iIsNonblocking )
		iFlags	|=	O_NONBLOCK | O_ASYNC;
	else
		iFlags	&=	~(O_NONBLOCK | O_ASYNC);

	return fcntl(a_iSock, F_SETFL, iFlags);
#endif
}

int gnt_socket_listen(GSOCKET a_iSock, struct sockaddr* a_pstAddr, int a_iLen, int a_iBacklog)
{
	if( bind(a_iSock, a_pstAddr, a_iLen)<0 )
		return -1;

	return listen(a_iSock, a_iBacklog);
}

int gnt_socket_start_connect(GSOCKET a_iSock, struct sockaddr* a_pstAddr, int a_iLen)
{
	int iErr;

	if( 0==connect(a_iSock, a_pstAddr, a_iLen) )
		return 0;

#if defined (_WIN32) || defined (_WIN64)
	iErr	=	WSAGetLastError();

	if( WSAEISCONN==iErr )
		return 0;
	else if( WSAEWOULDBLOCK==iErr || WSAEALREADY==iErr )
		return GSOCKET_INPROGRESS;
	else
		return -1;
#else
	iErr	=	errno;

	if( EISCONN==iErr )
		return 0;
	else if( EINPROGRESS==iErr || EALREADY==iErr )
		return GSOCKET_INPROGRESS;
	else
		return -1;
#endif
}

#if !defined (_WIN32) && !defined (_WIN64)
#include <sys/epoll.h>
int gnt_socket_epoll_check(GSOCKET a_iSock, int a_iEvent, int a_iTimeout)
{
    int iRet;
    int iEpollFd;
    struct epoll_event e ;
    struct epoll_event *events = NULL;

    iEpollFd = epoll_create(a_iSock + 1);
    if (0 > iEpollFd)
    {
        return -1;
    }

    memset(&e, 0, sizeof(e));
    e.events = a_iEvent;
    e.data.fd = a_iSock;

    iRet = epoll_ctl(iEpollFd, EPOLL_CTL_ADD, a_iSock, &e);
    if (0 == iRet)
    {
        events = (struct epoll_event*)malloc(sizeof(struct epoll_event)*(a_iSock+1));
        if (NULL == events)
        {
            iRet = -2;
        }
    }

    if (0 == iRet)
    {
        iRet = epoll_wait(iEpollFd, events, a_iSock+1, a_iTimeout);
        if (0 > iRet)
        {
            iRet = -3;
        } else if (0 == iRet)
        {
            iRet = TSOCKET_TIMEOUT;
        } else
        {
            iRet = 0;	//have ready
        }
    }

    if (NULL != events)
        free(events);

    close(iEpollFd);

    return iRet;
}
#endif

int gnt_socket_check_connect(GSOCKET a_iSock, int a_iTimeout)
{
	fd_set wrfs;
	struct timeval tv;
	int iRet;

#if !defined (_WIN32) && !defined (_WIN64)
	int iError;
	socklen_t iLen;
#else
	fd_set exceptfs;
#endif

	MS_TO_TV(tv, a_iTimeout);

#if defined (_WIN32) || defined (_WIN64)
    {
        FD_ZERO(&wrfs);
        FD_SET(a_iSock, &wrfs);

        FD_ZERO(&exceptfs);
        FD_SET(a_iSock, &exceptfs);

        if (GSOCKET_WAIT_INFINITE==a_iTimeout)
            iRet    =       select((int)(a_iSock+1), NULL, &wrfs, &exceptfs, NULL);
        else
            iRet    =       select((int)(a_iSock+1), NULL, &wrfs, &exceptfs, &tv);

        if (0 > iRet)
            return -1;
        else if(0 == iRet)
            return GSOCKET_TIMEOUT;

        if (FD_ISSET(a_iSock, &exceptfs))
            return -1;
        else
            return 0;
    }
#else
    {
        if (a_iSock < FD_SETSIZE)
        {
            FD_ZERO(&wrfs);
            FD_SET(a_iSock, &wrfs);

            if ( GSOCKET_WAIT_INFINITE==a_iTimeout )
                iRet    =       select(a_iSock+1, NULL, &wrfs, NULL, NULL);
            else
                iRet    =       select(a_iSock+1, NULL, &wrfs, NULL, &tv);

            //assert(iRet >=0);
            if (0 > iRet)
                return -1;
            else if (0 == iRet)
                return GSOCKET_TIMEOUT;
        } else
        {
            iRet = gnt_socket_epoll_check(a_iSock, EPOLLOUT, a_iTimeout);
            if (0 != iRet)
                return iRet;
        }

        iError  =       0;
        iLen    =       sizeof(iLen);

        getsockopt(a_iSock, SOL_SOCKET, SO_ERROR, (char*)&iError, &iLen);

        if( 0==iError )
            return 0;
        else
            return -1;
    }
#endif
}

int gnt_socket_connect(GSOCKET a_iSock, struct sockaddr* a_pstAddr, int a_iLen, int a_iTimeout)
{
	int iErr;

	iErr	=	gnt_socket_start_connect(a_iSock, a_pstAddr, a_iLen);

	if( 0==iErr )
		return 0;
	else if( -1==iErr )
		return -1;

	iErr	=	gnt_socket_check_connect(a_iSock, a_iTimeout);

	if( 0!=iErr )
		return -1;
	else
		return 0;
}

GSOCKET gnt_socket_accept(GSOCKET a_iSock, struct sockaddr* a_pstAddr, int* a_piLen, int a_iTimeout)
{
	fd_set rdfs;
	struct timeval tv;
	int iRet;

#if !defined(_WIN32) && !defined(_WIN64)
    if (a_iSock < FD_SETSIZE)
#endif
    {
        FD_ZERO(&rdfs);
        FD_SET(a_iSock, &rdfs);

        MS_TO_TV(tv, a_iTimeout);

        if( GSOCKET_WAIT_INFINITE==a_iTimeout )
            iRet    =       select((int)(a_iSock+1), &rdfs, NULL, NULL, NULL);
        else
            iRet    =       select((int)(a_iSock+1), &rdfs, NULL, NULL, &tv);

        if( iRet<=0 )
            return (GSOCKET)-1;
    }
#if !defined(_WIN32) && !defined(_WIN64)
    else
    {
        iRet = gnt_socket_epoll_check(a_iSock, EPOLLIN, a_iTimeout);
        if (0 != iRet)
            return (TSOCKET)-1;
    }
#endif

	return accept(a_iSock, a_pstAddr, (socklen_t*)a_piLen);
}

int gnt_socket_close(GSOCKET a_iSock)
{
#if defined (_WIN32) || defined (_WIN64)
	return closesocket(a_iSock);
#else
	return close(a_iSock);
#endif
}

int gnt_socket_send(GSOCKET a_iSock, const char* a_pszBuff, int a_iLen, int a_iTimeout)
{
	int iRet;
	struct timeval tv;
	struct timeval* pstTV;


#if !defined(_WIN32) && !defined(_WIN64)
	if(a_iTimeout && a_iSock < FD_SETSIZE)
#else
	if(a_iTimeout)
#endif
	{
        fd_set  fds;

        FD_ZERO(&fds);
        FD_SET(a_iSock, &fds);

		pstTV	=	&tv;

		if( GSOCKET_WAIT_INFINITE==a_iTimeout )
			pstTV	=	NULL;
		else
			MS_TO_TV(tv, a_iTimeout);

		iRet		=	select( (int)(a_iSock+1), NULL, &fds, NULL, pstTV );

		if( iRet<0 )
			return -1;

		if( 0==iRet )
			return 0;
	}

	iRet	=	send(a_iSock, a_pszBuff, a_iLen, 0);

	if( iRet<0 )
	{
		if( SOCKETEWOULDBLOCK==socketerrno || SOCKETEAGAIN==socketerrno )
		{
			if( a_iTimeout )
				usleep(200000);

			return 0;
		}

		return -2;
	}
	else if( 0==iRet )
		return -3;
	else
		return iRet;
}

int gnt_socket_sendall(GSOCKET a_iSock, const char* a_pszBuff, int a_iLen, int a_iTimeout)
{
    int iTotal;
    int iRet;
    struct timeval tv;
    struct timeval* pstTV;
    int iTime = 0;

    fd_set  fds;

    iTotal	=	0;
    pstTV	=	&tv;

    if( 0 == a_iTimeout )
    {
        /*不等待*/
        iRet = gnt_socket_send(a_iSock,a_pszBuff,a_iLen,a_iTimeout);
        return iRet;
    }

    if( GSOCKET_WAIT_INFINITE==a_iTimeout )
        MS_TO_TV(tv, 500);
    else
        MS_TO_TV(tv, a_iTimeout);

    while( iTotal<a_iLen )
    {
#if !defined(_WIN32) && !defined(_WIN64)
        if (a_iSock < FD_SETSIZE)
#endif
        {
            FD_ZERO(&fds);
            FD_SET(a_iSock, &fds);
            iRet		=	select( (int)(a_iSock+1), NULL, &fds, NULL, pstTV );

            if( iRet<0 )
                return -1;

            if( 0==iRet && GSOCKET_WAIT_INFINITE!=a_iTimeout )
                return iTotal;
        }
#if !defined(_WIN32) && !defined(_WIN64)
        else
        {
            if ((TSOCKET_WAIT_INFINITE != a_iTimeout) && (iTime >= a_iTimeout))
                return iTotal;
        }
#endif

        iRet	=	send(a_iSock, a_pszBuff + iTotal, a_iLen - iTotal, 0);

        if(iRet < 0)
        {
            if( SOCKETEWOULDBLOCK==socketerrno || SOCKETEAGAIN==socketerrno )
            {
                usleep(20000);
                iTime += 20000 /1000;
                continue;
            }

            return -2;
        }


        if( 0==iRet )
        {
            if( 0==iTotal )
                return -3;
            else
                return iTotal;
        }

        iTotal +=	iRet;
    }

    return iTotal;
}

#if !defined(_WIN32) && !defined(_WIN64) // for linux
int gnt_socket_recv(GSOCKET a_iSock, char* a_pszBuff, int a_iLen, int a_iTimeout)
{
	int iRet;
	struct timeval tv;
	struct timeval* pstTV;

	if(a_iTimeout)
    {
        if (a_iSock < FD_SETSIZE)
        {
            fd_set  fds;

            FD_ZERO(&fds);
            FD_SET(a_iSock, &fds);

            pstTV	=	&tv;
            if( GSOCKET_WAIT_INFINITE==a_iTimeout )
                pstTV	=	NULL;
            else
                MS_TO_TV(tv, a_iTimeout);

            iRet		=	select( (int)(a_iSock+1), &fds, NULL, NULL, pstTV );

            if( iRet<0 )
            {
                if (SOCKETINTR == socketerrno)
                {
                    return 0;
                }
                return -1;
            }
            if( 0==iRet )
                return 0;
        }
        else
        {
            iRet = gnt_socket_epoll_check(a_iSock, EPOLLIN, a_iTimeout);
            if (0 != iRet)
                return iRet;
        }
    }

	iRet	=	recv(a_iSock, a_pszBuff, a_iLen, 0);

	if( iRet<0 )
	{
		if( SOCKETEWOULDBLOCK==socketerrno || SOCKETEAGAIN==socketerrno )
		{
			if( a_iTimeout )
				usleep(100000);
			return 0;
		}
		return -2;
	}
	else if( 0==iRet )
		return -3;
	else
		return iRet;
}
#else // for windows
int gnt_socket_recv(GSOCKET a_iSock, char* a_pszBuff, int a_iLen, int a_iTimeout)
{
	int iRet;
	struct timeval tv;
	struct timeval* pstTV;

	if(a_iTimeout)
	{
        fd_set  fds;

        FD_ZERO(&fds);
        FD_SET(a_iSock, &fds);

		pstTV	=	&tv;

		if( GSOCKET_WAIT_INFINITE==a_iTimeout )
			pstTV	=	NULL;
		else
			MS_TO_TV(tv, a_iTimeout);

		iRet		=	select( (int)(a_iSock+1), &fds, NULL, NULL, pstTV );

		if( iRet<0 )
		{
			if (SOCKETINTR == socketerrno)
			{
				return 0;
			}

			return -1;
		}

		if( 0==iRet )
			return 0;
	}


	iRet	=	recv(a_iSock, a_pszBuff, a_iLen, 0);

	if( iRet<0 )
	{
		if( SOCKETEWOULDBLOCK==socketerrno || SOCKETEAGAIN==socketerrno )
		{
			if( a_iTimeout )
				usleep(100000);

			return 0;
		}

		return -2;
	}
	else if( 0==iRet )
		return -3;
	else
		return iRet;
}
#endif

int gnt_socket_recvall(GSOCKET a_iSock, char* a_pszBuff, int a_iLen, int a_iTimeout)
{
	int iTotal;
	int iRet;
	struct timeval tv;
	struct timeval* pstTV;
    int iTime = 0;

	fd_set  fds;

	iTotal	=	0;
	pstTV	=	&tv;

	if( GSOCKET_WAIT_INFINITE==a_iTimeout )
		pstTV	=	NULL;
	else
		MS_TO_TV(tv, a_iTimeout);

	while( iTotal<a_iLen )
	{
#if !defined(_WIN32) && !defined(_WIN64)
        if (a_iSock < FD_SETSIZE)
#endif
        {
            FD_ZERO(&fds);
            FD_SET(a_iSock, &fds);
            iRet		=	select( (int)(a_iSock+1), &fds, NULL, NULL, pstTV );

            if( iRet<0 )
            {
                if (SOCKETINTR == socketerrno)
                {
                    return iTotal;
                }

                return -1;
            }

            if( 0==iRet )
                return iTotal;
        }
#if !defined(_WIN32) && !defined(_WIN64)
        else
        {
            if ((TSOCKET_WAIT_INFINITE != a_iTimeout) && (iTime >= a_iTimeout))
                return iTotal;
        }
#endif


		iRet	=	recv(a_iSock, a_pszBuff + iTotal, a_iLen - iTotal, 0);

		if( iRet<0 )
		{
			if( SOCKETEWOULDBLOCK==socketerrno || SOCKETEAGAIN==socketerrno )
			{
				usleep(100000);
                iTime += 100000 /1000;
				continue;
			}

			return -1;
		}


		if( 0==iRet )
		{
			if( 0==iTotal )
				return -1;
			else
				return iTotal;
		}

		iTotal +=	iRet;
	}

	return iTotal;
}


int gnt_socket_set_recvbuff(GSOCKET a_iSock, int a_iSize)
{
	return setsockopt(a_iSock, SOL_SOCKET, SO_RCVBUF, (void*)&a_iSize, sizeof(a_iSize));
}

int gnt_socket_set_sendbuff(GSOCKET a_iSock, int a_iSize)
{
	return setsockopt(a_iSock, SOL_SOCKET, SO_SNDBUF, (void*)&a_iSize, sizeof(a_iSize));
}

int gnt_socket_get_type(GSOCKET a_iSock, int* a_piType)
{
	socklen_t iSize;

	iSize	=	sizeof(*a_piType);
	return getsockopt(a_iSock, SOL_SOCKET, SO_TYPE, (void*)a_piType, &iSize);
}

int gnt_socket_get_acceptconn(GSOCKET a_iSock, int* a_piAcceptConn)
{
	socklen_t iSize;

	iSize	=	sizeof(*a_piAcceptConn);
	return getsockopt(a_iSock, SOL_SOCKET, SO_ACCEPTCONN, (void*)a_piAcceptConn, &iSize);
}


