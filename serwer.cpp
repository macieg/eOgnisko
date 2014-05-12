#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include "err.h"
#include <boost/program_options.hpp>
namespace po = boost::program_options;

int port = 3856;
int fifo_size = 10560;
int fifo_low_watermark = 0;
int fifo_high_watermark = fifo_size;
int buf_len = 10;
int tx_interval = 5;

//Zmienna pomocnicza do obsługi sygnałów.
struct sigaction sigint_action;

//Informacja o tym, czy złapano sygnał SIG_INT.
bool is_stopped;

//Metoda obsługująca przechwycenie sygnału SIG_INT.
void sigint_handler(int i)
{
	is_stopped = true;
	fprintf(stderr, "Przechwycono SIG_INT %d", i);
}

//Metoda służąca do ustawienia startowych opcji w programie.
void setup(int argc, char** argv)
{
	//Ustawienia dotyczące wyłapywania sygnałów.
	is_stopped = false;

	sigint_action.sa_handler = sigint_handler;
	sigemptyset(&sigint_action.sa_mask);
	sigint_action.sa_flags = SA_RESTART;

	if (sigaction(SIGINT, &sigint_action, 0) == -1)
		syserr("Error in signal\n");

	//Parsowanie argumentów programu.
	po::options_description description("Server usage");
	
	description.add_options()
		("help,h", "Distplay this help message")
		("port,p", po::value<int>(), "Port number")
		("fifosize,F", po::value<int>(), "Fifo size")
		("fifolowwatermark,L", po::value<int>(), "Fifo low watermark")
		("fifohighwatermark,H", po::value<int>(), "Fifo high watermark")
		("buflen,X", po::value<int>(), "Buffer length")
		("txinterval,i", po::value<int>(), "Tx interval");

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
	po::notify(vm);

	if(vm.count("help"))
	{
		std::cerr << description;
	}

	if(vm.count("port"))
	{
		port = vm["port"].as<int>();
		fprintf(stderr, "Port number - %d\n", port);
	}

	if(vm.count("fifosize"))
	{
		fifo_size = vm["fifosize"].as<int>();
		fprintf(stderr, "Fifo size - %d\n", fifo_size);
	}

	if(vm.count("fifolowwatermark"))
	{
		fifo_low_watermark = vm["fifolowwatermark"].as<int>();
		fprintf(stderr, "Fifo low watermark - %d\n", fifo_low_watermark);
	}

	if(vm.count("fifohighwatermark"))
	{
		fifo_high_watermark = vm["fifohighwatermark"].as<int>();
		fprintf(stderr, "Fifo high watermark - %d\n", fifo_high_watermark);
	}

	if(vm.count("buflen"))
	{
		buf_len = vm["buflen"].as<int>();
		fprintf(stderr, "Buffer length - %d\n", buf_len);
	}

	if(vm.count("txinterval"))
	{
		tx_interval = vm["txinterval"].as<int>();
		fprintf(stderr, "Tx interval - %d\n", tx_interval);
	}
}

int main(int argc, char** argv)
{
	setup(argc, argv);
	
	//while(!is_stopped)
	//{
	//	fprintf(stderr, "Serwer sobie działa\n");
	//	sleep(1);
	//}
}
