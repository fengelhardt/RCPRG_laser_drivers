/*
 * setIP.cpp
 *
 *  Created on: 05-20-2012
 *      Author: Srećko Jurić-Kavelj <srecko.juric-kavelj@fer.hr>
 */

#include <boost/program_options.hpp>
#include <LMS1xx.h>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <stdio.h>

namespace po = boost::program_options;
using std::string;
using std::cout;
using std::endl;

int main(int argc, char **argv)
{
	string host, new_ip_str;
	po::options_description desc(
		"setIP is used to change the IP address of a SICK LMS1xx laser range sensor.\n\n"
		"Command line arguments:");
	desc.add_options()("help,h", "Produce this help message.")
		("current-ip,i", po::value<string>(&host)->default_value("192.168.0.1"), "Current IP address")
		("new-ip,n", po::value<string>(&new_ip_str), "New IP address");
	
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	
	if (vm.count("help") || !vm.count("new-ip"))
	{
		cout << desc << "\n";
		exit(0);
	}
	
	unsigned long new_ip;
	int s = inet_pton(AF_INET, new_ip_str.c_str(), &new_ip);
	if (s <= 0)
	{
		if (s == 0)
			std::cerr << "New IP not in presentation format" << endl;
		else
			perror("inet_pton");
		exit(1);
	}
	
	LMS1xx laser;
	laser.connect(host);
	if (!laser.isConnected())
	{
		cout << "Unable to connect to device at address: " << host << endl;
		exit(1);
	}
	laser.login();
	if (laser.setIP(new_ip))
	{
		cout << "Set IP command successfully completed." << endl;
		s = laser.reboot();
		if (s)
			cout << "Successfully rebooted the device." << endl
				<< "You can now connect to the laser using it's new IP: " << new_ip_str << endl;
		else
			std::cerr << "Device reboot attempt failed!" << endl
				<< "Current IP (" << host << ") still active." << endl;
	}
	else
		cout << "Set IP command unsuccessful." << endl;
	
	laser.disconnect();
	
	return 0;
}
