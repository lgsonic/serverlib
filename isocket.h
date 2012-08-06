#ifndef _D_ISOCKET_H_
#define _D_ISOCKET_H_

#pragma pack (1)
struct SocketClientData_t
{
public:
	unsigned int IP;
	unsigned short Port;
	int Socket;
public:
	bool operator<(const SocketClientData_t & info) const
	{
		if(IP < info.IP)
		{
			return true;
		}
		else if(IP == info.IP)
		{
			if(Port < info.Port)
			{
				return true;
			}
			else if(Port == info.Port)
			{
				return (Socket < info.Socket);
			}
		}
		return false;
	}
	bool operator==(const SocketClientData_t & info) const
	{
		return (IP == info.IP)&&(Port == info.Port)&&(Socket == info.Socket);
	}
};
#pragma pack ()

#endif
