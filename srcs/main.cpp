/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aperin <aperin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/12 11:22:56 by aperin            #+#    #+#             */
/*   Updated: 2023/05/17 15:46:20 by aperin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>

#include "Webserv.hpp"

int main(int ac, char **av)
{
	if (ac != 2) {
		std::cerr << "Usage: ./webserv <file.conf>" << std::endl;
		return (1);
	}
	std::string file_name(av[1]);
	try
	{
		Webserv my_serv(file_name);
		my_serv.display_serv_content();
		my_serv.setup_server();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return (1);
	}

	
	return (0);
}

// int main()
// {
// 	int server_fd, new_socket;
// 	ssize_t valread;
// 	struct sockaddr_in address;
// 	int addrlen = sizeof(address);

// 	std::string hello("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!");

// 	if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == 0)
// 	{
// 		perror("socket");
// 		return -1;
// 	}

// 	address.sin_family = PF_INET;
// 	address.sin_addr.s_addr = INADDR_ANY;
// 	address.sin_port = htons(80);

// 	if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0)
// 	{
// 		perror("bind");
// 		return -1;
// 	}
// 	if (listen(server_fd, 10) < 0)
// 	{
// 		perror("listen");
// 		return -1;
// 	}

// 	while(1)
// 	{
// 		std::cout << "\n+++++++ Waiting for new connection ++++++++\n\n";
// 		if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t*) &addrlen)) < 0)
// 		{
// 			perror("accept");
// 			return -1;
// 		}

// 		char buffer[30000] = {0};
// 		valread = recv(new_socket, buffer, 30000, 0);
// 		std::cout << buffer << std::endl;
// 		send(new_socket, hello.c_str(), hello.size(), 0);
// 		std::cout << "------------------Hello message sent-------------------\n";
// 	}
// 	return 0;
// }