#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
using namespace std;
using namespace boost::asio;

class move_sr
{
public:
move_sr();
~move_sr();
unsigned char Get_Crc8(unsigned char *ptr, unsigned short len);
void set_base(string open_port,short int vel,short int angular);
};
move_sr::move_sr(){};
move_sr::~move_sr(){};

inline unsigned char Get_Crc8(unsigned char *ptr, unsigned short len)
	{
		unsigned char crc;
		unsigned char i;
		crc = 0;
		while(len--)
		{
			crc ^= *ptr++;
			for(i = 0; i < 8; i++)
			{
				if(crc&0x01)crc=(crc>>1)^0x8C;
				else crc >>= 1;
			}
		}
		return crc;
	}

inline void set_base(string open_port,short int vel,short int angular)
	{
		unsigned char vel_char[16];
		vel_char[0] = 0x55;
		vel_char[1] = 0xAA;
		vel_char[2] = 0xA5;
		vel_char[3] = 0x5A;
		vel_char[4] = 0x08;
		vel_char[5] = 0xf4;
		vel_char[6] = 0x01;
		vel_char[7] = 0x00;
		vel_char[8] = 0x00;
		vel_char[9] = angular&0xff;
		vel_char[10] = angular>>8;
		vel_char[11] = 0x00;
		vel_char[12] = 0x00;
		vel_char[13] = Get_Crc8(vel_char,13);
		vel_char[14] = 0x0D;
		vel_char[15] = 0x0A;
		io_service iosev;
		serial_port sp(iosev, open_port);
		sp.set_option(serial_port::baud_rate(115200));
		sp.set_option(serial_port::flow_control(serial_port::flow_control::none));
		sp.set_option(serial_port::parity(serial_port::parity::none));
		sp.set_option(serial_port::stop_bits(serial_port::stop_bits::one));
		sp.set_option(serial_port::character_size(8));
		write(sp, buffer(vel_char, 16));//写入数据
		iosev.run();
	}

