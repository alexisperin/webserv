/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/17 10:24:21 by aperin            #+#    #+#             */
/*   Updated: 2023/06/11 13:16:20 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

Webserv::Webserv(void)
{
	
}

Webserv::~Webserv(void)
{
	std::list<Server *>::iterator it = this->_servers.begin();
	std::list<Server *>::iterator ite = this->_servers.end();
	for (; it != ite; it++)
		delete *it;
	this->_servers.clear();
	// std::cout << "Destructor of WebServer called." << std::endl;
}

// ************************************************************************** //
//                                  Private                                   //
// ************************************************************************** //


// ************************************************************************** //
//                                  Public                                    //
// ************************************************************************** //

void Webserv::init(std::string file_name)
{
	if (file_name.size() < 6)
		throw Webserv::InvalidFileExtensionException();
	std::string end_name = file_name.substr(file_name.size() - 5);
	if (end_name.compare(".conf"))
		throw Webserv::InvalidFileExtensionException();
	std::ifstream indata(file_name.c_str());
	if (!indata.is_open())
		throw Webserv::InvalidFileException();
	std::string line;
	bool in_server_block = false;
	while (!indata.eof())
	{
		std::getline(indata, line);
		// std::cout << "line: " << line << std::endl;
		line = trim_spaces(line);
		if (!line.empty())
		{
					// std::cout << in_server_block << "comp :" << !line.compare("server {") << ", line: |" << line << '|' << std::endl;
			if (!in_server_block && !line.compare("server {"))
			{
				in_server_block = true;
				this->_servers.push_back(new Server());
			}
			else if (in_server_block)
			{
				if (!line.compare("}"))
				{
					in_server_block = false;
					this->_servers.back()->check_set_default();
				}
				else
					this->_servers.back()->compare_block_info(line, indata);
			}
			else if (line[0] != '#')
				throw Webserv::InvalidFileContentException();
		}
	}
	indata.close();
	if (in_server_block)
		throw Webserv::InvalidFileContentException();
	std::set<int> all_ports;
	size_t number_of_ports = 0;

	std::list<Server *>::iterator it = this->_servers.begin();
	std::list<Server *>::iterator ite = this->_servers.end();
	for (; it != ite; it++)
		(*it)->add_ports(all_ports, &number_of_ports);
}

void Webserv::display_servs_content(void)
{
	std::list<Server *>::iterator it = this->_servers.begin();
	std::list<Server *>::iterator ite = this->_servers.end();
	for (; it != ite; it++)
	{
		(*it)->display_serv_content();
		std::cout << std::endl << std::endl;
	}
}

Server *Webserv::get_polling_serv(uint16_t sin_port)
{
	std::list<Server *>::iterator it = this->_servers.begin();
	std::list<Server *>::iterator ite = this->_servers.end();

	std::cout << "sin_port: " << (int)sin_port << std::endl;
	for (; it != ite; it++)
	{
		std::list<int>::iterator pit = (*it)->_ports.begin();
		std::list<int>::iterator pite = (*it)->_ports.end();
		for (; pit != pite; pit++)
		{
			if (*pit == sin_port)
				return (*it);
		}
	}
	return (NULL);
}

void Webserv::setup_servers(void)
{
	std::list<Server *>::iterator it = this->_servers.begin();
	std::list<Server *>::iterator ite = this->_servers.end();
	
	std::list<Server *>::iterator it_tmp = it;

	size_t total_ports = 0;
	for (; it_tmp != ite; it_tmp++)
		total_ports += (*it_tmp)->_ports.size();

	struct pollfd pfds[total_ports];
	struct sockaddr_in address[total_ports];
	size_t index = 0;

	for (; it != ite; it++)
	{
		if ((*it)->_ports.empty())
			continue ;

		std::list<int>::iterator pit = (*it)->_ports.begin();
		std::list<int>::iterator pite = (*it)->_ports.end();
		
		for (; pit != pite; pit++)
		{
			std::cout << "Setting up port " << *pit << '.' << std::endl;

			if ((pfds[index].fd = socket(PF_INET, SOCK_STREAM, 0)) == 0)
			{
				perror("socket");
				return ;
			}
			pfds[index].events = POLLIN;

			address[index].sin_family = PF_INET;
			address[index].sin_addr.s_addr = INADDR_ANY;
			address[index].sin_port = htons(*pit);

			if (bind(pfds[index].fd, (struct sockaddr *) &address[index], sizeof(address[index])) < 0)
			{
				perror("bind");
				return ;
			}

			if (listen(pfds[index].fd, 10) < 0)
			{
				perror("listen");
				return ;
			}
			++index;
		}
	}

	int ready;
	// size_t addrlen;

	while(1)
	{
		std::cout << "\n+++++++ Waiting for new connection ++++++++\n\n";
		ready = poll(pfds, total_ports, -1);
		if (ready == -1)
		{
			perror("poll");
			return ;
		}

		(ready == 1)
			? std::cout << "1 socket ready" << std::endl
			: std::cout << ready << " sockets ready" << std::endl;
		for (size_t index = 0; index < total_ports; index++) {

			if (pfds[index].revents != 0) {
				std::cout << "  - (index " << index << ") socket = " << pfds[index].fd << "; events: ";
				(pfds[index].revents & POLLIN)  ? std::cout << "POLLIN "  : std::cout << "";
				(pfds[index].revents & POLLHUP) ? std::cout << "POLLHUP " : std::cout << "";
				(pfds[index].revents & POLLERR) ? std::cout << "POLLERR " : std::cout << "";
				std::cout << std::endl << std::endl;

				Server *polling_serv = get_polling_serv(ntohs(address[index].sin_port));
				if (!polling_serv)
				{
					std::cerr << "ERROR: no matching port in servers ?????" << std::endl;
					continue ;
				}

				// addrlen = sizeof(address[index]);
				if ((polling_serv->_socket_fd = accept(pfds[index].fd, NULL, 0)) < 0)//(struct sockaddr *) &address->butnotaddressthatisup[index], (socklen_t*) &addrlen)) < 0)
				{
					perror("accept");
					return ;
				}

				try
				{
					std::string bufstr = polling_serv->recv_lines(1);
					polling_serv->analyse_request(bufstr);
				}
				catch (std::exception & e) {/*std::cerr << e.what() << std::endl;*/}
				std::cout << std::endl << "------------------content message sent to " << polling_serv->_socket_fd << "-------------------\n";
				close(polling_serv->_socket_fd);
			}
		}
		// return ;
	}
}

// ************************************************************************** //
//                                 Exceptions                                 //
// ************************************************************************** //

const char* Webserv::InvalidFileException::what() const throw()
{
	return ("[Webserv::InvalidFileException] Configuration file could not be opened.");
}

const char* Webserv::InvalidFileExtensionException::what() const throw()
{
	return ("[Webserv::InvalidFileExtensionException] Configuration file does not end with '.conf'.");
}

const char* Webserv::InvalidFileContentException::what() const throw()
{
	return ("[Webserv::InvalidFileContentException] Configuration file contains invalid server info.");
}

const char* Webserv::MissingDefault404Exception::what() const throw()
{
	return ("[Webserv::MissingDefault404Exception] No default 404 file in root provided by conf file.");
}

const char* Webserv::SystemCallException::what() const throw()
{
	return ("[Webserv::SystemCallException] System call did not call.");
}

const char* Webserv::QuickerReturnException::what() const throw()
{
	return ("[Webserv::QuickerReturnException] gota go fast.");
}
