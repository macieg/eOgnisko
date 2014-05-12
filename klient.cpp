#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <string>
#include "err.h"
#include <boost/program_options.hpp>
namespace po = boost::program_options;

//Odgórny ogranicznik na numer datagramu otrzymanego od serwera.
int retransmit_limit = 10;

//Nr portu serwera.
int port;

//Nazwa serwera.
std::string server;

//Zmienna pomocnicza do obsługi sygnałów.
struct sigaction sigint_action;

//Informacja o tym, czy złapano sygnał SIG_INT.
bool is_stopped;

//Metoda obsługująca przechwycenie sygnału SIG_INT.
void sigint_handler(int i)
{
	is_stopped = true;
	fprintf(stderr, "Przechwycono SIG_INT %d\n", i);	
}

//Metoda służąca do ustawienia startowych opcji w programie.
void setup(int argc, char **argv)
{
	//Ustawienia dotyczące wyłapywania sygnałów.
	is_stopped = false;

	sigint_action.sa_handler = sigint_handler;
	sigemptyset(&sigint_action.sa_mask);
	sigint_action.sa_flags = SA_RESTART;

	if (sigaction(SIGINT, &sigint_action, 0) == -1)
		syserr("Error in signal\n");

	//Parsowanie argumentów programu.
	po::options_description description("Client usage");

	description.add_options()
		("help,h", "Display this help message")
		("server,s", po::value<std::string>(),"Server name")
		("port,p", po::value<int>(), "Port number")
		("retransmit,X", po::value<int>(), "Retransmit limit");

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
	po::notify(vm);

	if(vm.count("help")) //TODO zmienic na fprintf
	{
		std::cerr << description;
	}

	if(vm.count("server")){
		server = vm["server"].as<std::string>();
		fprintf(stderr, "Server name - %s\n", server.c_str());
	}

	if(vm.count("port")){
		port = vm["port"].as<int>();
		fprintf(stderr, "Port number - %d\n", port);
	}

	if(vm.count("retransmit")){
		retransmit_limit = vm["retransmit"].as<int>();
		fprintf(stderr, "Retransmit limit - %d\n", retransmit_limit);
	}
}

int main(int argc, char** argv)
{
	setup(argc, argv);

	//while(!is_stopped)
	//{
	//	fprintf(stderr, "Klient sobie dziala\n");
	//	sleep(1);
	//}
}
