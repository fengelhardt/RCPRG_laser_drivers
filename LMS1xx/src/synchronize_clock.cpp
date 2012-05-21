/*
 * synchronize_clock.cpp
 *
 *  Created on: 05-21-2012
 *      Author: Srećko Jurić-Kavelj <srecko.juric-kavelj@fer.hr>
 */

#include <boost/program_options.hpp>
#include <LMS1xx.h>
#include <string>
#include <iostream>
#include <stdio.h>

namespace po = boost::program_options;
using std::string;
using std::cout;
using std::endl;

int main(int argc, char **argv)
{
	string host;
	double alpha, threshold;
	po::options_description desc(
		"synchronize_clock synchronizes SICK LMS1xx clock with host system clock.\n\n"
		"Command line arguments");
	desc.add_options()("help,h", "Produce this help message.")
		("device-ip,i", po::value<string>(&host)->default_value("192.168.0.1"), "Device IP address")
		("alpha,a", po::value<double>(&alpha)->default_value(0.1), "Filter alpha parameter")
		("threshold,t", po::value<double>(&threshold)->default_value(1e-7), "Filter threshold")
		("get-tcp_info", "Get tcp_info struct and print RTT values");
	
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	
	if (vm.count("help"))
	{
		cout << desc << "\n";
		exit(0);
	}
	
	LMS1xx laser;
	laser.connect(host);
	if (!laser.isConnected())
	{
		cout << "Unable to connect to device at address: " << host << endl;
		exit(1);
	}
	laser.login();
	cout << "Estimating RTT between the host PC and the LMS." << endl;
	
	struct tcp_info tcp_info;
	struct tcp_info *tcp_info_ptr = NULL;
	if (vm.count("get-tcp_info"))
		tcp_info_ptr = &tcp_info;

	double rtt = laser.estimateRoundTripTime(tcp_info_ptr, alpha, threshold);
	cout << "Estimated RTT: " << rtt*1e3 << " ms." << endl;
	if (tcp_info_ptr != NULL)
	{
		cout << "tcp_info struct stats:" << endl
			<< "  RTT: " << tcp_info.tcpi_rtt << endl
			<< "  RTT variance: " << tcp_info.tcpi_rttvar << endl;
	}
	
	cout << "Synchronizing the device clock..." << endl;
	while (!laser.setTime(rtt));
	cout << "Success!" << endl;
	
	laser.startDevice();
	laser.disconnect();
	
	return 0;
}
