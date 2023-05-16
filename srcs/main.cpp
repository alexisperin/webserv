/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/12 11:22:56 by aperin            #+#    #+#             */
/*   Updated: 2023/05/16 18:19:23 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <fstream>
#include <iostream>

// int main()
// {
// 	int server_fd, new_socket;
// 	ssize_t valread;
// 	struct sockaddr_in address;
// 	int addrlen = sizeof(address);

// 	std::string hello("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!");
// 	std::string refresh("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 29\n\nThis page has been refreshed ");
// 	std::string count_base("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: ");
// 	std::string times("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 7\n\n times.");

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
// 		perror("In listen");
// 		return -1;
// 	}

// 	int count = 0;
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
// 		std::cout << "send #0 : " << send(new_socket, hello.c_str(), hello.size(), 0) << std::endl;
		
// 		char bufferb[30000] = {0};
// 		valread = recv(new_socket, bufferb, 30000, 0);
		
// 		std::cout << "send #1 : " << send(new_socket, refresh.c_str(), refresh.size(), 0) << std::endl;
// 		char bufferbb[30000] = {0};
// 		valread = recv(new_socket, bufferbb, 30000, 0);
// 		std::string number = std::to_string(count);
// 		std::string count_str = count_base + std::to_string(number.size()) + "\n\n" + number;
// 		std::cout << "send #2 : " << send(new_socket, count_str.c_str(), count_str.size(), 0) << std::endl;
// 		char bufferbbb[30000] = {0};
// 		valread = recv(new_socket, bufferbbb, 30000, 0);
// 		std::cout << "send #3 : " << send(new_socket, times.c_str(), times.size(), 0) << std::endl;

// 		std::cout << "------------------Hello message sent-------------------\n";
// 		++count;
// 	}
// 	return 0;
// }

int main( int ac, char **av )
{
	if (ac != 2) {
		std::cerr << "Usage: ./webserv <file>" << std::endl;
		return (1);
	}

	std::ifstream indata( av[1] );
	std::string file_content = "";
	if (!indata.is_open()) {
		std::cerr << "Failed to open " << av[1] << std::endl;
	}
	std::string line;
	while (!indata.eof()) {
		std::getline( indata, line );
		file_content += line + '\n';
	}
	indata.close();

	int server_fd, new_socket;
	ssize_t valread;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	// std::string content("HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: 567\n\n<HTML>\n	<HEAD>\n		<TITLE>Your Title Here</TITLE>\n	</HEAD>\n\n	<BODY BGCOLOR='FFFFFF'>\n		<CENTER><IMG SRC='clouds.jpg' ALIGN='BOTTOM'> </CENTER>\n		<HR>\n		<a href='http://somegreatsite.com'>Link Name</a>\n		is a link to another nifty site\n		<H1>This is a Header</H1>\n		<H2>This is a Medium Header</H2>\n		Send me mail at <a href='mailto:support@yourcompany.com'>\n		support@yourcompany.com</a>.\n		<P> This is a new paragraph!\n		<P> <B>This is a new paragraph!</B>\n		<BR> <B><I>This is a new sentence without a paragraph break, in bold italics.</I></B>\n		<HR>\n	</BODY>\n</HTML>");
	std::string content("HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: ");
	content += std::to_string(file_content.size()) + "\n\n" + file_content;

	if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket");
		return -1;
	}

	address.sin_family = PF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(80);

	if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0)
	{
		perror("bind");
		return -1;
	}
	if (listen(server_fd, 10) < 0)
	{
		perror("In listen");
		return -1;
	}

	int count = 0;
	while(1)
	{
		std::cout << "\n+++++++ Waiting for new connection ++++++++\n\n";
		if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t*) &addrlen)) < 0)
		{
			perror("accept");
			return -1;
		}

		char buffer[30000] = {0};
		valread = recv(new_socket, buffer, 30000, 0);
		std::cout << buffer << std::endl;
		send(new_socket, content.c_str(), content.size(), 0);

		std::cout << "------------------content message sent-------------------\n";
		++count;
	}
	return 0;
}
